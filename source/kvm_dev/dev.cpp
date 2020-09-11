#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "common.h"

#define Key_DevName     "/dev/hidg0"
#define Mouse_DevName   "/dev/hidg1"

static int server_sockfd = -1, client_sockfd = -1;
static int server_len = 0, client_len = 0;
static struct sockaddr_in server_address, client_address;

static int key_fd = -1;
static int mouse_fd = -1;

static ST_INTELL_TransferInfo_t stData;

static pthread_t Worktid;

#ifdef DEBUG
static int idx = 0;
#endif

static bool HidgDevSockInit(void)
{
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sockfd == -1)
    {
        perror("socket");
        return false;
    }

    int on = 1;
    if(-1 == setsockopt(server_sockfd , SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        perror("setsockopt");
        return false;
    }
    if(-1 == setsockopt(server_sockfd , IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)))
    {
        perror("setsockopt");
        return false;
    }

    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9000);
    server_len = sizeof(server_address);

    if(-1 == bind(server_sockfd, (struct sockaddr *)&server_address, server_len))
    {
        perror("bind");
        return false;
    }

    if(-1 == listen(server_sockfd, 5))
    {
        perror("listen");
        return false;
    }

    return true;
}

static void HidgDevSockDeinit(void)
{
    close(client_sockfd);
    close(server_sockfd);
    client_sockfd = -1;
    server_sockfd = -1;
}

static int HidgDevSocketWaitData(ST_INTELL_TransferInfo_t *pTransData)
{
    int retVal = 0;

    if(client_sockfd == -1)
    {
        printf("Server is waiting for client connect...\n");
        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&server_address, (socklen_t *)&client_len);
        if(client_sockfd == -1)
        {
            perror("accept");
            return -1;
        }
    }

#ifdef DEBUG
    struct timeval tv1;
    struct timezone tz1;
    struct timeval tv2;
    struct timezone tz2;
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf("HidgDevSocketWaitData:%d\n", idx++);
    if(gettimeofday(&tv1, &tz1)){
        perror("gettimeofday1 err\n");
        return -1;
    }
    printf("before recv:tv_sec=%d \t tv_usec=%d \n", tv1.tv_sec, tv1.tv_usec);
#endif

    retVal = recv(client_sockfd, &pTransData->u32BufSize, sizeof(int), 0);
    if(retVal != sizeof(int))
    {
        perror("recv");
        if(retVal == 0)
        {
            printf("Client disconnect!\n");
            HidgDevSockDeinit();
            HidgDevSockInit();
        }
        return -1;
    }

    retVal = recv(client_sockfd, pTransData->pVirAddr, (pTransData->u32BufSize & Clear_Mask), 0);
    if(retVal != (pTransData->u32BufSize & Clear_Mask))
    {
        perror("recv");
        if(retVal == 0)
        {
            printf("Client disconnect!\n");
            HidgDevSockDeinit();
            HidgDevSockInit();
        }
        return -1;
    }

#ifdef DEBUG
    if(gettimeofday(&tv2, &tz2)){
        perror("gettimeofday1 err\n");
        return -1;
    }
    printf("after recv:tv_sec=%d \t tv_usec=%d \n", tv2.tv_sec, tv2.tv_usec);
    printf("pTransData->u32BufSize = %d\n", pTransData->u32BufSize);
    for(int i=0; i<(pTransData->u32BufSize & Clear_Mask); i++)
    {
        printf("[%d]:%d\t", i, pTransData->pVirAddr[i]);
    }
    printf("\n");
    printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    printf("\n");
#endif

    return PacketSize(pTransData->u32BufSize & Clear_Mask);
}

static bool Hidg_Dev_Init(char *key_path, char *mouse_path)
{ 
    key_fd = open(key_path, O_RDWR, 0666);
    if(key_fd < 0)
    {
        return false;
    }

    mouse_fd = open(mouse_path, O_RDWR, 0666);
    if(mouse_fd < 0)
    {
        return false;
    }

    return true;
}

static void Hidg_Dev_DeInit(void)
{ 
    close(key_fd);
    close(mouse_fd);
}

static bool Hidg_Do_Work(ST_INTELL_TransferInfo_t *pstTransInfo)
{
    if(pstTransInfo->u32BufSize & Key_Mask)
    {
        pstTransInfo->u32BufSize &= Clear_Mask;
        for(int i=0; i<pstTransInfo->u32BufSize/Key_DataSize; i++)
        {
            char report[8];
            memcpy(report, pstTransInfo->pVirAddr + i*Key_DataSize, Key_DataSize);

            if(write(key_fd, report, Key_DataSize) != Key_DataSize) 
            {
                return false;
            }
        }
    }

    if(pstTransInfo->u32BufSize & Mouse_Mask)
    {
        pstTransInfo->u32BufSize &= Clear_Mask;
        for(int i=0; i<pstTransInfo->u32BufSize/Mouse_DataSize; i++)
        {
            char report[8];
            memcpy(report, pstTransInfo->pVirAddr + i*Mouse_DataSize, Mouse_DataSize);

            if(write(mouse_fd, report, Mouse_DataSize) != Mouse_DataSize) 
            {
                return false;
            }
        }
    } 

    return true;
}

static void signalHandler(int signum)
{
    char report[8] = {0};
    write(key_fd, report, Key_DataSize);
    write(mouse_fd, report, Mouse_DataSize);
   
    if(stData.pVirAddr)
        free(stData.pVirAddr);

    HidgDevSockDeinit();
    Hidg_Dev_DeInit();

    exit(0);
}

void *Dev_Do_Work(void* data)
{
    memset(&stData, 0, sizeof(ST_INTELL_TransferInfo_t));
    stData.pVirAddr = (char *)malloc(1024);

    while(1)
    {
        memset(stData.pVirAddr, 0, 1024);
        stData.u32BufSize = 0;
        int retSize = HidgDevSocketWaitData(&stData);
        if(retSize <= 0)
        {
            printf("HidgDevSocketWaitData err\n");
            break;
        }   

        if(!Hidg_Do_Work(&stData))
        {
            printf("Hidg_Do_Work err\n");
            break;
        }
    }

    if(stData.pVirAddr)
    {
        free(stData.pVirAddr);
    }

    HidgDevSockDeinit();
    Hidg_Dev_DeInit();

}

int main(int argc, char **argv)
{
    signal(SIGINT, signalHandler);

    if(!Hidg_Dev_Init(Key_DevName, Mouse_DevName))
    {
        printf("Hidg_Dev_Init err\n");
        return -1;
    }

    if(!HidgDevSockInit())
    {
        printf("HidgDevSockInit err\n");
        Hidg_Dev_DeInit();
        return -1;
    }
    
    pthread_create(&Worktid, NULL, Dev_Do_Work, NULL);

    pthread_join(Worktid, NULL);

    return 0;
}


