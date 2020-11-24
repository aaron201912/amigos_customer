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
#include <string.h>

#include "mi_common.h"
#include "mi_sys.h"
#ifdef INTERFACE_VENC
#include "mi_venc.h"
#endif
#ifdef INTERFACE_VDEC
#include "mi_vdec.h"
#endif
#ifdef INTERFACE_AI
#include "mi_ai.h"
#endif
#ifdef INTERFACE_AO
#include "mi_ao.h"
#endif

#include "sys.h"
#ifdef INTERFACE_VENC
#include "venc.h"
#endif
#ifdef INTERFACE_AI
#include "ai.h"
#endif

std::map<std::string, Sys *> Sys::connectMap;
std::map<std::string, unsigned int> Sys::connectIdMap;
std::vector<Sys *> Sys::connectOrder;
dictionary * Sys::m_pstDict = NULL;
pthread_mutex_t Sys::gstUsrMutex = PTHREAD_MUTEX_INITIALIZER;
std::map<unsigned int, unsigned int> Sys::mapSysModuleType;

typedef struct stSys_BindInfo_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
#ifndef SSTAR_CHIP_I2
    MI_SYS_BindType_e eBindType;
    MI_U32 u32BindParam;
#endif
} stSys_BindInfo_T;
static MI_S32 Sys_Init(void)
{
    MI_SYS_Version_t stVersion;
    MI_U64 u64Pts = 0;

    STCHECKRESULT(MI_SYS_Init());

    memset(&stVersion, 0x0, sizeof(MI_SYS_Version_t));
    STCHECKRESULT(MI_SYS_GetVersion(&stVersion));
    AMIGOS_INFO("u8Version:%s\n", stVersion.u8Version);

    STCHECKRESULT(MI_SYS_GetCurPts(&u64Pts));
    AMIGOS_INFO("u64Pts:0x%llx\n", u64Pts);

    u64Pts = 0xF1237890F1237890;
    STCHECKRESULT(MI_SYS_InitPtsBase(u64Pts));

    u64Pts = 0xE1237890E1237890;
    STCHECKRESULT(MI_SYS_SyncPts(u64Pts));

    return MI_SUCCESS;
}

static MI_S32 Sys_Exit(void)
{
    STCHECKRESULT(MI_SYS_Exit());

    return MI_SUCCESS;
}


int Sys::GetIniInt(std::string section, std::string key, int intDefault)
{
    std::string strTmp;

    if (!m_pstDict)
    {
        AMIGOS_ERR("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

   //AMIGOS_INFO("[%s]get str [%s]\n", __FUNCTION__, strTmp.c_str());
    return iniparser_getint(m_pstDict, strTmp.c_str(), intDefault);
}
unsigned int Sys::GetIniUnsignedInt(std::string section, std::string key, unsigned int uintDefault)
{
    std::string strTmp;

    if (!m_pstDict)
    {
        AMIGOS_ERR("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    //printf("[%s]get str [%s]\n", __FUNCTION__, strTmp.c_str());
    return iniparser_getunsignedint(m_pstDict, strTmp.c_str(), uintDefault);
}
char* Sys::GetIniString(std::string section, std::string key, char *pDefaultStr)
{
    std::string strTmp;

    if (!m_pstDict)
    {
        AMIGOS_ERR("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    //printf("[%s]get str [%s]\n", __FUNCTION__, strTmp.c_str());
    return iniparser_getstring(m_pstDict, strTmp.c_str(), pDefaultStr);
}
void Sys::InitSys(std::string strIniPath, std::map<std::string, unsigned int> &mapModId)
{
    std::map<std::string, Sys *> maskMap;

    CreateObj(strIniPath, mapModId);
    maskMap.clear();
    Begin(maskMap);
}
void Sys::DeinitSys()
{
    std::map<std::string, Sys *> maskMap;

    maskMap.clear();
    End(maskMap);
    DestroyObj();
}
void Sys::CreateObj(std::string strIniPath, std::map<std::string, unsigned int> &mapModId)
{
    unsigned int i = 0;
    Sys *pClass = NULL;
    SysAutoLock AutoLock(gstUsrMutex);

    if (!m_pstDict)
    {
        m_pstDict = iniparser_load(strIniPath.c_str());
        if (!m_pstDict)
        {
            AMIGOS_ERR("INI file: [%s] read error!\n", strIniPath.c_str());
            return;
        }
    }
    connectIdMap = mapModId;
    SetupModuleType();
    CreateConnection();
    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        AMIGOS_INFO("LoadDb: %s\n", pClass->stModDesc.modKeyString.c_str());
        pClass->LoadDb();
    }
    Sys_Init();
}
void Sys::DestroyObj()
{
    SysAutoLock AutoLock(gstUsrMutex);

    if (!m_pstDict)
    {
        return;
    }
    Sys_Exit();
    DestroyConnection();
    connectIdMap.clear();
    mapSysModuleType.clear();
    if (m_pstDict)
    {
        iniparser_freedict(m_pstDict);
        m_pstDict = NULL;
    }
}
void Sys::Begin(std::map<std::string, Sys *> &maskMap)
{
    unsigned int i = 0;
    Sys *pClass = NULL;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapOut;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    SysAutoLock AutoLock(gstUsrMutex);

    //init
    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        AMIGOS_INFO("init %s\n", pClass->stModDesc.modKeyString.c_str());
        pClass->Init();
    }

    //bind
    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        for (itMapIn = pClass->mapModInputInfo.begin(); itMapIn != pClass->mapModInputInfo.end(); ++itMapIn)
        {
            if (maskMap.find(itMapIn->second.stPrev.modKeyString) != maskMap.end())
            {
                continue;
            }
            pClass->BindBlock(itMapIn->second);
        }
    }

    //start
    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        pClass->Start();
    }

}
void Sys::End(std::map<std::string, Sys *> &maskMap)
{
    Sys *pClass = NULL;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    unsigned int i = 0;
    SysAutoLock AutoLock(gstUsrMutex);

    //stop
    for (i = connectOrder.size(); i != 0; i--)
    {
        pClass = connectOrder[i - 1];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        pClass->Stop();
    }

    //unbind
    for (i = connectOrder.size(); i != 0; i--)
    {
        pClass = connectOrder[i - 1];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        for (itMapIn = pClass->mapModInputInfo.begin(); itMapIn != pClass->mapModInputInfo.end(); ++itMapIn)
        {
            if (maskMap.find(itMapIn->second.stPrev.modKeyString) != maskMap.end())
            {
                continue;
            }
            pClass->UnBindBlock(itMapIn->second);
        }
    }

    //deinit
    for (i = connectOrder.size(); i != 0; i--)
    {
        pClass = connectOrder[i - 1];
        if (maskMap.find(pClass->stModDesc.modKeyString) != maskMap.end())
        {
            continue;
        }
        pClass->Deinit();
        AMIGOS_INFO("deinit %s\n", pClass->stModDesc.modKeyString.c_str());
    }

}
void Sys::SwtichSrc(Sys *srcObj, unsigned int srcOutPort, Sys *srcObjNew, unsigned int srcOutPortNew, Sys *dstObj, unsigned int dstInPort)
{
    stModIoInfo_t stIoInfo;
    stReceiverPortDesc_t *pstReceiverPortDesc = NULL;
    stReceiverPortDesc_t stReceiverPortDesc;
    unsigned int i = 0;
    SysAutoLock AutoLock(gstUsrMutex);
    SysAutoLock SwtichSrc(dstObj->gstSwitchSrcMutex);

    ASSERT(srcObj);
    ASSERT(dstObj);

    if (dstObj->mapModInputInfo.find(dstInPort) == dstObj->mapModInputInfo.end())
    {
        AMIGOS_ERR("Dst in port not found !\n");

        return;
    }
    if (srcObj->mapModOutputInfo.find(srcOutPort) == srcObj->mapModOutputInfo.end())
    {
        AMIGOS_ERR("Src out port not found !\n");

        return;
    }
    for (i = 0; i < srcObj->mapModOutputInfo[srcOutPort].vectNext.size(); i++)
    {
        if (srcObj->mapModOutputInfo[srcOutPort].vectNext[i].portId == dstObj->mapModInputInfo[dstInPort].curPortId
            && srcObj->mapModOutputInfo[srcOutPort].vectNext[i].modKeyString == dstObj->stModDesc.modKeyString)
        {
            break;
        }
    }
    if (i == srcObj->mapModOutputInfo[srcOutPort].vectNext.size())
    {
        AMIGOS_ERR("Not found connection!\n");
        return;
    }
    dstObj->UnBindBlock(dstObj->mapModInputInfo[dstInPort]);
    if (srcObj->mapRecevier.find(srcOutPort) != srcObj->mapRecevier.end())
    {

        pthread_mutex_lock(&srcObj->mapRecevier[srcOutPort].stDeliveryMutex);
        if (srcObj->mapRecevier[srcOutPort].mapPortDesc.find(dstObj->mapModInputInfo[dstInPort].curIoKeyString) != srcObj->mapRecevier[srcOutPort].mapPortDesc.end())
        {
            stReceiverPortDesc = srcObj->mapRecevier[srcOutPort].mapPortDesc[dstObj->mapModInputInfo[dstInPort].curIoKeyString];
            pstReceiverPortDesc = &stReceiverPortDesc;
            AMIGOS_INFO("Recevier exist! in port %d recv %p\n", pstReceiverPortDesc->portId, pstReceiverPortDesc->fpRec);
        }
        pthread_mutex_unlock(&srcObj->mapRecevier[srcOutPort].stDeliveryMutex);
        if (pstReceiverPortDesc)
        {
            if (stReceiverPortDesc.bStart)
            {
                dstObj->StopReceiver(dstInPort);
            }
            dstObj->DestroyReceiver(dstInPort);
        }
    }
    srcObj->mapModOutputInfo[srcOutPort].vectNext.erase(srcObj->mapModOutputInfo[srcOutPort].vectNext.begin() + i);
    dstObj->mapModInputInfo[dstInPort].stPrev.modKeyString = srcObjNew->stModDesc.modKeyString;
    dstObj->mapModInputInfo[dstInPort].stPrev.portId = srcOutPortNew;
    dstObj->mapModInputInfo[dstInPort].stPrev.frmRate = srcObjNew->mapModOutputInfo[srcOutPortNew].curFrmRate;
    stIoInfo.modKeyString = dstObj->stModDesc.modKeyString;
    stIoInfo.portId = dstInPort;
    stIoInfo.frmRate = dstObj->mapModInputInfo[dstInPort].curFrmRate;
    srcObjNew->mapModOutputInfo[srcOutPortNew].vectNext.push_back(stIoInfo);
    if (pstReceiverPortDesc)
    {
        dstObj->CreateReceiver(dstInPort, pstReceiverPortDesc->fpRec, pstReceiverPortDesc->pUsrData);
        if (pstReceiverPortDesc->bStart)
        {
            dstObj->StartReceiver(dstInPort);
        }
    }
    dstObj->BindBlock(dstObj->mapModInputInfo[dstInPort]);

}
void Sys::Extract(std::vector<Sys *> &objVect)
{
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapOut;
    std::vector<stModIoInfo_t>::iterator itOutMod;
    Sys *pNextClass = NULL;
    Sys *Object = NULL;
    unsigned int i = 0;
    SysAutoLock AutoLock(gstUsrMutex);

    for (i = objVect.size(); i != 0; i--)
    {
        Object = objVect[i - 1];
        Object->Stop();
    }
    for (i = objVect.size(); i != 0; i--)
    {

        Object = objVect[i - 1];
        if (i == objVect.size())
        {
            for (itMapOut = Object->mapModOutputInfo.begin(); itMapOut != Object->mapModOutputInfo.end(); ++itMapOut)
            {
                for (itOutMod = itMapOut->second.vectNext.begin(); itOutMod !=  itMapOut->second.vectNext.end(); ++itOutMod)
                {
                    pNextClass = GetInstance(itOutMod->modKeyString);
                    pNextClass->UnBindBlock(pNextClass->mapModInputInfo[itOutMod->portId]);
                }
            }
        }
        for (itMapIn = Object->mapModInputInfo.begin(); itMapIn != Object->mapModInputInfo.end(); ++itMapIn)
        {
            Object->UnBindBlock(itMapIn->second);
        }
    }
    for (i = objVect.size(); i != 0; i--)
    {
        Object = objVect[i - 1];
        Object->Deinit();
    }

}
void Sys::Insert(std::vector<Sys *> &objVect)
{
    std::vector<Sys *>::iterator itObjVect;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapOut;
    std::vector<stModIoInfo_t>::iterator itOutMod;
    Sys *pNextClass = NULL;
    Sys *Object = NULL;
    unsigned int uintSz = 0;
    SysAutoLock AutoLock(gstUsrMutex);

    for (itObjVect = objVect.begin(); itObjVect != objVect.end(); ++itObjVect)
    {
        Object = *itObjVect;
        Object->Init();
    }
    for (itObjVect = objVect.begin(); itObjVect != objVect.end(); ++itObjVect)
    {
        Object = *itObjVect;
        for (itMapIn = Object->mapModInputInfo.begin(); itMapIn != Object->mapModInputInfo.end(); ++itMapIn)
        {
            Object->BindBlock(itMapIn->second);
        }
        if (++uintSz == objVect.size())
        {
            for (itMapOut = Object->mapModOutputInfo.begin(); itMapOut != Object->mapModOutputInfo.end(); ++itMapOut)
            {
                for (itOutMod = itMapOut->second.vectNext.begin(); itOutMod !=  itMapOut->second.vectNext.end(); ++itOutMod)
                {
                    pNextClass = GetInstance(itOutMod->modKeyString);
                    pNextClass->BindBlock(pNextClass->mapModInputInfo[itOutMod->portId]);
                }
            }
        }
    }
    for (itObjVect = objVect.begin(); itObjVect != objVect.end(); ++itObjVect)
    {
        Object = *itObjVect;
        Object->Start();
    }
}
void Sys::SetupModuleType()
{
    mapSysModuleType[E_SYS_MOD_DISP] = E_STREAM_IN_DATA_IN_KERNEL_MODULE | E_STREAM_IN_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_VENC] = E_STREAM_IN_DATA_IN_KERNEL_MODULE | E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_VDEC] = E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_KERNEL_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_VPE] = E_STREAM_IN_DATA_IN_KERNEL_MODULE | E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_KERNEL_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_VIF] = E_STREAM_OUT_DATA_IN_KERNEL_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_DIVP] = E_STREAM_IN_DATA_IN_KERNEL_MODULE | E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_KERNEL_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_VDISP] = E_STREAM_IN_DATA_IN_KERNEL_MODULE | E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_KERNEL_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_LDC] = E_STREAM_IN_DATA_IN_KERNEL_MODULE | E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_KERNEL_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_AI] = E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_AO] = E_STREAM_IN_DATA_IN_USER_MODULE;

    mapSysModuleType[E_SYS_MOD_DLA] = E_STREAM_IN_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_FDFR] = E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_UI] = E_STREAM_IN_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_FILE] = E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_RTSP] = E_STREAM_IN_DATA_IN_USER_MODULE | E_STREAM_OUT_DATA_IN_USER_MODULE;

#ifndef SSTAR_CHIP_I2
    mapSysModuleType[E_SYS_MOD_SNR] = 0;
#endif
    mapSysModuleType[E_SYS_MOD_SIGNAL_MONITOR] = 0;
    mapSysModuleType[E_SYS_MOD_IQ] = 0;
    mapSysModuleType[E_SYS_MOD_SLOT] = 0;
    mapSysModuleType[E_SYS_MOD_UAC] = E_STREAM_IN_DATA_IN_USER_MODULE;
    mapSysModuleType[E_SYS_MOD_UVC] = E_STREAM_IN_DATA_IN_USER_MODULE;
}
void Sys::CreateConnection()
{
    std::map<unsigned int, std::string>::iterator it;
    std::map<unsigned int, std::string> mapRootKeyStr;
    int rootCnt;
    char root[30];
    std::string rootName;
    int i = 0;

    AMIGOS_INFO("%d\n",iniparser_getint(m_pstDict, "ROOT:COUNT", -1));
    rootCnt = iniparser_getint(m_pstDict, "ROOT:COUNT", -1);
    for(i = 0; i < rootCnt; i++)
    {
        sprintf(root,"ROOT:NAME_%d",i);
        rootName = iniparser_getstring(m_pstDict, root, NULL);
        AMIGOS_INFO("root %s\n",rootName.c_str());
        mapRootKeyStr[i] = rootName;
    }
    for (it = mapRootKeyStr.begin(); it != mapRootKeyStr.end(); ++it)
    {
        Implement(it->second);
    }
}
void Sys::DestroyConnection(void)
{
    Sys *pClass = NULL;
    unsigned int i = 0;

    for (i = 0; i < connectOrder.size(); i++)
    {
        pClass = connectOrder[i];
        pClass->mapModInputInfo.clear();
        pClass->mapModOutputInfo.clear();
        delete (pClass);
        connectOrder[i] = NULL;
    }
    connectMap.clear();
    connectOrder.clear();
}
void Sys::SetCurInfo(std::string &strKey)
{
    unsigned int inCnt = 0;
    unsigned int outCnt = 0;
    unsigned int count = 0;
    int tmpPos = 0;
    char tmpStr[20];
    char *pRes = NULL;
    unsigned int i = 0;
    std::string strTempString;
    std::string strTempPrevPort;
    stModInputInfo_t stInputInfo;
    stModOutputInfo_t stOutputInfo;

    inCnt = GetIniUnsignedInt(strKey, "IN_CNT");
    outCnt = GetIniUnsignedInt(strKey, "OUT_CNT");
    stModDesc.modKeyString = strKey;
    stModDesc.modId = FindBlockId(strKey);
    stModDesc.chnId = GetIniUnsignedInt(strKey, "CHN");
    stModDesc.devId = GetIniUnsignedInt(strKey, "DEV");
    if (inCnt)
    {
        for (i = 0; i < MAX_INPUT_CNT; i++)
        {
            memset(tmpStr, 0, 20);
            sprintf(tmpStr, "IN_%d", i);
            pRes = GetIniString(strKey, tmpStr);
            if(pRes != NULL)
            {
                stInputInfo.curIoKeyString = pRes;
                strTempString = GetIniString(pRes, "PREV");
                tmpPos = strTempString.find_first_of(':');
                strTempPrevPort = strTempString;
                strTempPrevPort.erase(0, tmpPos + 1);
                stInputInfo.stPrev.modKeyString = strTempString.erase(tmpPos, strTempString.size() - tmpPos);

                strTempString.erase();
                strTempString = GetIniString(stInputInfo.stPrev.modKeyString, strTempPrevPort.c_str());
                stInputInfo.stPrev.portId = GetIniUnsignedInt(strTempString, "PORT");
                stInputInfo.stPrev.frmRate = GetIniUnsignedInt(strTempString, "FPS");
                stInputInfo.curPortId = GetIniUnsignedInt(pRes, "PORT");
                stInputInfo.curFrmRate = GetIniUnsignedInt(pRes, "FPS");
                mapModInputInfo[stInputInfo.curPortId] = stInputInfo;
                count++;
            }
            if(count == inCnt)
            {
                break;
            }
        }
    }
    if (outCnt)
    {
        count = 0;
        for (i = 0; i < MAX_OUTPUT_CNT; i++)
        {
            memset(tmpStr, 0, 20);
            sprintf(tmpStr, "OUT_%d", i);
            pRes = GetIniString(strKey, tmpStr);
            if(pRes != NULL)
            {
                stOutputInfo.curIoKeyString = pRes;
                stOutputInfo.curPortId = GetIniUnsignedInt(pRes, "PORT");
                stOutputInfo.curFrmRate = GetIniUnsignedInt(pRes, "FPS");
                mapModOutputInfo[stOutputInfo.curPortId] = stOutputInfo;
                count++;
            }
            if(count == outCnt)
            {
                break;
            }
        }
    }
    connectMap[stModDesc.modKeyString] = this;

    return;
}
void Sys::BuildModTree()
{
    Sys *pPreClass;
    stModIoInfo_t stIoInfo;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapInput;

    for (itMapInput = mapModInputInfo.begin(); itMapInput != mapModInputInfo.end(); ++itMapInput)
    {
        Implement(itMapInput->second.stPrev.modKeyString);
        pPreClass =  GetInstance(itMapInput->second.stPrev.modKeyString);
        if (pPreClass)
        {
            stIoInfo.modKeyString = stModDesc.modKeyString;
            stIoInfo.portId = itMapInput->second.curPortId;
            stIoInfo.frmRate = itMapInput->second.curFrmRate;
            pPreClass->mapModOutputInfo[itMapInput->second.stPrev.portId].vectNext.push_back(stIoInfo);
        }
    }
    connectOrder.push_back(this);

}
void Sys::Start()
{
}
void Sys::Stop()
{
}
void Sys::PrevExtBind(stModInputInfo_t & stIn)
{
    CreateReceiver(stIn.curPortId, DataReceiver, this);
    StartReceiver(stIn.curPortId);
}
void Sys::PrevIntBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc)
{
    stSys_BindInfo_T stBindInfo;

    memset(&stBindInfo, 0x0, sizeof(stSys_BindInfo_T));
#ifndef SSTAR_CHIP_I2
    stBindInfo.eBindType = (MI_SYS_BindType_e)GetIniInt(stIn.curIoKeyString, "BIND_TYPE");
    stBindInfo.u32BindParam = GetIniInt(stIn.curIoKeyString, "BIND_PARAM");
#endif
    stBindInfo.stSrcChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId ;
    stBindInfo.stSrcChnPort.u32DevId = stPreDesc.devId;
    stBindInfo.stSrcChnPort.u32ChnId = stPreDesc.chnId;
    stBindInfo.stSrcChnPort.u32PortId = stIn.stPrev.portId;
    stBindInfo.u32SrcFrmrate = stIn.stPrev.frmRate;
    stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)stModDesc.modId;
    stBindInfo.stDstChnPort.u32DevId = stModDesc.devId;
    stBindInfo.u32DstFrmrate = stIn.curFrmRate;
    stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
    stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
#ifndef SSTAR_CHIP_I2
    MI_SYS_BindChnPort2(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate, stBindInfo.eBindType, stBindInfo.u32BindParam);
#else
    MI_SYS_BindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate);
#endif
}
void Sys::PrevExtUnBind(stModInputInfo_t & stIn)
{
    StopReceiver(stIn.curPortId);
    DestroyReceiver(stIn.curPortId);
}
void Sys::PrevIntUnBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc)
{
    stSys_BindInfo_T stBindInfo;

    memset(&stBindInfo, 0x0, sizeof(stSys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId ;
    stBindInfo.stSrcChnPort.u32DevId = stPreDesc.devId;
    stBindInfo.stSrcChnPort.u32ChnId = stPreDesc.chnId;
    stBindInfo.stSrcChnPort.u32PortId = stIn.stPrev.portId;
    stBindInfo.u32SrcFrmrate = stIn.stPrev.frmRate;
    stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)stModDesc.modId;
    stBindInfo.stDstChnPort.u32DevId = stModDesc.devId;
    stBindInfo.u32DstFrmrate = stIn.curFrmRate;
    stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
    stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
    MI_SYS_UnBindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort);
}

void Sys::BindBlock(stModInputInfo_t &stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    AMIGOS_INFO("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

    if ((mapSysModuleType[stModDesc.modId] & E_STREAM_IN_DATA_IN_KERNEL_MODULE)
        && (mapSysModuleType[stPreDesc.modId] & E_STREAM_OUT_DATA_IN_KERNEL_MODULE))
    {
        PrevIntBind(stIn, stPreDesc);
    }
    else if ((mapSysModuleType[stModDesc.modId] & E_STREAM_IN_DATA_IN_USER_MODULE)
        && (mapSysModuleType[stPreDesc.modId] & E_STREAM_OUT_DATA_IN_USER_MODULE))
    {
        PrevExtBind(stIn);
    }
    else
    {
        AMIGOS_INFO("Error bind solution!\n");
        ASSERT(0);
    }
}
void Sys::UnBindBlock(stModInputInfo_t &stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    AMIGOS_INFO("UnBind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

    if ((mapSysModuleType[stModDesc.modId] & E_STREAM_IN_DATA_IN_KERNEL_MODULE)
        && (mapSysModuleType[stPreDesc.modId] & E_STREAM_OUT_DATA_IN_KERNEL_MODULE))
    {
        PrevIntUnBind(stIn, stPreDesc);
    }
    else if ((mapSysModuleType[stModDesc.modId] & E_STREAM_IN_DATA_IN_USER_MODULE)
        && (mapSysModuleType[stPreDesc.modId] & E_STREAM_OUT_DATA_IN_USER_MODULE))
    {
        PrevExtUnBind(stIn);
    }
    else
    {
        AMIGOS_INFO("Error unbind solution!\n");
        ASSERT(0);
    }
}


int Sys::CreateSender(unsigned int outPortId)
{
    ST_TEM_ATTR stTemAttr;
    stReceiverDesc_t stDest;

    PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
    memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
    stTemAttr.fpThreadDoSignal = NULL;
    stTemAttr.fpThreadWaitTimeOut = SenderMonitor;
    stTemAttr.u32ThreadTimeoutMs = 0;
    stTemAttr.bSignalResetTimer = 0;
    stTemAttr.stTemBuf.pTemBuffer = (void *)&(mapRecevier[outPortId]);
    stTemAttr.stTemBuf.u32TemBufferSize = 0;
    stTemAttr.maxEventCout = 30;
    stTemAttr.bDropEvent = FALSE;
    TemOpen(mapModOutputInfo[outPortId].curIoKeyString.c_str(), stTemAttr);

    return 0;
}
int Sys::StartSender(unsigned int outPortId)
{
    TemStartMonitor(mapModOutputInfo[outPortId].curIoKeyString.c_str());

    return 0;
}
int Sys::StopSender(unsigned int outPortId)
{
    ASSERT(mapModOutputInfo.find(outPortId) != mapModOutputInfo.end());
    if (mapModOutputInfo[outPortId].bSenderConnect == 1)
    {
        Disconnect(outPortId);
        mapModOutputInfo[outPortId].bSenderConnect = 0;
    }
    TemStop(mapModOutputInfo[outPortId].curIoKeyString.c_str());

    return 0;
}
int Sys::DestroySender(unsigned int outPortId)
{
    TemClose(mapModOutputInfo[outPortId].curIoKeyString.c_str());

    return 0;
}
int Sys::Send(unsigned int outPortId, void *pData, unsigned int intDataSize)
{
    std::map<std::string, stReceiverPortDesc_t>::iterator it;
    std::map<std::string, stReceiverPortDesc_t> *pMap = &mapRecevier[outPortId].mapPortDesc;

    pthread_mutex_lock(&mapRecevier[outPortId].stDeliveryMutex);
    for (it = pMap->begin(); it != pMap->end(); ++it)
    {
        if (it->second.bStart)
        {
            it->second.fpRec(pData, intDataSize, it->second.pUsrData, it->second.portId);
        }
    }
    pthread_mutex_unlock(&mapRecevier[outPortId].stDeliveryMutex);

    return 0;
}
int Sys::Connect(unsigned int outPortId, stStreamInfo_t *pInfo)
{
    Sys *pCurClass = NULL;
    Sys *pNextClass = NULL;
    std::map<std::string, stReceiverPortDesc_t>::iterator it;
    std::map<std::string, stReceiverPortDesc_t> *pMap = &mapRecevier[outPortId].mapPortDesc;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapOut;

    pthread_mutex_lock(&mapRecevier[outPortId].stDeliveryMutex);
    for (it = pMap->begin(); it != pMap->end(); ++it)
    {
        pCurClass = it->second.pSysClass;
        for (itMapOut = pCurClass->mapModOutputInfo.begin(); itMapOut != pCurClass->mapModOutputInfo.end(); ++itMapOut)
        {
            for (unsigned int i = 0; i < itMapOut->second.vectNext.size(); i++)
            {
                pNextClass = GetInstance(itMapOut->second.vectNext[i].modKeyString);
                if (pNextClass)
                    pNextClass->UnBindBlock(pNextClass->mapModInputInfo[itMapOut->second.vectNext[i].portId]);
            }
        }
        pCurClass->Incoming(pInfo);
        for (itMapOut = pCurClass->mapModOutputInfo.begin(); itMapOut != pCurClass->mapModOutputInfo.end(); ++itMapOut)
        {
            for (unsigned int i = 0; i < itMapOut->second.vectNext.size(); i++)
            {
                pNextClass = GetInstance(itMapOut->second.vectNext[i].modKeyString);
                if (pNextClass)
                    pNextClass->BindBlock(pNextClass->mapModInputInfo[itMapOut->second.vectNext[i].portId]);
            }
        }
    }
    pthread_mutex_unlock(&mapRecevier[outPortId].stDeliveryMutex);

    return 0;
}
int Sys::Disconnect(unsigned int outPortId)
{
    std::map<std::string, stReceiverPortDesc_t>::iterator it;
    std::map<std::string, stReceiverPortDesc_t> *pMap = &mapRecevier[outPortId].mapPortDesc;

    pthread_mutex_lock(&mapRecevier[outPortId].stDeliveryMutex);
    for (it = pMap->begin(); it != pMap->end(); ++it)
    {
        it->second.pSysClass->Outcoming();
    }
    pthread_mutex_unlock(&mapRecevier[outPortId].stDeliveryMutex);

    return 0;
}

int Sys::CreateReceiver(unsigned int inPortId, DeliveryRecFp funcRecFp, void *pUsrData)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;
    stReceiverDesc_t stReceiverDesc;
    stReceiverPortDesc_t stReceiverPortDesc;

    if (!funcRecFp)
    {
        AMIGOS_ERR("funcRecFp is null!\n");

        return -1;
    }
    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        AMIGOS_ERR("Can not find input port %d\n", inPortId);
        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        AMIGOS_ERR("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    stReceiverPortDesc.bStart = FALSE;
    stReceiverPortDesc.fpRec = funcRecFp;
    stReceiverPortDesc.pUsrData = pUsrData;
    stReceiverPortDesc.portId = inPortId;
    stReceiverPortDesc.pSysClass = this;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) == pPrevClass->mapRecevier.end())
    {
        stReceiverDesc.mapPortDesc[mapModInputInfo[inPortId].curIoKeyString] = stReceiverPortDesc;
        stReceiverDesc.pSysClass = pPrevClass;
        stReceiverDesc.uintPort = intPrevOutPort;
        stReceiverDesc.stDeliveryMutex = PTHREAD_MUTEX_INITIALIZER;
        stReceiverDesc.uintRefsCnt = 0;
        pPrevClass->mapRecevier[intPrevOutPort] = stReceiverDesc;
        pPrevClass->CreateSender(intPrevOutPort);
    }
    else
    {
        pthread_mutex_lock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString] = stReceiverPortDesc;
        pthread_mutex_unlock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
    }

    return 0;
}
int Sys::StartReceiver(unsigned int inPortId)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        AMIGOS_ERR("Can not find input port %d\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        AMIGOS_ERR("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) != pPrevClass->mapRecevier.end())
    {
        pthread_mutex_lock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString].bStart = TRUE;
        pthread_mutex_unlock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        if (pPrevClass->mapRecevier[intPrevOutPort].uintRefsCnt == 0)
        {
            pPrevClass->StartSender(intPrevOutPort);
        }
        pPrevClass->mapRecevier[intPrevOutPort].uintRefsCnt++;
    }
    else
    {
        AMIGOS_ERR("Receiver did not create. inpot id %d current %s prev %s\n", inPortId, mapModInputInfo[inPortId].curIoKeyString.c_str(), mapModInputInfo[inPortId].stPrev.modKeyString.c_str());

        return -1;
    }

    return 0;
}
int Sys::StopReceiver(unsigned int inPortId)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;
    stReceiverDesc_t stReceiverDesc;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        AMIGOS_ERR("Can not find input port %d\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        AMIGOS_ERR("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) != pPrevClass->mapRecevier.end())
    {
        pthread_mutex_lock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        if (pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.find(mapModInputInfo[inPortId].curIoKeyString) != pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.end())
        {
            pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc[mapModInputInfo[inPortId].curIoKeyString].bStart = FALSE;
        }
        pthread_mutex_unlock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        if (pPrevClass->mapRecevier[intPrevOutPort].uintRefsCnt)
        {
            pPrevClass->mapRecevier[intPrevOutPort].uintRefsCnt--;
        }
        if (pPrevClass->mapRecevier[intPrevOutPort].uintRefsCnt == 0)
        {
            pPrevClass->StopSender(intPrevOutPort);
        }
    }
    else
    {
        AMIGOS_ERR("Receiver did not create. inpot id %d current %s prev %s\n", inPortId, mapModInputInfo[inPortId].curIoKeyString.c_str(), mapModInputInfo[inPortId].stPrev.modKeyString.c_str());

        return -1;
    }

    return 0;
}
int Sys::DestroyReceiver(unsigned int inPortId)
{
    Sys *pPrevClass = NULL;
    unsigned int intPrevOutPort = 0;
    unsigned char bDestroySender = FALSE;
    std::map<unsigned int, stReceiverDesc_t>::iterator it;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        AMIGOS_ERR("Can not find input port %d\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        printf("Prev class is null!\n");

        return -1;
    }
    intPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapRecevier.find(intPrevOutPort) != pPrevClass->mapRecevier.end())
    {
        pthread_mutex_lock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        if (pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.find(mapModInputInfo[inPortId].curIoKeyString) != pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.end())
        {
            pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.erase(mapModInputInfo[inPortId].curIoKeyString);
        }
        if (pPrevClass->mapRecevier[intPrevOutPort].mapPortDesc.size() == 0)
        {
            bDestroySender = TRUE;
        }
        pthread_mutex_unlock(&pPrevClass->mapRecevier[intPrevOutPort].stDeliveryMutex);
        if (bDestroySender)
        {
            pPrevClass->DestroySender(intPrevOutPort);
            pPrevClass->mapRecevier.erase(intPrevOutPort);
        }
    }

    return 0;
}
int Sys::GetInputStreamInfo(unsigned int inPortId, stStreamInfo_t *pInfo)
{
    unsigned int uintPrevOutPort = 0;
    Sys *pPrevClass = NULL;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        AMIGOS_ERR("Input port %d not found!!!\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        AMIGOS_ERR("Prev class is null!\n");

        return -1;
    }
    uintPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapModOutputInfo.find(uintPrevOutPort) == pPrevClass->mapModOutputInfo.end())
    {
        AMIGOS_ERR("Output port %d not found!!!\n", inPortId);

        return -1;
    }
    *pInfo = pPrevClass->mapModOutputInfo[uintPrevOutPort].stStreanInfo;

    return 0;
}

int Sys::UpdateInputStreamInfo(unsigned int inPortId, stStreamInfo_t *pInfo)
{
    unsigned int uintPrevOutPort = 0;
    Sys *pPrevClass = NULL;

    if (mapModInputInfo.find(inPortId) == mapModInputInfo.end())
    {
        AMIGOS_ERR("Input port %d not found!!!\n", inPortId);

        return -1;
    }
    pPrevClass = GetInstance(mapModInputInfo[inPortId].stPrev.modKeyString);
    if (!pPrevClass)
    {
        AMIGOS_ERR("Prev class is null!\n");

        return -1;
    }
    UnBindBlock(mapModInputInfo[inPortId]);
    uintPrevOutPort = mapModInputInfo[inPortId].stPrev.portId;
    if (pPrevClass->mapModOutputInfo.find(uintPrevOutPort) == pPrevClass->mapModOutputInfo.end())
    {
        AMIGOS_ERR("Output port %d not found!!!\n", inPortId);

        return -1;
    }
    pPrevClass->ResetOut(uintPrevOutPort, pInfo);
    pPrevClass->mapModOutputInfo[uintPrevOutPort].stStreanInfo = *pInfo;
    BindBlock(mapModInputInfo[inPortId]);

    return 0;
}
void * Sys::SenderMonitor(ST_TEM_BUFFER stBuf)
{
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle;
    MI_S32 s32Fd;
    MI_S32 s32Ret;
    fd_set read_fds;
    struct timeval tv;
#ifdef INTERFACE_VENC
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack[16];
    MI_VENC_ChnStat_t stStat;
#endif
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)stBuf.pTemBuffer;
    Sys *pClass = pReceiver->pSysClass;
    stStreamData_t stStreamData;

    ASSERT(pClass);
    memset(&stStreamData, 0, sizeof(stStreamData_t));
    stChnOutputPort.eModId = (MI_ModuleId_e)pClass->stModDesc.modId;
    stChnOutputPort.u32DevId = (MI_U32)pClass->stModDesc.devId;
    stChnOutputPort.u32ChnId = (MI_U32)pClass->stModDesc.chnId;
    stChnOutputPort.u32PortId = (MI_U32)pReceiver->uintPort;
    switch (pClass->stModDesc.modId)
    {
        case E_SYS_MOD_VPE:
        case E_SYS_MOD_DIVP:
        case E_SYS_MOD_VDISP:
        case E_SYS_MOD_LDC:
        case E_SYS_MOD_VIF:
        case E_SYS_MOD_VDEC:
        {
            s32Ret = MI_SYS_GetFd(&stChnOutputPort, &s32Fd);
            if (s32Ret < 0)
            {
                return NULL;
            }

            FD_ZERO(&read_fds);
            FD_SET(s32Fd, &read_fds);
            tv.tv_sec = 0;
            tv.tv_usec = 10 * 1000;
            s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
            if (s32Ret < 0)
            {
                MI_SYS_CloseFd(s32Fd);

                return NULL;
            }
            else if (0 == s32Ret)
            {
                MI_SYS_CloseFd(s32Fd);

                return NULL;
            }
            if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stChnOutputPort , &stBufInfo,&hHandle))
            {
                stStreamData.stInfo.eStreamType = (E_STREAM_TYPE)stBufInfo.stFrameData.ePixelFormat;
                stStreamData.stInfo.stFrameInfo.streamWidth = stBufInfo.stFrameData.u16Width;
                stStreamData.stInfo.stFrameInfo.streamHeight = stBufInfo.stFrameData.u16Height;
                if(stStreamData.stInfo.eStreamType == E_STREAM_YUV420)
                {
                    stStreamData.stYuvSpData.pYdataAddr = (char*)stBufInfo.stFrameData.pVirAddr[0];
                    stStreamData.stYuvSpData.pUvDataAddr = (char*)stBufInfo.stFrameData.pVirAddr[1];
                }
                else if(stStreamData.stInfo.eStreamType == E_STREAM_YUV422)
                {
                    stStreamData.pYuvData = (char*)stBufInfo.stFrameData.pVirAddr[0];
                }
                pClass->Send(pReceiver->uintPort, &stStreamData, sizeof(stStreamData_t));
                MI_SYS_ChnOutputPortPutBuf(hHandle);
            }
            MI_SYS_CloseFd(s32Fd);
        }
        break;
#ifdef INTERFACE_AI
        case E_SYS_MOD_AI:
        {
            MI_AUDIO_Frame_t stFrm;
            MI_AUDIO_AecFrame_t stAecFrm;
            stAiInfo_t stAiInfo;
            Ai *pAiClass = dynamic_cast<Ai*>(pClass);

            ASSERT(pAiClass);
            memset(&stFrm, 0, sizeof(MI_AUDIO_Frame_t));
            memset(&stAecFrm, 0, sizeof(MI_AUDIO_AecFrame_t));
            if (MI_SUCCESS == MI_AI_GetFrame((MI_AUDIO_DEV)pClass->stModDesc.devId, (MI_AI_CHN)pClass->stModDesc.chnId, &stFrm, &stAecFrm, 40))
            {
                stStreamData.stInfo.eStreamType = E_STREAM_PCM;
#ifndef SSTAR_CHIP_I2
                stStreamData.stPcmData.pData = (char *)stFrm.apSrcPcmVirAddr[0];
                stStreamData.stPcmData.uintSize = stFrm.u32SrcPcmLen;
#else
                stStreamData.stPcmData.pData = (char *)stFrm.apVirAddr[0];
                stStreamData.stPcmData.uintSize = stFrm.u32Len;
#endif
                pAiClass->GetInfo(stAiInfo);
                stStreamData.stInfo.stPcmInfo.uintBitLength = stAiInfo.uintBitWidth;
                stStreamData.stInfo.stPcmInfo.uintBitRate = stAiInfo.uintSampleRate;
                ASSERT(pClass->mapModOutputInfo.find(pReceiver->uintPort) != pClass->mapModOutputInfo.end());
                if (pClass->mapModOutputInfo[pReceiver->uintPort].bSenderConnect == 0)
                {
                    pClass->Connect(pReceiver->uintPort, &stStreamData.stInfo);
                    pClass->mapModOutputInfo[pReceiver->uintPort].bSenderConnect = 1;
                }
                pClass->Send(pReceiver->uintPort, &stStreamData, sizeof(stStreamData_t));
                MI_AI_ReleaseFrame(pClass->stModDesc.devId, pClass->stModDesc.chnId, &stFrm, &stAecFrm);
            }
        }
        break;
#endif
#ifdef INTERFACE_VENC
        case E_SYS_MOD_VENC:
        {
            s32Fd = MI_VENC_GetFd((MI_VENC_CHN)stChnOutputPort.u32ChnId);
            if (s32Fd < 0)
            {
                return NULL;
            }

            FD_ZERO(&read_fds);
            FD_SET(s32Fd, &read_fds);
            tv.tv_sec = 0;
            tv.tv_usec = 10 * 1000;
            s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
            if (s32Ret < 0)
            {
                MI_VENC_CloseFd(s32Fd);

                return NULL;
            }
            else if (0 == s32Ret)
            {
                MI_VENC_CloseFd(s32Fd);

                return NULL;
            }
            else
            {
                Venc *pVencClass = dynamic_cast<Venc*>(pClass);
                memset(&stStream, 0, sizeof(stStream));
                memset(stPack, 0, sizeof(stPack));
                stStream.pstPack = stPack;
                s32Ret = MI_VENC_Query(stChnOutputPort.u32ChnId, &stStat);
                if(s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
                {
                    MI_VENC_CloseFd(s32Fd);
                    return NULL;
                }
                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = MI_VENC_GetStream(stChnOutputPort.u32ChnId, &stStream, 40);
                if(MI_SUCCESS == s32Ret)
                {
                    stVencInfo_t stVencInfo;
                    stEsPackage_t stEsPacket[16];

                    pVencClass->GetInfo(stVencInfo);
                    stStreamData.stInfo.eStreamType = (E_STREAM_TYPE)stVencInfo.intEncodeType;
                    stStreamData.stEsData.uintPackCnt = stStream.u32PackCount;
                    for (MI_U8 i = 0; i < stStream.u32PackCount; i++)
                    {
                        stEsPacket[i].uintDataSize = stStream.pstPack[i].u32Len;
                        stEsPacket[i].pData = (char*)stStream.pstPack[i].pu8Addr;
                        stEsPacket[i].bSliceEnd = stStream.pstPack[i].bFrameEnd;
                    }
                    stStreamData.stEsData.pDataAddr = stEsPacket;
                    //AMIGOS_INFO("Receiver %p Get venc chn %d and send to port %d this is %s\n", pReceiver, stChnOutputPort.u32ChnId, pReceiver->uintPort, pClass->stModDesc.modKeyString.c_str());
                    ASSERT(pClass->mapModOutputInfo.find(pReceiver->uintPort) != pClass->mapModOutputInfo.end());
                    if (pClass->mapModOutputInfo[pReceiver->uintPort].bSenderConnect == 0)
                    {
                        pClass->Connect(pReceiver->uintPort, &stStreamData.stInfo);
                        pClass->mapModOutputInfo[pReceiver->uintPort].bSenderConnect = 1;
                    }
                    pClass->Send(pReceiver->uintPort, &stStreamData, sizeof(stStreamData_t));
                    MI_VENC_ReleaseStream(stChnOutputPort.u32ChnId, &stStream);
                }
            }
            MI_VENC_CloseFd(s32Fd);
        }
        break;
#endif
        default:
            AMIGOS_ERR("Do not support this mod %d now\n", pClass->stModDesc.modId);
            break;
    }
    return NULL;
}
#if INTERFACE_VDEC
static MI_U64 _GetPts(MI_U32 u32FrameRate)
{
    if (0 == u32FrameRate)
    {
        return (MI_U64)(-1);
    }

    return (MI_U64)(1000 / u32FrameRate);
}
#endif

void Sys::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    int y_size = 0;
    int uv_size = 0;
    int data_size = 0;
    stStreamData_t *pStreamData = NULL;
    MI_S32 s32Ret = E_MI_ERR_FAILED;

    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_ChnPort_t stSysChnInputPort;
    Sys *pInstance = NULL;

    pInstance = (Sys *)pUsrData;
    ASSERT(pInstance);
    if (sizeof(stStreamData_t) == dataSize)
    {
        pStreamData = (stStreamData_t *)pData;
        switch (pStreamData->stInfo.eStreamType)
        {
            case E_STREAM_YUV420:
            {
                memset(&stBufConf ,  0 , sizeof(stBufConf));
                memset(&stBufInfo ,  0 , sizeof(stBufInfo));
                memset(&stSysChnInputPort, 0, sizeof(stSysChnInputPort));

                stSysChnInputPort.eModId = (MI_ModuleId_e)pInstance->stModDesc.modId;
                stSysChnInputPort.u32DevId = pInstance->stModDesc.devId;
                stSysChnInputPort.u32ChnId = pInstance->stModDesc.chnId;
                stSysChnInputPort.u32PortId = portId;

                MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
                stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
                stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
                stBufConf.stFrameCfg.u16Width = pStreamData->stInfo.stFrameInfo.streamWidth;
                stBufConf.stFrameCfg.u16Height = pStreamData->stInfo.stFrameInfo.streamHeight;
                stBufConf.stFrameCfg.eFormat = (MI_SYS_PixelFormat_e)pStreamData->stInfo.eStreamType;
                if(MI_SUCCESS  == (s32Ret = MI_SYS_ChnInputPortGetBuf(&stSysChnInputPort, &stBufConf, &stBufInfo, &hHandle, 0)))
                {
                    y_size = pStreamData->stInfo.stFrameInfo.streamWidth * pStreamData->stInfo.stFrameInfo.streamHeight;
                    uv_size = y_size/2;
                    memcpy(stBufInfo.stFrameData.pVirAddr[0], pStreamData->stYuvSpData.pYdataAddr, y_size);
                    memcpy(stBufInfo.stFrameData.pVirAddr[1], pStreamData->stYuvSpData.pUvDataAddr, uv_size);
                    s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
                    if(s32Ret != MI_SUCCESS)
                    {
                        AMIGOS_ERR("put buf err is %x\n", s32Ret);
                    }
                }
                else
                {
                    AMIGOS_ERR("get port buf err is %x\n", s32Ret);
                }
            }
            break;
            case E_STREAM_YUV422:
            {
                memset(&stBufConf ,  0 , sizeof(stBufConf));
                memset(&stBufInfo ,  0 , sizeof(stBufInfo));
                memset(&stSysChnInputPort, 0, sizeof(stSysChnInputPort));

                stSysChnInputPort.eModId = (MI_ModuleId_e)pInstance->stModDesc.modId;
                stSysChnInputPort.u32DevId = pInstance->stModDesc.devId;
                stSysChnInputPort.u32ChnId = pInstance->stModDesc.chnId;
                stSysChnInputPort.u32PortId = portId;

                MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
                stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
                stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
                stBufConf.stFrameCfg.u16Width = pStreamData->stInfo.stFrameInfo.streamWidth;
                stBufConf.stFrameCfg.u16Height = pStreamData->stInfo.stFrameInfo.streamHeight;
                stBufConf.stFrameCfg.eFormat = (MI_SYS_PixelFormat_e)pStreamData->stInfo.eStreamType;
                if(MI_SUCCESS  == (s32Ret = MI_SYS_ChnInputPortGetBuf(&stSysChnInputPort, &stBufConf, &stBufInfo, &hHandle, 0)))
                {
                    data_size = pStreamData->stInfo.stFrameInfo.streamWidth * pStreamData->stInfo.stFrameInfo.streamHeight * 2;
                    memcpy(stBufInfo.stFrameData.pVirAddr[0], pStreamData->pYuvData, data_size);
                    s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
                    if(s32Ret != MI_SUCCESS)
                    {
                        AMIGOS_ERR("put buf err is %x\n", s32Ret);
                    }
                }
                else
                {
                    AMIGOS_ERR("get port buf err is %x\n", s32Ret);
                }
            }
            break;
            case E_STREAM_H264:
            case E_STREAM_H265:
            case E_STREAM_JPEG:
            {
#ifdef INTERFACE_VDEC
                unsigned int modId = pInstance->stModDesc.modId;

                if(modId == E_SYS_MOD_VDEC)
                {
                    unsigned int i=0;
                    MI_U64 u64Pts = 0;
                    MI_VDEC_VideoStream_t stVdecStream;
                    for(i = 0; i < pStreamData->stEsData.uintPackCnt; i++)
                    {
                        memset(&stVdecStream, 0x0, sizeof(stVdecStream));
                        stVdecStream.pu8Addr = (MI_U8*)pStreamData->stEsData.pDataAddr[i].pData;
                        stVdecStream.u32Len = pStreamData->stEsData.pDataAddr[i].uintDataSize;
                        stVdecStream.u64PTS = u64Pts + _GetPts(((MI_U32)30));
                        stVdecStream.bEndOfFrame = 1;
                        stVdecStream.bEndOfStream = 0;
                        //printf("Addr %p len %d\n", stVdecStream.pu8Addr, stVdecStream.u32Len);
                        s32Ret = MI_VDEC_SendStream(pInstance->stModDesc.chnId, &stVdecStream, 20);
                        if (MI_SUCCESS != s32Ret)
                        {
                            AMIGOS_ERR("MI_VDEC_SendStream fail, chn:%d, 0x%X\n", pInstance->stModDesc.chnId, s32Ret);
                        }
                    }
                }
#endif
            }
            break;
            case E_STREAM_PCM:
            {
#ifdef INTERFACE_AO
                unsigned int modId = pInstance->stModDesc.modId;
                if(modId == E_SYS_MOD_AO)
                {
                    MI_AUDIO_Frame_t stAoFrame;

                    memset(&stAoFrame, 0, sizeof(MI_AUDIO_Frame_t));
                    switch (pStreamData->stInfo.stPcmInfo.uintBitLength)
                    {
                        case 16:
                            stAoFrame.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
                            break;
                        case 24:
                            stAoFrame.eBitwidth = E_MI_AUDIO_BIT_WIDTH_24;
                            break;
                        default:
                            ASSERT(0);
                    }
                    stAoFrame.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
#ifndef SSTAR_CHIP_I2
                    stAoFrame.u32Len = stAoFrame.u32SrcPcmLen = pStreamData->stPcmData.uintSize;
#else
                    stAoFrame.u32Len = pStreamData->stPcmData.uintSize;
#endif
                    stAoFrame.apVirAddr[0] = pStreamData->stPcmData.pData;
                    //printf("Send ao p %p size %d\n", pStreamInfo->stPcmInfo.pData, stAoFrame.u32SrcPcmLen);
                    if (MI_SUCCESS != MI_AO_SendFrame((MI_AUDIO_DEV)pInstance->stModDesc.devId, (MI_AO_CHN)pInstance->stModDesc.chnId, &stAoFrame, 40))
                    {
                        AMIGOS_ERR("MI_AO_SendFrame fail, chn:%d, 0x%X\n", pInstance->stModDesc.chnId, s32Ret);
                    }
                }
#endif
            }
            break;
            default:
                AMIGOS_ERR("Not support!!\n");
                assert(0);
                break;
        }
    }
    else
    {
        assert(0);
    }
}

