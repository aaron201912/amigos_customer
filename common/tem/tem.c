/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (��Sigmastar Confidential Information��) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/prctl.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "list.h"
#include "tem.h"

#define AddTime(ts, ms)                         \
    do{                                             \
        if((ms) >= 1000)                               \
        {                                           \
            ts.tv_sec += (ms) / 1000;                 \
        }                                           \
        ts.tv_nsec += (((ms) % 1000)*1000000);        \
        if(ts.tv_nsec >= 1000000000)                \
        {                                           \
            ts.tv_sec++;                            \
            ts.tv_nsec = ts.tv_nsec - 1000000000;   \
        }                                           \
    }while(0);

#define DEBUG(fmt, args...) //printf(fmt, ##args);
#define TRACE(fmt, args...) //printf(fmt, ##args);
#define INFO(fmt, args...) //printf(fmt, ##args);
#define WARN(fmt, args...) //printf(fmt, ##args);
#define ERROR(fmt, args...) printf("[TEM][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);

#if 1
//TEM: Thread event manager, Writed by Alias.Peng from SigmaStar
// Start######
typedef enum{
    E_TEM_IDLE,
    E_TEM_DO_USER_DATA,
    E_TEM_DROP_USER_DATA,
    E_TEM_DROP_USER_DATA_END,
    E_TEM_EXIT,
    E_TEM_START_MONITOR,
    E_TEM_START_ONESHOT,
    E_TEM_STOP,
}EN_TEM_STATUS;

typedef struct ST_TEM_DATA_NODE_s{
    EN_TEM_STATUS enTemThrSt;  //Thread status.
    ST_TEM_USER_DATA stUserData; //User data
    struct list_head stDataList; //Data list node
}ST_TEM_DATA_NODE;

typedef struct{
    pthread_attr_t thread_attr;
    pthread_mutex_t mutex;
    pthread_mutex_t data_mutex;
    pthread_cond_t cond;
    pthread_t thread;
    pthread_condattr_t cond_attr;
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_t data_mutex_attr;
}ST_TEM_INFO;

typedef struct ST_TEM_NODE_s{
    char *pThreadName;     // Tem thread name.
    ST_TEM_ATTR *pTemAttr; // User setting.
    ST_TEM_INFO *pTemInfo; // Info of thread.
    unsigned int intTotalDataCnt; // List total data count.
    unsigned int intTotalListCnt; // List total count.
    struct list_head stDataListHead; // Tem data event list head of ST_TEM_DATA_NODE.
    struct list_head stTemNodeList; // Tem node.
    unsigned int maxEventCout; //Event max
    unsigned int maxDataCnt; //data max
    unsigned char bDropEvent;  //b Drop event
    unsigned char bDropData;  //b Drop data
}ST_TEM_NODE;

static pthread_mutex_t m_MutexTem = PTHREAD_MUTEX_INITIALIZER;
unsigned int _GetTime0()
{
    struct timespec ts;
    unsigned int ms;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    if(ms == 0)
    {
        ms = 1;
    }
    return ms;
}
LIST_HEAD(stTemNodeHead); // Tem node, head of ST_TEM_NODE.

static unsigned char _TemPushEvent(ST_TEM_NODE *pTemNode, EN_TEM_STATUS enStatus, ST_TEM_USER_DATA *pstData, pthread_mutex_t *pdata_mutex)
{
    ST_TEM_DATA_NODE *pNode = NULL;

    if (pTemNode == NULL)
    {
        ERROR("pTemNode  is NULL!!\n");
        return -1;
    }

    pNode = (ST_TEM_DATA_NODE *)malloc(sizeof(ST_TEM_DATA_NODE));
    ASSERT(pNode);
    //printf("Malloc %lu\n", (unsigned long)pNode);
    pNode->stUserData.u32UserDataSize = 0;
    pNode->stUserData.u32BufferRealSize = 0;
    if (pstData != NULL)
    {
        memcpy(&pNode->stUserData, pstData, sizeof(ST_TEM_USER_DATA));
    }
    pNode->enTemThrSt = enStatus;
    pthread_mutex_lock(pdata_mutex);
    list_add_tail(&pNode->stDataList, &pTemNode->stDataListHead);
    pTemNode->intTotalListCnt++;
    pTemNode->intTotalDataCnt += pNode->stUserData.u32BufferRealSize;
    pthread_mutex_unlock(pdata_mutex);
    if (enStatus == E_TEM_DO_USER_DATA)
    {
        ST_TEM_DATA_NODE *pNodeDrop = NULL;
        ST_TEM_DATA_NODE *pNodeDropEnd = NULL;

        //printf("total %d max %d total list %d max %d\n", pTemNode->intTotalDataCnt, pTemNode->maxDataCnt, pTemNode->intTotalListCnt, pTemNode->maxEventCout);
        if (pTemNode->intTotalDataCnt > pTemNode->maxDataCnt && pTemNode->bDropData)
        {
            ERROR("Event data is over range %d[max: %d]! Drop data event!\n", pTemNode->intTotalDataCnt, pTemNode->maxDataCnt);
            pNodeDrop = (ST_TEM_DATA_NODE *)malloc(sizeof(ST_TEM_DATA_NODE));
            ASSERT(pNodeDrop);
            pNodeDrop->stUserData.u32UserDataSize = 0;
            pNodeDrop->stUserData.pUserData = NULL;
            pNodeDrop->stUserData.u32BufferRealSize = 0;
            pNodeDrop->enTemThrSt = E_TEM_DROP_USER_DATA;
            pNodeDropEnd = (ST_TEM_DATA_NODE *)malloc(sizeof(ST_TEM_DATA_NODE));
            ASSERT(pNodeDropEnd);
            pNodeDropEnd->stUserData.u32UserDataSize = 0;
            pNodeDropEnd->stUserData.pUserData = NULL;
            pNodeDropEnd->stUserData.u32BufferRealSize = 0;
            pNodeDropEnd->enTemThrSt = E_TEM_DROP_USER_DATA_END;

            pthread_mutex_lock(pdata_mutex);
            list_add(&pNodeDrop->stDataList, &pTemNode->stDataListHead);
            list_add_tail(&pNodeDropEnd->stDataList, &pTemNode->stDataListHead);
            pTemNode->intTotalListCnt += 2;
            pthread_mutex_unlock(pdata_mutex);

            return -1;
        }
        if (pTemNode->intTotalListCnt > pTemNode->maxEventCout && pTemNode->bDropEvent)
        {
            ERROR("Event list is over range %d[max: %d]! Drop data event!\n", pTemNode->intTotalListCnt, pTemNode->maxEventCout);
            pNodeDrop = (ST_TEM_DATA_NODE *)malloc(sizeof(ST_TEM_DATA_NODE));
            ASSERT(pNodeDrop);
            pNodeDrop->stUserData.u32UserDataSize = 0;
            pNodeDrop->stUserData.pUserData = NULL;
            pNodeDrop->stUserData.u32BufferRealSize = 0;
            pNodeDrop->enTemThrSt = E_TEM_DROP_USER_DATA;
            pNodeDropEnd = (ST_TEM_DATA_NODE *)malloc(sizeof(ST_TEM_DATA_NODE));
            ASSERT(pNodeDropEnd);
            pNodeDropEnd->stUserData.u32UserDataSize = 0;
            pNodeDropEnd->stUserData.pUserData = NULL;
            pNodeDropEnd->stUserData.u32BufferRealSize = 0;
            pNodeDropEnd->enTemThrSt = E_TEM_DROP_USER_DATA_END;

            pthread_mutex_lock(pdata_mutex);
            list_add(&pNodeDrop->stDataList, &pTemNode->stDataListHead);
            list_add_tail(&pNodeDropEnd->stDataList, &pTemNode->stDataListHead);
            pTemNode->intTotalListCnt += 2;
            pthread_mutex_unlock(pdata_mutex);

            return -1;
        }
    }

    return 0;
}
static EN_TEM_STATUS _TemPopEvent(ST_TEM_NODE *pTemNode, ST_TEM_USER_DATA *pstData, pthread_mutex_t *pdata_mutex)
{
    ST_TEM_DATA_NODE *pNode = NULL;
    EN_TEM_STATUS enRev = E_TEM_IDLE;

    if (pTemNode == NULL)
    {
        ERROR("pTemNode  is NULL!!\n");
        return E_TEM_IDLE;
    }
    if (pstData == NULL)
    {
        ERROR("pstData  is NULL!!\n");
        return E_TEM_IDLE;
    }
    if (pdata_mutex == NULL)
    {
        ERROR("pdata_mutex  is NULL!!\n");
        return E_TEM_IDLE;
    }
    pthread_mutex_lock(pdata_mutex);
    if (list_empty(&pTemNode->stDataListHead))
    {
        pthread_mutex_unlock(pdata_mutex);
        return E_TEM_IDLE;
    }
    pNode = list_entry(pTemNode->stDataListHead.next, ST_TEM_DATA_NODE, stDataList);
    enRev = pNode->enTemThrSt;
    memcpy(pstData, &pNode->stUserData, sizeof(ST_TEM_USER_DATA));
    list_del(&pNode->stDataList);
    pTemNode->intTotalListCnt--;
    pTemNode->intTotalDataCnt -= pNode->stUserData.u32BufferRealSize;
    pthread_mutex_unlock(pdata_mutex);
    //printf("Free %lu\n", (unsigned long)pNode);
    free(pNode);
    return enRev;
}
static void _TemEventMonitor(ST_TEM_NODE *pTempNode, unsigned int *pu32TimeOut, unsigned char *bRunOneShotFb, unsigned char *pbRun)
{
    unsigned short u16DropEvent = 0;
    ST_TEM_USER_DATA stData;
    EN_TEM_STATUS enTemThrSt;

    while (1)
    {
        enTemThrSt = _TemPopEvent(pTempNode, &stData, &pTempNode->pTemInfo->data_mutex);
        if (E_TEM_IDLE == enTemThrSt)
        {
            break;
        }
        switch (enTemThrSt)
        {
            case E_TEM_EXIT:
                {
                    INFO("Exit thread id:[%x], name[%s]\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName);
                    *pbRun = 0;
                    *pu32TimeOut = 0;
                }
                break;
            case E_TEM_STOP:
                {
                    INFO("Stop thread id:[%x], name[%s]\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName);
                    *pu32TimeOut = 0xFFFFFFFF;
                }
                break;
            case E_TEM_START_MONITOR:
                {
                    INFO("Start thread monitor id:[%x], name[%s]\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName);
                    *pu32TimeOut = pTempNode->pTemAttr->u32ThreadTimeoutMs;
                }
                break;
            case E_TEM_START_ONESHOT:
                {
                    INFO("Start one shot thread id:[%x], name[%s]\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName);
                    *bRunOneShotFb = 1;
                    *pu32TimeOut = pTempNode->pTemAttr->u32ThreadTimeoutMs;
                }
                break;
            case E_TEM_DO_USER_DATA:
                {
                    if (u16DropEvent)
                    {
                        //ERROR("Do user data thread id:[%x], name[%s] Drop event!\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName);
                        if (pTempNode->pTemAttr->fpThreadDropEvent)
                        {
                            pTempNode->pTemAttr->fpThreadDropEvent(pTempNode->pTemAttr->stTemBuf, stData);
                        }
                    }
                    else
                    {
                        INFO("Do user data thread id:[%x], name[%s]\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName);
                        if (pTempNode->pTemAttr->fpThreadDoSignal)
                        {
                            pTempNode->pTemAttr->fpThreadDoSignal(pTempNode->pTemAttr->stTemBuf, stData);
                        }
                    }
                    if (stData.u32UserDataSize != 0)
                    {
                        INFO("Free buffer %lu\n", (unsigned long)stData.pUserData);
                        free(stData.pUserData);
                        stData.pUserData = NULL;
                    }
                }
                break;
            case E_TEM_DROP_USER_DATA:
                {
                    u16DropEvent++;
                    ERROR("T[%x],N[%s] Drop event! cnt: %d\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName, u16DropEvent);
                }
                break;
            case E_TEM_DROP_USER_DATA_END:
                {
                    u16DropEvent--;
                    ERROR("T[%x],N[%s] Drop event! cnt: %d\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName, u16DropEvent);
                }
                break;
            default:
                {
                    ERROR("Error tem thread event.id:[%x], name[%s]![%d]",  (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName, enTemThrSt);
                }
                break;
        }
    }
    if (*pbRun == 0)
    {
        INFO("TEM name [%s]: Bye bye~\n", pTempNode->pThreadName);

        return;
    }

    return;
}
static void *_TemThreadMain(void * pArg)
{
    ST_TEM_NODE *pTempNode = (ST_TEM_NODE *)pArg;
    int intCondWaitRev = 0;
    struct timespec stOuttime;
    unsigned int u32FuncRunTime = 0;
    unsigned char bRun = 1;
    unsigned char bRunOneShot = 0;
    unsigned int u32TimeOut = 0xFFFFFFFF;

    if(pTempNode == NULL)
    {
        ERROR("Error pArg is NULL!!\n");
        return NULL;
    }
    prctl(PR_SET_NAME, (unsigned long)pTempNode->pThreadName);
    memset(&stOuttime, 0, sizeof(struct timespec));
    while (bRun)
    {
        while (1)
        {
            _TemEventMonitor(pTempNode, &u32TimeOut, &bRunOneShot, &bRun);
            if (!bRun)
                return NULL;
            if (u32TimeOut)
                break;
            if (pTempNode->pTemAttr->fpThreadWaitTimeOut)
            {
                pTempNode->pTemAttr->fpThreadWaitTimeOut(pTempNode->pTemAttr->stTemBuf);
                if (bRunOneShot)
                {
                    u32TimeOut = 0xFFFFFFFF;
                    bRunOneShot = 0;
                    break;
                }
            }
        }
        PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
        while (1)
        {
            _TemEventMonitor(pTempNode, &u32TimeOut, &bRunOneShot, &bRun);
            if (!bRun)
                break;
            if (!u32TimeOut)
                break;
            u32FuncRunTime = _GetTime0();
            if (pTempNode->pTemAttr->fpThreadWaitTimeOut && intCondWaitRev == ETIMEDOUT)
            {
                pTempNode->pTemAttr->fpThreadWaitTimeOut(pTempNode->pTemAttr->stTemBuf);
                if (bRunOneShot)
                {
                    u32TimeOut = 0xFFFFFFFF;
                    bRunOneShot = 0;
                }
            }
            u32FuncRunTime = _GetTime0() - u32FuncRunTime;
            switch (u32TimeOut)
            {
                case 0xFFFFFFFF:
                    intCondWaitRev = pthread_cond_wait(&pTempNode->pTemInfo->cond, &pTempNode->pTemInfo->mutex);
                    break;
                default:
                {
                    struct timespec stCurTime;
                    unsigned char bWait = 0;
                    clock_gettime(CLOCK_MONOTONIC, &stCurTime);
                    if ((stCurTime.tv_sec > stOuttime.tv_sec) \
                            ||((stCurTime.tv_sec == stOuttime.tv_sec)?(stCurTime.tv_nsec >= stOuttime.tv_nsec):0) \
                            || pTempNode->pTemAttr->bSignalResetTimer)
                    {
                        /*         Case 1:
                         *                   Reset timer flag  is on.
                         *                   Current time is later than or  equal  the pthread wait time(stOuttime).
                         *                   In this case stOuttime will do stCurTime+u32ThreadTimeoutMs
                         *          Case 0:
                         *                   Current time is smaller than the pthread wait time(stOuttime).
                         *                   This case must be get cond signal while waitting time out .
                         *          When first run, and stOuttime is zero and current time is definally larger.
                         */
                        memcpy(&stOuttime, &stCurTime, sizeof(struct timespec));
                        if (u32FuncRunTime < u32TimeOut)
                        {
                            AddTime(stOuttime, u32TimeOut - u32FuncRunTime);
                            bWait = 1;
                        }
                        else if (u32TimeOut != 0)
                        {
                            WARN("Func run time is too much, and it's larger than monitor wait time!!\n ");
                        }
                    }
                    if (bWait)
                        intCondWaitRev = pthread_cond_timedwait(&pTempNode->pTemInfo->cond, &pTempNode->pTemInfo->mutex, &stOuttime);
                    else
                        intCondWaitRev = ETIMEDOUT;
                }
                break;
            }
            //INFO("thread id:[%x], name[%s] Func run time %d\n", (int)pTempNode->pTemInfo->thread, pTempNode->pThreadName, u32FuncRunTime);
        }
        PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    }

    return NULL;
}
static struct list_head *_TemFindFp(struct list_head *pstPos, void *pKey)
{
    ST_TEM_NODE *pstNode =  list_entry(pstPos, ST_TEM_NODE, stTemNodeList);
    char *pName = (char *)pKey;

    return strcmp(pName, pstNode->pThreadName)?pstPos->next:pstPos;
}
static ST_TEM_NODE *_TemFindNode(const char* pStr)
{
    ST_TEM_NODE *pTemp = NULL;
    struct list_head *pList = NULL;

    MUTEXCHECK(pthread_mutex_lock(&m_MutexTem));
    pList = list_find(&stTemNodeHead, (void *)pStr, _TemFindFp);
    if (pList != &stTemNodeHead)
    {
        pTemp = list_entry(pList, ST_TEM_NODE, stTemNodeList);
    }
    else
    {
        ERROR("Not found compaired list: %s\n", pStr);
    }
    MUTEXCHECK(pthread_mutex_unlock(&m_MutexTem));

    return pTemp;
}
static void _TemCondSignal(ST_TEM_NODE *pTempNode)
{
    if (pTempNode->pTemInfo->thread != getpid())
    {
        PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
        PTH_RET_CHK(pthread_cond_signal(&pTempNode->pTemInfo->cond));
        PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    }
}
int TemOpen(const char* pStr, ST_TEM_ATTR stAttr)
{
    ST_TEM_NODE *pTemp = NULL;
    struct list_head *pList = NULL;

    pTemp = (ST_TEM_NODE *)malloc(sizeof(ST_TEM_NODE));
    ASSERT(pTemp);
    memset(pTemp, 0, sizeof(ST_TEM_NODE));
    /*Init data event list*/
    INIT_LIST_HEAD(&pTemp->stDataListHead);
    /*End*/

    /*Malloc thread name buffer.*/
    pTemp->pThreadName = (char *)malloc(strlen(pStr) + 1);
    ASSERT(pTemp->pThreadName);
    memset(pTemp->pThreadName, 0, strlen(pStr) + 1);
    strcpy(pTemp->pThreadName, pStr);
    /*End*/

    /*Malloc Tem Attr buffer*/
    pTemp->pTemAttr = (ST_TEM_ATTR *)malloc(sizeof(ST_TEM_ATTR));
    ASSERT(pTemp->pTemAttr);
    memset(pTemp->pTemAttr, 0, sizeof(ST_TEM_ATTR));
    memcpy(pTemp->pTemAttr, &stAttr, sizeof(ST_TEM_ATTR));
    /*End*/

    /*Malloc Tem internal buffer size*/
    if (stAttr.stTemBuf.u32TemBufferSize != 0 && stAttr.stTemBuf.pTemBuffer != NULL)
    {
        pTemp->pTemAttr->stTemBuf.pTemBuffer = (void *)malloc(stAttr.stTemBuf.u32TemBufferSize);
        ASSERT(pTemp->pTemAttr->stTemBuf.pTemBuffer);
        memcpy(pTemp->pTemAttr->stTemBuf.pTemBuffer, stAttr.stTemBuf.pTemBuffer, stAttr.stTemBuf.u32TemBufferSize);
    }
    /*End*/

    /*Event choice*/
    pTemp->bDropEvent = stAttr.bDropEvent;
    pTemp->bDropData = stAttr.bDropData;
    if (stAttr.bDropData)
    {
        pTemp->maxDataCnt = stAttr.maxDataCout;
    }
    if (stAttr.bDropEvent)
    {
        pTemp->maxEventCout = stAttr.maxEventCout;
    }
    pTemp->pTemInfo = (ST_TEM_INFO *)malloc(sizeof(ST_TEM_INFO));
    ASSERT(pTemp->pTemInfo);
    memset(pTemp->pTemInfo, 0, sizeof(ST_TEM_INFO));

    /*End*/
    MUTEXCHECK(pthread_mutex_lock(&m_MutexTem));
    pList = list_find(&stTemNodeHead, (void *)pStr, _TemFindFp);
    if (pList != &stTemNodeHead)
    {
        ERROR("List exist: %s !!!\n", pStr);
        MUTEXCHECK(pthread_mutex_unlock(&m_MutexTem));
        if (pTemp->pTemInfo)
            free(pTemp->pTemInfo);
        if (pTemp->pThreadName)
            free(pTemp->pThreadName);
        if (pTemp->pTemAttr)
        {
            if (pTemp->pTemAttr->stTemBuf.pTemBuffer
                && pTemp->pTemAttr->stTemBuf.u32TemBufferSize != 0)
            {
                free(pTemp->pTemAttr->stTemBuf.pTemBuffer);
            }
            free(pTemp->pTemAttr);
        }
        free(pTemp);
        return -1;
    }
    list_add_tail(&pTemp->stTemNodeList, &stTemNodeHead);
    MUTEXCHECK(pthread_mutex_unlock(&m_MutexTem));

    /*Malloc Tem info*/
    PTH_RET_CHK(pthread_attr_init(&pTemp->pTemInfo->thread_attr));
    PTH_RET_CHK(pthread_condattr_init(&(pTemp->pTemInfo->cond_attr)));
    PTH_RET_CHK(pthread_condattr_setclock(&(pTemp->pTemInfo->cond_attr), CLOCK_MONOTONIC));
    PTH_RET_CHK(pthread_mutexattr_init(&(pTemp->pTemInfo->mutex_attr)));
    PTH_RET_CHK(pthread_mutexattr_settype(&(pTemp->pTemInfo->mutex_attr), PTHREAD_MUTEX_RECURSIVE));
    PTH_RET_CHK(pthread_mutex_init(&(pTemp->pTemInfo->mutex), &(pTemp->pTemInfo->mutex_attr)));
    PTH_RET_CHK(pthread_mutexattr_settype(&(pTemp->pTemInfo->data_mutex_attr), PTHREAD_MUTEX_RECURSIVE));
    PTH_RET_CHK(pthread_mutex_init(&(pTemp->pTemInfo->data_mutex), &(pTemp->pTemInfo->data_mutex_attr)));
    PTH_RET_CHK(pthread_cond_init(&(pTemp->pTemInfo->cond), &(pTemp->pTemInfo->cond_attr)));
    PTH_RET_CHK(pthread_create(&(pTemp->pTemInfo->thread), &(pTemp->pTemInfo->thread_attr), _TemThreadMain, (void *)pTemp));
    /*End*/

    return 0;
}
int TemClose(const char* pStr)
{
    ST_TEM_NODE *pTempNode = NULL;
    void *retval = NULL;

    if(pStr == NULL)
    {
        ERROR("pStr is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    ASSERT(pTempNode);

    _TemPushEvent(pTempNode, E_TEM_EXIT, NULL, &pTempNode->pTemInfo->data_mutex);
    PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
    PTH_RET_CHK(pthread_cond_signal(&pTempNode->pTemInfo->cond));
    PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    PTH_RET_CHK(pthread_join(pTempNode->pTemInfo->thread, &retval));
    ASSERT(list_empty(&pTempNode->stDataListHead));
    PTH_RET_CHK(pthread_mutexattr_destroy(&pTempNode->pTemInfo->mutex_attr));
    PTH_RET_CHK(pthread_mutex_destroy(&pTempNode->pTemInfo->mutex));
    PTH_RET_CHK(pthread_condattr_destroy(&pTempNode->pTemInfo->cond_attr));
    PTH_RET_CHK(pthread_cond_destroy(&pTempNode->pTemInfo->cond));
    PTH_RET_CHK(pthread_attr_destroy(&pTempNode->pTemInfo->thread_attr));
    MUTEXCHECK(pthread_mutex_lock(&m_MutexTem));
    list_del(&pTempNode->stTemNodeList);
    MUTEXCHECK(pthread_mutex_unlock(&m_MutexTem));

    if (pTempNode->pTemInfo)
    {
        free(pTempNode->pTemInfo);
        pTempNode->pTemInfo = NULL;
    }
    if (pTempNode->pTemAttr)
    {
        if (pTempNode->pTemAttr->stTemBuf.pTemBuffer
            && pTempNode->pTemAttr->stTemBuf.u32TemBufferSize != 0)
        {
            free(pTempNode->pTemAttr->stTemBuf.pTemBuffer);
        }
        pTempNode->pTemAttr->stTemBuf.pTemBuffer = NULL;
        free(pTempNode->pTemAttr);
        pTempNode->pTemAttr = NULL;
    }
    if (pTempNode->pThreadName)
    {
        free(pTempNode->pThreadName);
        pTempNode->pThreadName = NULL;
    }
    free(pTempNode);
    pTempNode = NULL;

    return 0;
}
int TemStartMonitor(const char* pStr)
{
    ST_TEM_NODE *pTempNode = NULL;

    if(pStr == NULL)
    {
        ERROR("pStr is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    _TemPushEvent(pTempNode, E_TEM_START_MONITOR, NULL, &pTempNode->pTemInfo->data_mutex);
    _TemCondSignal(pTempNode);

    return 0;
}
int TemStartOneShot(const char* pStr)
{
    ST_TEM_NODE *pTempNode = NULL;

    if(pStr == NULL)
    {
        ERROR("pStr is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    _TemPushEvent(pTempNode, E_TEM_START_ONESHOT, NULL, &pTempNode->pTemInfo->data_mutex);
    _TemCondSignal(pTempNode);

    return 0;
}
int TemConfigTimer(const char* pStr, unsigned int u32TimeOut, unsigned char bSignalResetTimer)
{
    ST_TEM_NODE *pTempNode = NULL;
    if(pStr == NULL)
    {
        ERROR("pStr is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    if (pTempNode->pTemInfo->thread == getpid())
    {
        if (u32TimeOut != 0)
        {
            pTempNode->pTemAttr->u32ThreadTimeoutMs = u32TimeOut;
        }
        pTempNode->pTemAttr->bSignalResetTimer = bSignalResetTimer;
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
        if (u32TimeOut != 0)
        {
            pTempNode->pTemAttr->u32ThreadTimeoutMs = u32TimeOut;
        }
        pTempNode->pTemAttr->bSignalResetTimer = bSignalResetTimer;
        PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    }

    return 0;
}
int TemStop(const char* pStr)
{
    ST_TEM_NODE *pTempNode = NULL;

    if(pStr == NULL)
    {
        ERROR("pStr is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    _TemPushEvent(pTempNode, E_TEM_STOP, NULL, &pTempNode->pTemInfo->data_mutex);
    _TemCondSignal(pTempNode);

    return 0;
}
int TemSend(const char* pStr, ST_TEM_USER_DATA stUserData)
{
    ST_TEM_NODE *pTempNode = NULL;
    void *pDataBuffer = NULL;

    if(pStr == NULL)
    {
        ERROR("pStr or pUserData is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    if (stUserData.u32UserDataSize != 0)
    {
        pDataBuffer = (void *)malloc(stUserData.u32UserDataSize);
        ASSERT(pDataBuffer);
        INFO("Malloc buffer 0x%lx\n", (unsigned long)pDataBuffer);
        memcpy(pDataBuffer, stUserData.pUserData, stUserData.u32UserDataSize);
        stUserData.pUserData = pDataBuffer;
    }
    _TemPushEvent(pTempNode, E_TEM_DO_USER_DATA, &stUserData, &pTempNode->pTemInfo->data_mutex);
    _TemCondSignal(pTempNode);

    return 0;
}
int TemSetBuffer(const char* pStr, void *pBufferData)
{
    ST_TEM_NODE *pTempNode = NULL;

    if(pStr == NULL || pBufferData == NULL)
    {
        ERROR("pStr or pBufferData is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    if (pTempNode->pTemInfo->thread == getpid())
    {
        memcpy(pTempNode->pTemAttr->stTemBuf.pTemBuffer, pBufferData, pTempNode->pTemAttr->stTemBuf.u32TemBufferSize);
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
        memcpy(pTempNode->pTemAttr->stTemBuf.pTemBuffer, pBufferData, pTempNode->pTemAttr->stTemBuf.u32TemBufferSize);
        PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    }

    return 0;
}
int TemGetBuffer(const char* pStr, void *pBufferData)
{
    ST_TEM_NODE *pTempNode = NULL;

    if(pStr == NULL || pBufferData == NULL)
    {
        ERROR("pStr or pBufferData is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    if (pTempNode->pTemInfo->thread == getpid())
    {
        memcpy(pBufferData, pTempNode->pTemAttr->stTemBuf.pTemBuffer, pTempNode->pTemAttr->stTemBuf.u32TemBufferSize);
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
        memcpy(pBufferData, pTempNode->pTemAttr->stTemBuf.pTemBuffer, pTempNode->pTemAttr->stTemBuf.u32TemBufferSize);
        PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    }

    return 0;
}

int TemSetPartBufData(const char* pStr, void *pstBufHeadAddr, void *pstBufPartAddr, unsigned int u32DataSize)
{
    ST_TEM_NODE *pTempNode = NULL;
    unsigned int u32AddrOffSide = 0;

    if(pStr == NULL || pstBufHeadAddr == NULL || pstBufPartAddr == NULL || u32DataSize == 0)
    {
        ERROR("pStr or pBufferData is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    if (pTempNode->pTemInfo->thread == getpid())
    {
        u32AddrOffSide = pstBufPartAddr - pstBufHeadAddr;
        if (pTempNode->pTemAttr->stTemBuf.u32TemBufferSize != 0 && u32AddrOffSide < pTempNode->pTemAttr->stTemBuf.u32TemBufferSize)
        {
            memcpy(pTempNode->pTemAttr->stTemBuf.pTemBuffer + u32AddrOffSide, pstBufPartAddr, u32DataSize);
        }
        else
        {
            ERROR("######Your part of tem buf data addr is error!!!\n");
        }
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
        u32AddrOffSide = pstBufPartAddr - pstBufHeadAddr;
        if (pTempNode->pTemAttr->stTemBuf.u32TemBufferSize != 0 && u32AddrOffSide < pTempNode->pTemAttr->stTemBuf.u32TemBufferSize)
        {
            memcpy(pTempNode->pTemAttr->stTemBuf.pTemBuffer + u32AddrOffSide, pstBufPartAddr, u32DataSize);
        }
        else
        {
            ERROR("######Your part of tem buf data addr is error!!!\n");
        }
        PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    }

    return 0;
}
int TemGetPartBufData(const char* pStr, void *pstBufHeadAddr, void *pstBufPartAddr, unsigned int u32DataSize)
{
    ST_TEM_NODE *pTempNode = NULL;
    unsigned int u32AddrOffSide = 0;

    if(pStr == NULL || pstBufHeadAddr == NULL || pstBufPartAddr == NULL || u32DataSize == 0)
    {
        ERROR("pStr or pBufferData is NULL!!!\n");
        return -1;
    }
    pTempNode = _TemFindNode(pStr);
    if (pTempNode == NULL)
    {
        return -1;
    }
    if (pTempNode->pTemInfo->thread == getpid())
    {
        u32AddrOffSide = pstBufPartAddr - pstBufHeadAddr;
        if (pTempNode->pTemAttr->stTemBuf.u32TemBufferSize != 0 && u32AddrOffSide < pTempNode->pTemAttr->stTemBuf.u32TemBufferSize)
        {
            memcpy(pstBufPartAddr, pTempNode->pTemAttr->stTemBuf.pTemBuffer + u32AddrOffSide, u32DataSize);
        }
        else
        {
            ERROR("######Your part of tem buf data addr is error!!!\n");
        }
    }
    else
    {
        PTH_RET_CHK(pthread_mutex_lock(&pTempNode->pTemInfo->mutex));
        u32AddrOffSide = pstBufPartAddr - pstBufHeadAddr;
        if (pTempNode->pTemAttr->stTemBuf.u32TemBufferSize != 0 && u32AddrOffSide < pTempNode->pTemAttr->stTemBuf.u32TemBufferSize)
        {
            memcpy(pstBufPartAddr, pTempNode->pTemAttr->stTemBuf.pTemBuffer + u32AddrOffSide, u32DataSize);
        }
        else
        {
            ERROR("######Your part of tem buf data addr is error!!!\n");
        }
        PTH_RET_CHK(pthread_mutex_unlock(&pTempNode->pTemInfo->mutex));
    }

    return 0;
}

//end###################################################
#endif
