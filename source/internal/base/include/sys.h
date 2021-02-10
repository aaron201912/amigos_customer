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

#ifndef __SYS__H__
#define __SYS__H__
#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <pthread.h>

#include "mi_sys_datatype.h"

#include "tem.h"

#include "iniparser.h"


#define MAX_INPUT_CNT    64
#define MAX_OUTPUT_CNT   64

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)
#define ALIGN16_DOWN(x) (x&0xFFF0)

#define AMIGOS_INFO(fmt, args...) printf("\033[1;32m[AMI_INFO][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);printf("\033[0m");
#define AMIGOS_ERR(fmt, args...) printf("\033[1;31m[AMI_ERR][%s][%d]", __FUNCTION__, __LINE__);printf(fmt, ##args);printf("\033[0m")

#ifndef ExecFunc
#define ExecFunc(_func_, _ret_) \
    do{ \
        MI_S32 s32Ret = MI_SUCCESS; \
        s32Ret = _func_; \
        if (s32Ret != _ret_) \
        { \
            AMIGOS_ERR("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret; \
        } \
        else \
        { \
            AMIGOS_INFO("[%s %d]exec function pass\n", __func__, __LINE__); \
        } \
    } while(0)
#endif

#ifndef STCHECKRESULT
#define STCHECKRESULT(_func_)\
    do{ \
        MI_S32 s32Ret = MI_SUCCESS; \
        s32Ret = _func_; \
        if (s32Ret != MI_SUCCESS)\
        { \
            AMIGOS_ERR("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret; \
        } \
        else \
        { \
            AMIGOS_INFO("(%s %d)exec function pass\n", __FUNCTION__,__LINE__); \
        } \
    } while(0)
#endif


typedef enum
{
    E_SENDER_STATE_START,
    E_SENDER_STATE_STOP,
    E_SENDER_STATE_MAX
}E_SENDER_STATE;

typedef enum
{
    E_STREAM_YUV422 = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV,
#ifndef SSTAR_CHIP_I2
    E_STREAM_YUV422_UYVY = E_MI_SYS_PIXEL_FRAME_YUV422_UYVY,
    E_STREAM_YUV422_YVYU = E_MI_SYS_PIXEL_FRAME_YUV422_YVYU,
    E_STREAM_YUV422_VYUY = E_MI_SYS_PIXEL_FRAME_YUV422_VYUY,
    E_STREAM_ARGB8888 = E_MI_SYS_PIXEL_FRAME_ARGB8888,
    E_STREAM_ABGR8888 = E_MI_SYS_PIXEL_FRAME_ABGR8888,
    E_STREAM_BGRA8888 = E_MI_SYS_PIXEL_FRAME_BGRA8888,
#endif
    E_STREAM_YUV420 = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420,
#ifndef SSTAR_CHIP_I2
    E_STREAM_RGB_BAYER_BASE = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_BASE,
    E_STREAM_RGB_BAYER_MAX = E_MI_SYS_PIXEL_FRAME_RGB_BAYER_NUM,
#endif
    E_STREAM_MAX = E_MI_SYS_PIXEL_FRAME_FORMAT_MAX,
}E_VIDEO_RAW_FORMAT;

typedef enum
{
    E_STREAM_H264 = 12,
    E_STREAM_H265 = 13,
    E_STREAM_JPEG = 14,
}E_VIDEO_CODEC_FORMAT;

typedef enum
{
    E_STREAM_PCM,
    E_STREAM_AAC
}E_AUDIO_CODEC_FORMAT;

typedef enum
{
    E_STREAM_VIDEO_RAW_DATA,
    E_STREAM_VIDEO_CODEC_DATA,
    E_STREAM_AUDIO_CODEC_DATA
}E_STREAM_TYPE;

typedef enum
{
    E_STREAM_IN_DATA_IN_KERNEL_MODULE = 0x1,
    E_STREAM_IN_DATA_IN_USER_MODULE = 0x2,
    E_STREAM_OUT_DATA_IN_KERNEL_MODULE = 0x4,
    E_STREAM_OUT_DATA_IN_USER_MODULE = 0x8,
}E_SYS_MODULE_TYPE;

typedef struct stSenderState_s
{
    E_SENDER_STATE eState;
    void *pData;
}stSenderState_t;

typedef struct stFaceInfo_s
{
    char faceName[64];
    unsigned short xPos;
    unsigned short yPos;
    unsigned short faceW;
    unsigned short faceH;
    unsigned short winWid;
    unsigned short winHei;
}stFaceInfo_t;

typedef struct stEsPackage_s
{
    unsigned int uintDataSize;
    unsigned char bSliceEnd;
    char *pData;
}stEsPackage_t;

typedef struct stEsData_s
{
    unsigned int uintPackCnt;
    stEsPackage_t *pDataAddr;
}stEsData_t;

typedef struct stPcmData_s
{
    unsigned int uintSize;
    char *pData;
}stPcmData_t;

typedef struct stYuvSpData_s
{
    char *pYdataAddr;
    char *pUvDataAddr;
}stYuvSpData_t;

typedef struct stVideoEsInfo_s
{
    E_VIDEO_CODEC_FORMAT enVideoCodecFmt;
    unsigned int streamWidth;
    unsigned int streamHeight;
}stVideoEsInfo_t;

typedef struct stVideoFrameInfo_s
{
    E_VIDEO_RAW_FORMAT enVideoRawFmt;
    unsigned int streamWidth;
    unsigned int streamHeight;
}stVideoFrameInfo_t;

typedef struct stPcmInfo_s
{
    E_AUDIO_CODEC_FORMAT enAudioCodecFmt;
    unsigned int uintBitRate;
    unsigned int uintBitLength;
    unsigned int uintChannelCnt;
}stPcmInfo_t;

typedef struct stStreamInfo_s
{
    E_STREAM_TYPE eStreamType;    
    union
    {
        stVideoFrameInfo_t stFrameInfo;
        stVideoEsInfo_t stEsInfo;
        stPcmInfo_t stPcmInfo;
    };
}stStreamInfo_t;
typedef struct stStreamData_s
{
    stStreamInfo_t stInfo;
    union
    {
        char *pYuvData;
        stPcmData_t stPcmData;
        stYuvSpData_t stYuvSpData;
        stEsData_t stEsData;
    };
}stStreamData_t;

typedef struct stModDesc_s
{
    std::string modKeyString;
    unsigned int modId;
    unsigned int chnId;
    unsigned int devId;
}stModDesc_t;
typedef struct stModIoInfo_s
{
    std::string modKeyString;
    unsigned int portId;
    unsigned int frmRate;
}stModIoInfo_t;
typedef struct stModInputInfo_s
{
    std::string curIoKeyString;
    unsigned int curPortId;
    unsigned int curFrmRate;
#ifndef SSTAR_CHIP_I2
    unsigned int bindType;
    unsigned int bindPara;
#endif
    stModIoInfo_t stPrev;
}stModInputInfo_t;
typedef struct stModOutputInfo_s
{
    std::string curIoKeyString;
    unsigned int curPortId;
    unsigned int curFrmRate;
    unsigned int bSenderConnect;
    stStreamInfo_t stStreanInfo;
    std::vector<stModIoInfo_t> vectNext;
}stModOutputInfo_t;
typedef void (*DeliveryRecFp)(void *, unsigned int, void *, unsigned char);

typedef enum
{
    E_SYS_MOD_DISP = E_MI_MODULE_ID_DISP,
    E_SYS_MOD_VENC = E_MI_MODULE_ID_VENC,
    E_SYS_MOD_VDEC = E_MI_MODULE_ID_VDEC,
    E_SYS_MOD_VPE = E_MI_MODULE_ID_VPE,
    E_SYS_MOD_VIF = E_MI_MODULE_ID_VIF,
    E_SYS_MOD_DIVP = E_MI_MODULE_ID_DIVP,
    E_SYS_MOD_VDISP = E_MI_MODULE_ID_VDISP,
    E_SYS_MOD_LDC = E_MI_MODULE_ID_LDC,
    E_SYS_MOD_AI = E_MI_MODULE_ID_AI,
    E_SYS_MOD_AO = E_MI_MODULE_ID_AO,
#ifndef SSTAR_CHIP_I2
    E_SYS_MOD_SNR = E_MI_MODULE_ID_SNR,
#endif
    E_SYS_MOD_INT_MAX = E_MI_MODULE_ID_MAX,
    E_SYS_MOD_SIGNAL_MONITOR,
    E_SYS_MOD_IQ,
    E_SYS_MOD_SLOT,
    E_SYS_MOD_EXT,
    E_SYS_MOD_RTSP,
    E_SYS_MOD_DLA,
    E_SYS_MOD_FDFR,
    E_SYS_MOD_UI,
    E_SYS_MOD_FILE,
    E_SYS_MOD_UAC,
    E_SYS_MOD_UVC,
    E_SYS_MOD_EMPTY,
    E_SYS_MOD_MAX
}E_SYS_MOD;
typedef struct stModuleAttr_s
{
    E_SYS_MOD enMod;
    unsigned char bMaskMod;
}stModuleAttr_t;
class Sys
{
    public:
        typedef struct stReceiverPortDesc_s
        {
            unsigned char portId; //current in port
            unsigned char bStart;
            DeliveryRecFp fpRec;
            Sys * pSysClass; //current class
            void *pUsrData;
        }stReceiverPortDesc_t;
        typedef struct stReceiverDesc_s
        {
            Sys * pSysClass; //prev class
            unsigned int uintPort; //prev out port
            pthread_mutex_t stDeliveryMutex;
            unsigned int uintRefsCnt;
            std::map<std::string, stReceiverPortDesc_t> mapPortDesc;
        }stReceiverDesc_t;
        class SysAutoLock
        {
            public:
                explicit SysAutoLock(pthread_mutex_t &mutex) : mAutoLock(mutex)
                {
                    pthread_mutex_lock(&mutex);
                }
                ~SysAutoLock()
                {
                    pthread_mutex_unlock(&mAutoLock);
                }
                pthread_mutex_t &mAutoLock;
        };
        Sys()
        {
            gstSwitchSrcMutex = PTHREAD_MUTEX_INITIALIZER;
            pRecvEsBuffer = NULL;
            uintRecvEsBufferSize = 0;
        }
        virtual ~Sys()
        {
            if (pRecvEsBuffer)
                free(pRecvEsBuffer);
        }
        static void InitSys(std::string strIniPath, std::map<std::string, unsigned int> &mapModId);
        static void DeinitSys();
        static void CreateObj(std::string strIniPath, std::map<std::string, unsigned int> &mapModId);
        static void DestroyObj();
        static void Begin(std::map<std::string, Sys *> &maskMap);
        static void End(std::map<std::string, Sys *> &maskMap);
        static void Extract(std::vector<Sys *> &objVect);
        static void Insert(std::vector<Sys *> &objVect);
        static void SwtichSrc(Sys *srcObj, unsigned int srcOutPort, Sys *srcObjNew, unsigned int srcOutPortNew, Sys *dstObj, unsigned int dstInPort);
        static Sys* GetInstance(std::string &strKey){return (connectMap.find(strKey) != connectMap.end()) ? connectMap[strKey] : NULL;}
        void GetModDesc(stModDesc_t &stDesc){stDesc = stModDesc;}
        void GetInputPortInfo(unsigned int inPortId, stModInputInfo_t & stIn);
        void UpdateInputPortInfo(unsigned int inPortId, stModInputInfo_t & stIn);
        void GetOutputPortInfo(unsigned int outPortId, stModOutputInfo_t & stOut);
        void UpdateOutputPortInfo(unsigned int outPortId, stModOutputInfo_t & stOut);

        //Reset stream api
        int GetInputStreamInfo(unsigned int inPortId, stStreamInfo_t *pInfo);
        int UpdateInputStreamInfo(unsigned int inPortId, stStreamInfo_t *pInfo);

    protected:
        //Modules flow: Init->BindBlock->Start;Stop->UnBindBlock->Deinit
        virtual void BindBlock(stModInputInfo_t & stIn);
        virtual void UnBindBlock(stModInputInfo_t & stIn);
        virtual void PrevExtBind(stModInputInfo_t & stIn);
        virtual void PrevIntBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc);
        virtual void PrevExtUnBind(stModInputInfo_t & stIn);
        virtual void PrevIntUnBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc);

        virtual void LoadDb(){};
        virtual void Init() = 0;
        virtual void Deinit() = 0;
        virtual void Start();
        virtual void Stop();
        virtual void Incoming(stStreamInfo_t *pInfo){};
        virtual void Outcoming(){};
        virtual void ResetOut(unsigned int outPortId, stStreamInfo_t *pInfo){};

        //Delivery api
        virtual int CreateSender(unsigned int outPortId);
        virtual int DestroySender(unsigned int outPortId);
        virtual int StartSender(unsigned int outPortId);
        virtual int StopSender(unsigned int outPortId);

         //Delivery internal api
        int Send(unsigned int outPortId, void *pData, unsigned int intDataSize);
        int Connect(unsigned int outPortId, stStreamInfo_t *pInfo);
        int Disconnect(unsigned int outPortId);
        int CreateReceiver(unsigned int inPortId, DeliveryRecFp funcRecFp, void *pUsrData);
        int DestroyReceiver(unsigned int inPortId);
        int StartReceiver(unsigned int inPortId);
        int StopReceiver(unsigned int inPortId);

        //INI operation
        static int GetIniInt(std::string section, std::string key, int intDefault = -1);
        static unsigned int GetIniUnsignedInt(std::string section, std::string key, unsigned int uintDefault = -1);
        static char *GetIniString(std::string section, std::string key, char *pDefaultStr = NULL);
        static std::map<unsigned int, unsigned int> mapSysModuleType;

        std::map<unsigned int, stModInputInfo_t> mapModInputInfo;
        std::map<unsigned int, stModOutputInfo_t> mapModOutputInfo;
        stModDesc_t stModDesc;
        //Delivery data
        std::map<unsigned int, stReceiverDesc_t> mapRecevier;
        pthread_mutex_t gstSwitchSrcMutex;

    private:

        //Module's tree implement
        template<class SYSCHILD>
        class SysChild
        {
            public:
                explicit SysChild(std::string &strKey)
                {
                    Sys *pClass;

                    pClass = new (std::nothrow) SYSCHILD;
                    assert(pClass);
                    pClass->SetCurInfo(strKey);
                }
                virtual ~SysChild(){};
        };
        void SetCurInfo(std::string &strKey);
        void BuildModTree();
        static void SetupModuleType();
        static void CreateConnection();
        static void DestroyConnection();
        static void Implement(std::string &strKey);
        static bool FindBlock(std::string &strKey){return connectMap.find(strKey) != connectMap.end();}
        static unsigned int FindBlockId(std::string &strKey)
        {
            std::string strModName;

            strModName = GetIniString(strKey, "MOD");

            return (connectIdMap.find(strModName) != connectIdMap.end()) ? connectIdMap[strModName] : -1;
        }
        static void *SenderMonitor(ST_TEM_BUFFER stBuf);
        static void DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId);

        static std::map<std::string, Sys *> connectMap;
        static std::vector<Sys *> connectOrder;
        static std::map<std::string, unsigned int> connectIdMap;
        static dictionary *m_pstDict;
        static pthread_mutex_t gstUsrMutex;
        char *pRecvEsBuffer;
        unsigned int uintRecvEsBufferSize;
};

#endif
