#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/input.h>
#include <linux/hidraw.h>

#include "common.h"

static int key_fd = -1;
static int mouse_fd = -1;
static int sock_fd = -1;

static ST_INTELL_TransferInfo_t stKeyData;
static ST_INTELL_TransferInfo_t stMouseData;

static pthread_t Worktid;

#ifdef DEBUG
static int idx = 0;
#endif

static bool HidHostSocketInit(int *fdSocket, char *pAddr)
{
    struct sockaddr_in stAddress;
    int socketFd;
    int len;
    int intResult;

    printf("Server ip:%s\n", pAddr);
    if((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return false;
    }

    int on = 1;
    if(-1 == setsockopt(socketFd , IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on) ))
    {
        perror("setsockopt");
        return false;
    }

    stAddress.sin_family = AF_INET;
    stAddress.sin_port = htons(9000);
    if(inet_pton(AF_INET, pAddr, &stAddress.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", pAddr);
        return false;
    }
    len = sizeof(stAddress);

    intResult = connect(socketFd, (struct sockaddr *)&stAddress, len);
    if(intResult == -1)
    {
        printf("ensure the server is up\n");
        perror("connect");
        return false;
    }

    *fdSocket = socketFd;

    return true;
}

static void HidHostSocketDeinit()
{
    close(sock_fd);
}

static int HidHostSocketTransfer(int fdSocket, ST_INTELL_TransferInfo_t *pTransData, unsigned int u32DataSize)
{
    int retSize = 0;

#ifdef DEBUG
    struct timeval tv1;
    struct timezone tz1;
    struct timeval tv2;
    struct timezone tz2;
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
    printf("HidHostSocketTransfer:%d\n", idx++);
    if(gettimeofday(&tv1, &tz1)){
        perror("gettimeofday1 failed\n");
        return -1;
    }
    printf("before send:tv_sec=%d \t tv_usec=%d \n", tv1.tv_sec, tv1.tv_usec);
#endif

    retSize = send(fdSocket, &pTransData->u32BufSize, sizeof(int), 0);
    if(retSize != sizeof(int))
    {
        perror("send");
        return -1;
    }   

    retSize = send(fdSocket, pTransData->pVirAddr, (pTransData->u32BufSize & Clear_Mask), 0);
    if(retSize != (pTransData->u32BufSize & Clear_Mask))
    {
        perror("send");
        return -1;
    }   

#ifdef DEBUG
    if(gettimeofday(&tv2, &tz2)){
        perror("gettimeofday1 failed\n");
        return -1;
    }
    printf("after send:tv_sec=%d \t tv_usec=%d \n", tv2.tv_sec, tv2.tv_usec);
    printf("u32DataSize = %d\tretSize = %d\n", u32DataSize, retSize);
    for(int i=0; i<(pTransData->u32BufSize & Clear_Mask); i++)
    {
        printf("[%d]:%d\t", i, pTransData->pVirAddr[i]);
    }
    printf("\n");
    printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    printf("\n");
#endif

    return u32DataSize;
}

static bool Hid_Dev_Probe(void)
{
    DIR *dir;
    struct dirent *ptr;
    dir = opendir("/dev/");
    chdir("/dev/");
    while(ptr = readdir(dir))
    {
        if(strstr(ptr->d_name, "hidraw"))
        { 
            int fd = open(ptr->d_name, O_RDWR|O_NONBLOCK);
            if(fd < 0)
            {
                printf("can not open %s\n", ptr->d_name);
                return false;
            }

            int ret;
            int len;
            struct hidraw_report_descriptor desc;

            ret = ioctl(fd, HIDIOCGRDESCSIZE, &len);
            if(ret < 0)
            {
                printf("HIDIOCGRDESCSIZE err!\n");
                return false;
            }

            desc.size = len;
            ret = ioctl(fd, HIDIOCGRDESC, &desc);
            if(ret < 0)
            {
                printf("HIDIOCGRDESC err!\n");
                return false;
            }

            switch(desc.value[3])
            {
                case Key_Type:
                    key_fd = fd;
#ifdef DEBUG
                    printf("%s is keyboard\n", ptr->d_name);
#endif
                    break;
                case Mouse_Type:
                    mouse_fd = fd;
#ifdef DEBUG
                    printf("%s is mouse\n", ptr->d_name);
#endif
                    break;
                default:
#ifdef DEBUG
                    printf("%s is not support\n", ptr->d_name);
#endif
                    close(fd);
                    break;
            }
        }                
    }    
    closedir(dir);

    return true;
}

static bool Hid_Dev_Init(char *DevName)
{ 
    return true;
}

static void Hid_Dev_DeInit(void)
{
    if(key_fd > 0)
    {
        close(key_fd);
        key_fd = -1;
    }
    if(mouse_fd > 0)
    {
        close(mouse_fd);
        mouse_fd = -1;
    }
}

static bool HidHostGetBuf(ST_INTELL_TransferInfo_t *pKeyBufInfo, ST_INTELL_TransferInfo_t *pMouseBufInfo)
{
    int retval;
    struct timeval tv1;
    struct timezone tz1;
    struct timeval tv2;
    struct timezone tz2;

    fd_set readfds;
    FD_ZERO(&readfds);
    if(key_fd > 0)
        FD_SET(key_fd, &readfds);
    if(mouse_fd > 0)
        FD_SET(mouse_fd, &readfds);

    int max = key_fd > mouse_fd ? key_fd : mouse_fd;
    retval = select(max + 1, &readfds, NULL, NULL, NULL);
    if(retval <= 0)
    {
        printf("select err!\n");
        return false;
    }

    if(gettimeofday(&tv1, &tz1)){
        perror("gettimeofday1 err\n");
    }

    while(1)
    {
        if(key_fd > 0)
        {
            retval = read(key_fd, pKeyBufInfo->pVirAddr + pKeyBufInfo->u32BufSize, Key_DataSize);
            if(retval == Key_DataSize)
            {
                pKeyBufInfo->u32BufSize += Key_DataSize;
            }   
        }

        if(mouse_fd > 0)
        {
            retval = read(mouse_fd, pMouseBufInfo->pVirAddr + pMouseBufInfo->u32BufSize, Mouse_DataSize);
            if(retval == Mouse_DataSize)
            {
                pMouseBufInfo->u32BufSize += Mouse_DataSize;
            }
        }

        if(gettimeofday(&tv2, &tz2)){
            perror("gettimeofday2 err\n");
        }

        if((tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec) > 10*1000)
        {   
            return true;
        }
    }   
}

static void signalHandler(int signum)
{
    if(stKeyData.pVirAddr)
    {
        free(stKeyData.pVirAddr);
    }

    if(stMouseData.pVirAddr)
    {
        free(stMouseData.pVirAddr);
    }

    Hid_Dev_DeInit();
    HidHostSocketDeinit();

    exit(0);
}

void *Host_Do_Work(void* data)
{  
    while(1)
    {
        memset(stKeyData.pVirAddr, 0, 1024);
        stKeyData.u32BufSize = 0;

        memset(stMouseData.pVirAddr, 0, 1024);
        stMouseData.u32BufSize = 0;

        if(key_fd>0 || mouse_fd>0)
        {
            if(!HidHostGetBuf(&stKeyData, &stMouseData))
            {       
                printf("HidHostGetBuf err\n");
                break;
            }
        }
        
        if(key_fd > 0)
        {
            if(stKeyData.u32BufSize > 0)
            {
#ifdef DEBUG
                printf("Key:");
                for(int i=0; i<stKeyData.u32BufSize; i++)
                {
                    printf("[%d]:%d\t", i, stKeyData.pVirAddr[i]);
                }
                printf("\n");
#endif

                stKeyData.u32BufSize |= Key_Mask;
                if(HidHostSocketTransfer(sock_fd, &stKeyData, PacketSize(stKeyData.u32BufSize & Clear_Mask)) != PacketSize(stKeyData.u32BufSize & Clear_Mask))
                {
                    printf("HidHostSocketTransfer err\n");
                    break;
                }
            }
        }

        if(mouse_fd > 0)
        {
            if(stMouseData.u32BufSize > 0)
            {
#ifdef DEBUG
                printf("Mouse:");
                for(int i=0; i<stMouseData.u32BufSize; i++)
                {
                    printf("[%d]:%d\t", i, stMouseData.pVirAddr[i]);
                }
                printf("\n"); 
#endif

                stMouseData.u32BufSize |= Mouse_Mask;
                if(HidHostSocketTransfer(sock_fd, &stMouseData, PacketSize(stMouseData.u32BufSize & Clear_Mask)) != PacketSize(stMouseData.u32BufSize & Clear_Mask))
                {
                    printf("HidHostSocketTransfer err\n");
                    break;
                }
            }
        }
    }   

    if(stKeyData.pVirAddr)
    {
        free(stKeyData.pVirAddr);
    }

    if(stMouseData.pVirAddr)
    {
        free(stMouseData.pVirAddr);
    }

    Hid_Dev_DeInit();
    HidHostSocketDeinit();

}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("./host IPaddr\n");
        return -1; 
    }    

    signal(SIGINT, signalHandler);

    if(!HidHostSocketInit(&sock_fd, argv[1]))
    {
        printf("HidHostSocketInit err\n");
        Hid_Dev_DeInit();
        return -1;
    }

    memset(&stKeyData, 0, sizeof(ST_INTELL_TransferInfo_t));
    stKeyData.pVirAddr = (char *)malloc(1024);

    memset(&stMouseData, 0, sizeof(ST_INTELL_TransferInfo_t));
    stMouseData.pVirAddr = (char *)malloc(1024);

    pthread_create(&Worktid, NULL, Host_Do_Work, NULL);


#if 1
    if(!Hid_Dev_Probe())
        {
            printf("Hid_Dev_Probe err\n");
            return -1;
        }

    if(key_fd<0 && mouse_fd<0)
        printf("Device not found\n");

    struct sockaddr_nl nl_client;
    int nl_fd, ret, size = 1024;
    
    nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    
    memset(&nl_client, 0, sizeof(nl_client));
    nl_client.nl_family = AF_NETLINK;
    nl_client.nl_pid = getpid();
    nl_client.nl_groups = 1;

    setsockopt(nl_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

    bind(nl_fd, (struct sockaddr*)&nl_client, sizeof(nl_client));
    
    fd_set fds;
    while(1)
    {
        char buf[1024] = { 0 };

        FD_ZERO(&fds);
        FD_SET(nl_fd, &fds);
        ret = select(nl_fd + 1, &fds, NULL, NULL, NULL);
        if(ret <= 0)
        {
            printf("select err!\n");
            return -1;
        }

        /* receive data */
        ret = recv(nl_fd, &buf, sizeof(buf), 0);
        if(ret > 0)
        {
            if(strstr(buf, "hid"))
            {
#ifdef DEBUG
                printf("%s\n", buf);
#endif
                Hid_Dev_DeInit();
                pthread_cancel(Worktid);
                sleep(1);
                if(!Hid_Dev_Probe())
                {
                    printf("Hid_Dev_Probe err\n");
                    return -1;
                }
                if(key_fd<0 && mouse_fd<0)
                {
                    printf("Device not found\n");
                    continue;
                }
                pthread_create(&Worktid, NULL, Host_Do_Work, NULL);
            }
        }
    }

    close(nl_fd);
#endif

    pthread_join(Worktid, NULL);

    return 0;
}

