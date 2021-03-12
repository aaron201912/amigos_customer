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
#include "snr.h"
#include "vif.h"
#include "vpe.h"
#include "ai.h"
#include "sys.h"
#include "divp.h"
#include "rtsp.h"
#include "venc.h"
#include "file.h"
#include "inject.h"
#include "empty.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

//#include "es8156.h"
#include "mi_sensor.h"
#include "tem.h"


#define I2C_ADAPTER_STR     ("/dev/i2c-1")
#define ES8156_CHIP_ADDR   (0x08)

typedef enum
{
    EN_CUST_CMD_GET_STATE,
    EN_CUST_CMD_GET_AUDIO_INFO,
    EN_CUST_CMD_CONFIG_I2S,
    EN_CUST_CMD_MAX
}SS_HdmiConv_UsrCmd_e;

typedef enum
{
    EN_I2S_MODE_NORMAL,
    EN_I2S_MODE_LEFT_JUSTIFIED,
    EN_I2S_MODE_RIGHT_JUSTIFIED
}SS_HdmiConv_I2SMode_e;

typedef enum
{
    EN_SIGNAL_NO_CONNECTION,
    EN_SIGNAL_CONNECTED,
    EN_SIGNAL_LOCK
}SS_HdmiConv_SignalInfo_e;
typedef enum
{
    EN_AUDIO_TYPE_PCM,
    EN_AUDIO_TYPE_AC3,
    EN_AUDIO_TYPE_AAC,
    EN_AUDIO_TYPE_DTS
}SS_HdmiConv_AudFormat_e;
typedef struct SS_HdmiConv_AudInfo_s
{
    SS_HdmiConv_AudFormat_e enAudioFormat;
    MI_U8 u8ChannelCount;
    MI_U8 u8BitWidth;     /*16bit, 24bit, 32bit*/
    MI_U32 u32SampleRate; /*48000, 44100, 32000, 16000, 8000*/
}SS_HdmiConv_AudInfo_t;

typedef struct
{
    std::vector<Sys *> * pVectVideoPipeLine;
    std::vector<Sys *> * pVectNoSignalVideoPipeLine;
    Sys * pDstObject;
    SS_HdmiConv_SignalInfo_e enCurState;
    MI_U16 u16SnrWidth;
    MI_U16 u16SnrHeight;
}stMonitorDataPackage_t;

typedef enum
{
    EN_HDMICONV_START_MONITOR,
    EN_HDMICONV_EXIT_MONITOR
}SS_HdmiConv_MonitorCmd_e;

static const unsigned char u8Es8156InitSetting[][2] = {
    {0x02, 0x84},
    {0x00, 0x3c},
    {0x00, 0x1c},
    {0x02, 0x84},
    {0x0A, 0x01},
    {0x0B, 0x01},
    {0x0D ,0x14},
    {0x01, 0xC1},
    {0x18, 0x00},
    {0x08, 0x3F},
    {0x09, 0x02},
    {0x00, 0x01},
    {0x22, 0x00},
    {0x23, 0xCA},
    {0x24, 0x00},
    {0x25, 0x20},
    {0x14, 0xbf},
    {0x11, 0x31},
};

static const unsigned char u8Es8156DeinitSetting[][2] = {
    {0x14, 0x00},
    {0x19, 0x02},
    {0x21, 0x1F},
    {0x22, 0x02},
    {0x25, 0x21},
    {0x25, 0x01},
    {0x25, 0x87},
    {0x18, 0x01},
    {0x09, 0x02},
    {0x09, 0x01},
    {0x08, 0x00},
};
static int s8I2cAdapterFd = -1;
static int bI2cInit = 0;

int I2C_Init(unsigned int timeout, unsigned int retryTime)
{
    int s32Ret;
    if (0 == bI2cInit)
    {
        s8I2cAdapterFd = open((const char *)I2C_ADAPTER_STR, O_RDWR);
        if (-1 == s8I2cAdapterFd)
        {
            printf("Error: %s\n", strerror(errno));
            return -1;
        }

        s32Ret = ioctl(s8I2cAdapterFd, I2C_TIMEOUT, timeout);
        if (s32Ret < 0)
        {
            printf("Failed to set i2c time out param.\n");
            close(s8I2cAdapterFd);
            return -1;
        }

        s32Ret = ioctl(s8I2cAdapterFd, I2C_RETRIES, retryTime);
        if (s32Ret < 0)
        {
            printf("Failed to set i2c retry time param.\n");
            close(s8I2cAdapterFd);
            return -1;
        }

        bI2cInit = 1;
    }
    return 0;
}

int I2C_Deinit(void)
{
    if (1 == bI2cInit)
    {
        close(s8I2cAdapterFd);
        bI2cInit = 0;
    }
    return 0;
}

int I2C_GetFd(void)
{
    return s8I2cAdapterFd;
}

int I2C_WriteByte(unsigned char reg, unsigned char val, unsigned char i2cAddr)
{
    int s32Ret;
    unsigned char data[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages;

    if (-1 == I2C_GetFd())
    {
        printf("ES7210 hasn't been call I2C_Init.\n");
        return -1;
    }

    memset((&packets), 0x0, sizeof(packets));
    memset((&messages), 0x0, sizeof(messages));

    // send one message
    packets.nmsgs = 1;
    packets.msgs = &messages;

    // fill message
    messages.addr = i2cAddr;   // codec reg addr
    messages.flags = 0;                 // read/write flag, 0--write, 1--read
    messages.len = 2;                   // size
    messages.buf = data;                // data addr

    // fill data
    data[0] = reg;
    data[1] = val;

    s32Ret = ioctl(I2C_GetFd(), I2C_RDWR, (unsigned long)&packets);
    if (s32Ret < 0)
    {
        printf("Failed to write byte to ES7210: %s.\n", strerror(errno));
        return -1;
    }

    return 0;
}

int I2C_ReadByte(unsigned char reg, unsigned char *val, unsigned char i2cAddr)
{
    int s32Ret;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];
    unsigned char tmpReg, tmpVal;

    if (-1 == I2C_GetFd())
    {
        printf("ES7210 hasn't been call I2C_Init.\n");
        return -1;
    }

    if (NULL == val)
    {
        printf("val param is NULL.\n");
        return -1;
    }

    tmpReg = reg;
    memset((&packets), 0x0, sizeof(packets));
    memset((&messages), 0x0, sizeof(messages));

    packets.nmsgs = 2;
    packets.msgs = messages;

    messages[0].addr = i2cAddr;
    messages[0].flags = 0;
    messages[0].len = 1;
    messages[0].buf = &tmpReg;

    tmpVal = 0;
    messages[1].addr = i2cAddr;
    messages[1].flags = 1;
    messages[1].len = 1;
    messages[1].buf = &tmpVal;

    s32Ret = ioctl(I2C_GetFd(), I2C_RDWR, (unsigned long)&packets);
    if (s32Ret < 0)
    {
        printf("Failed to read byte from ES7210: %s.\n", strerror(errno));
        return -1;
    }

    *val = tmpVal;
    return 0;
}

static int ES8156_WriteByte(unsigned char reg, unsigned char val)
{
    return I2C_WriteByte(reg, val, ES8156_CHIP_ADDR);
}

static int ES8156_ReadByte(unsigned char reg, unsigned char *val)
{
    return I2C_ReadByte(reg, val, ES8156_CHIP_ADDR);
}

static int ES8156_SelectWordLength(unsigned char ucharWordLength)
{
    int s32Ret;
    unsigned char val;

    s32Ret = ES8156_ReadByte(0x11, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 reg 0x11 :%x.\n", val);
    }
    else
    {
        return s32Ret;
    }
    switch (ucharWordLength)
    {
        case 16:
        {
            val = (0x3 << 4) | (val & 0xF);
        }
        break;
        case 18:
        {
            val = (0x2 << 4) | (val & 0xF);
        }
        break;
        case 20:
        {
            val = (0x1 << 4) | (val & 0xF);
        }
        break;
        case 24:
        {
            val = val & 0xF;
        }
        break;
        case 32:
        {
            val = (0x4 << 4) | (val & 0xF);
        }
        break;
        default:
            val = (0x3 << 4) | (val & 0xF);
            break;
    }
    s32Ret = ES8156_WriteByte(0x11, val);
    if (0 != s32Ret)
    {
        return s32Ret;
    }
    printf("Write val 0x%x\n", val);

    return 0;

}
static int ES8156_ConfigI2s(SS_HdmiConv_I2SMode_e enI2sMode)
{
    unsigned char u8ReadVal = 0;

    switch (enI2sMode)
    {
        case EN_I2S_MODE_NORMAL:
        {
            ES8156_ReadByte(0x11, &u8ReadVal);
            u8ReadVal &= 0xFE; //bit 0 = 0
            ES8156_WriteByte(0x11, u8ReadVal);
        }
        break;
        case EN_I2S_MODE_LEFT_JUSTIFIED:
        {
            ES8156_ReadByte(0x11, &u8ReadVal);
            u8ReadVal = (u8ReadVal & 0xFE) | 0x1; //bit 0 = 1
            ES8156_WriteByte(0x11, u8ReadVal);
        }
        break;
        case EN_I2S_MODE_RIGHT_JUSTIFIED:
        {
            // I don't know.
        }
        break;
        default:
            break;
    }

    return 0;
}

static int ES8156_Init(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    I2C_Init(10, 5);
    s32Ret = ES8156_ReadByte(0xfe, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 ID0:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    s32Ret = ES8156_ReadByte(0xfd, &val);
    if (0 == s32Ret)
    {
        printf("ES8156 ID1:%x.\n", val);
    }
    else
    {
        return s32Ret;
    }

    for (u32Index = 0; u32Index < ((sizeof(u8Es8156InitSetting)) / (sizeof(u8Es8156InitSetting[0]))); u32Index++)
    {
        reg = u8Es8156InitSetting[u32Index][0];
        val = u8Es8156InitSetting[u32Index][1];

        s32Ret = ES8156_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }
    printf("======================Init ES8156 success.========================\n");

    return 0;
}

static int ES8156_Deinit(void)
{
    int s32Ret;
    unsigned char val = 0;
    unsigned char reg = 0;
    unsigned int u32Index = 0;

    for (u32Index = 0; u32Index < ((sizeof(u8Es8156DeinitSetting)) / (sizeof(u8Es8156DeinitSetting[0]))); u32Index++)
    {
        reg = u8Es8156DeinitSetting[u32Index][0];
        val = u8Es8156DeinitSetting[u32Index][1];

        s32Ret = ES8156_WriteByte(reg, val);
        if (0 != s32Ret)
        {
            return s32Ret;
        }
    }
    I2C_Deinit();

	printf("Deinit ES8156 success.\n");
    return 0;
}

void Sys::Implement(std::string &strKey)
{
    unsigned int intId = 0;

    //printf("Connect key str %s\n", strKey.c_str());
    intId = Sys::FindBlockId(strKey);
    if (intId == (unsigned int)-1)
    {
        printf("Can't find key str %s\n", strKey.c_str());
        return;
    }
    if (!Sys::FindBlock(strKey))
    {
        switch (intId)
        {
            case E_SYS_MOD_RTSP:
            {
                SysChild<Rtsp> Rtsp(strKey);
            }
            break;
#if INTERFACE_VENC
            case E_SYS_MOD_VENC:
            {
                SysChild<Venc> Venc(strKey);
            }
            break;
#endif
#if INTERFACE_VPE
            case E_SYS_MOD_VPE:
            {
                SysChild<Vpe> Vpe(strKey);
            }
            break;
#endif
#if INTERFACE_VIF
            case E_SYS_MOD_VIF:
            {
                SysChild<Vif> Vif(strKey);
            }
            break;
#endif
#if INTERFACE_AI
            case E_SYS_MOD_AI:
            {
                SysChild<Ai> Ai(strKey);
            }
            break;
#endif
#if INTERFACE_DIVP
            case E_SYS_MOD_DIVP:
            {
                SysChild<Divp> Divp(strKey);
            }
            break;
#endif
            case E_SYS_MOD_INJECT:
            {
                SysChild<Inject> Inject(strKey);
            }
            break;
#if INTERFACE_SENSOR
            case E_SYS_MOD_SNR:
            {
                SysChild<Snr> Snr(strKey);
            }
            break;
#endif
            case E_SYS_MOD_FILE:
            {
                SysChild<File> File(strKey);
            }
            break;
            case E_SYS_MOD_EMPTY:
            {
                SysChild<Empty> Inject(strKey);
            }
            break;
            default:
                return;
        }
        GetInstance(strKey)->BuildModTree();
    }

    return;
}
void *HdmiConvDoCmd(ST_TEM_BUFFER stBuf, ST_TEM_USER_DATA stData)
{
    SS_HdmiConv_MonitorCmd_e *penCmd;
    stMonitorDataPackage_t *pstPackage;

    ASSERT(stData.u32UserDataSize == sizeof(SS_HdmiConv_MonitorCmd_e));
    ASSERT(sizeof(stMonitorDataPackage_t) == stBuf.u32TemBufferSize);

    pstPackage = (stMonitorDataPackage_t *)stBuf.pTemBuffer;
    penCmd = (SS_HdmiConv_MonitorCmd_e*)stData.pUserData;
    switch (*penCmd)
    {
        case EN_HDMICONV_START_MONITOR:
        {
            printf("Nothing to do !\n");
        }
        break;
        case EN_HDMICONV_EXIT_MONITOR:
        {
            if (pstPackage->enCurState == EN_SIGNAL_LOCK)
            {
                ES8156_Deinit();
                printf("Signal lock need release!\n");
                Sys::Extract(*pstPackage->pVectVideoPipeLine);
                printf("Release video done !\n");
            }
            else
            {
                Sys::Extract(*pstPackage->pVectNoSignalVideoPipeLine);
                printf("Release File done !\n");
            }
        }
        break;
        default:
            ASSERT(0);
    }

    return NULL;
}
static void SetVpeOut(MI_U16 u16W, MI_U16 u16H)
{
    Vpe *VpeObj = NULL;
    std::string objName;
    stVpeInfo_t stVpeInfo;
    std::vector<stVpeOutInfo_t> vVpeOut;

    objName = "VPE_CH0_DEV0";
    VpeObj = dynamic_cast<Vpe *>(Sys::GetInstance(objName));
    if (!VpeObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        return;
    }
    VpeObj->GetInfo(stVpeInfo, vVpeOut);
    for (unsigned int i = 0; i < vVpeOut.size(); i++)
    {
        if (vVpeOut[i].intPortId == 0)
        {
            vVpeOut[i].intVpeOutWidth = u16W;
            vVpeOut[i].intVpeOutHeight = u16H;
            break;
        }
    }
    VpeObj->UpdateInfo(stVpeInfo, vVpeOut);
}
static void SetVenc(MI_U16 u16W, MI_U16 u16H)
{
    Venc *VencObj = NULL;
    std::string objName;
    stVencInfo_t stVencInfo;
    stModInputInfo_t stInputPortInfo;

    objName = "VENC_CH0_DEV0";
    VencObj = dynamic_cast<Venc *>(Sys::GetInstance(objName));
    if (!VencObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        return;
    }
    VencObj->GetInfo(stVencInfo);
    stVencInfo.intWidth = u16W;
    stVencInfo.intHeight = u16H;
    VencObj->UpdateInfo(stVencInfo);
    VencObj->GetInputPortInfo(0, stInputPortInfo);
    stInputPortInfo.bindPara = u16H;
    VencObj->UpdateInputPortInfo(0, stInputPortInfo);
}
static void * HdmiConvSensorMonitor(ST_TEM_BUFFER stBuf)
{
    SS_HdmiConv_SignalInfo_e enTcState;
    stMonitorDataPackage_t *pstPackage;
    SS_HdmiConv_AudInfo_t stAudioInfo;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    Sys *pSrcOb = NULL;
    Sys *pSrcObNew = NULL;

    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    ASSERT(sizeof(stMonitorDataPackage_t) == stBuf.u32TemBufferSize);
    pstPackage = (stMonitorDataPackage_t *)stBuf.pTemBuffer;
    if ((*pstPackage->pVectVideoPipeLine).size() == 0 || (*pstPackage->pVectNoSignalVideoPipeLine).size() == 0)
    {
        printf("Pipe line is empty!\n");
        return NULL;
    }
    MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_GET_STATE, sizeof(SS_HdmiConv_UsrCmd_e), (void *)&enTcState, E_MI_SNR_CUSTDATA_TO_USER);
    if (enTcState == EN_SIGNAL_LOCK)
    {
        MI_SNR_PlaneInfo_t stSnrPlane0Info;
        MI_U16 u16SnrWidth;
        MI_U16 u16SnrHeight;
        SS_HdmiConv_I2SMode_e enI2sMode = EN_I2S_MODE_NORMAL;

        MI_SNR_GetPlaneInfo((MI_SNR_PAD_ID_e)0, 0, &stSnrPlane0Info);
        u16SnrWidth = stSnrPlane0Info.stCapRect.u16Width;
        u16SnrHeight = stSnrPlane0Info.stCapRect.u16Height;
        if (pstPackage->enCurState != EN_SIGNAL_LOCK)
        {
            MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_GET_AUDIO_INFO, sizeof(SS_HdmiConv_AudInfo_t), (void *)&stAudioInfo, E_MI_SNR_CUSTDATA_TO_USER);
            MI_SNR_CustFunction((MI_SNR_PAD_ID_e)0, EN_CUST_CMD_CONFIG_I2S, sizeof(SS_HdmiConv_I2SMode_e), (void *)&enI2sMode, E_MI_SNR_CUSTDATA_TO_DRIVER);
            printf("Signal lock %d fmt %d audio sample rate %d audio bit width %d channels %d\n", enTcState, stAudioInfo.enAudioFormat, stAudioInfo.u32SampleRate, stAudioInfo.u8BitWidth, stAudioInfo.u8ChannelCount);
            ES8156_SelectWordLength(stAudioInfo.u8BitWidth);
            ES8156_ConfigI2s(enI2sMode);

            pSrcOb = (*pstPackage->pVectNoSignalVideoPipeLine)[(*pstPackage->pVectNoSignalVideoPipeLine).size() - 1];
            pSrcObNew = (*pstPackage->pVectVideoPipeLine)[(*pstPackage->pVectVideoPipeLine).size() - 1];
            SetVpeOut(u16SnrWidth, u16SnrHeight);
            SetVenc(u16SnrWidth, u16SnrHeight);
            Sys::Insert(*pstPackage->pVectVideoPipeLine);
            printf("Insert video done !\n");
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 0);
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 2);
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 4);
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 6);
            printf("Switch source done !\n");
            Sys::Extract(*pstPackage->pVectNoSignalVideoPipeLine);
            printf("Release file done !\n");
        }
        else
        {
            if (u16SnrWidth != pstPackage->u16SnrWidth || u16SnrHeight != pstPackage->u16SnrHeight)
            {
                Sys::Extract(*pstPackage->pVectVideoPipeLine);
                SetVpeOut(u16SnrWidth, u16SnrHeight);
                SetVenc(u16SnrWidth, u16SnrHeight);
                Sys::Insert(*pstPackage->pVectVideoPipeLine);
            }
        }
        pstPackage->u16SnrWidth = u16SnrWidth;
        pstPackage->u16SnrHeight = u16SnrHeight;
    }
    else
    {
        if (pstPackage->enCurState == EN_SIGNAL_LOCK)
        {
            printf("Signal unlock %d\n", enTcState);
            pSrcOb = (*pstPackage->pVectVideoPipeLine)[(*pstPackage->pVectVideoPipeLine).size() - 1];
            pSrcObNew = (*pstPackage->pVectNoSignalVideoPipeLine)[(*pstPackage->pVectNoSignalVideoPipeLine).size() - 1];
            Sys::Insert(*pstPackage->pVectNoSignalVideoPipeLine);
            printf("Insert source ok !\n");
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 0);
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 2);
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 4);
            Sys::SwtichSrc(pSrcOb, 0, pSrcObNew, 0, pstPackage->pDstObject, 6);
            printf("Switch source done !\n");
            Sys::Extract(*pstPackage->pVectVideoPipeLine);
            printf("Release video done !\n");
        }
    }
    pstPackage->enCurState = enTcState;
    return NULL;
}
static void HdmiConvInit(std::vector<Sys *> *pVectVideoPipeLine, std::vector<Sys *> *pVectNoSignalVideoPipeLine, Sys *pDstObj)
{
    ST_TEM_ATTR stTemAttr;
    stMonitorDataPackage_t stTmpPackage;
    ST_TEM_USER_DATA stData;
    SS_HdmiConv_MonitorCmd_e enCmd;

    ES8156_Init(); //Audio dac init

    memset(&stTmpPackage, 0, sizeof(stMonitorDataPackage_t));
    stTmpPackage.enCurState = EN_SIGNAL_NO_CONNECTION;
    stTmpPackage.pVectVideoPipeLine = pVectVideoPipeLine;
    stTmpPackage.pVectNoSignalVideoPipeLine = pVectNoSignalVideoPipeLine;
    stTmpPackage.pDstObject = pDstObj;
    PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
    memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
    stTemAttr.fpThreadDoSignal = HdmiConvDoCmd;
    stTemAttr.fpThreadWaitTimeOut = HdmiConvSensorMonitor;
    stTemAttr.u32ThreadTimeoutMs = 500;
    stTemAttr.bSignalResetTimer = 0;
    stTemAttr.stTemBuf.pTemBuffer = &stTmpPackage;
    stTemAttr.stTemBuf.u32TemBufferSize = sizeof(stMonitorDataPackage_t);
    stTemAttr.maxEventCout = 30;
    stTemAttr.bDropEvent = FALSE;
    TemOpen("hdmi_conv_monitor", stTemAttr);
    enCmd = EN_HDMICONV_START_MONITOR;
    stData.pUserData = (void *)&enCmd;
    stData.u32UserDataSize = sizeof(SS_HdmiConv_MonitorCmd_e);
    stData.u32BufferRealSize = 0;
    TemSend("hdmi_conv_monitor", stData);
    TemStartMonitor("hdmi_conv_monitor");
}
static void HdmiConvDeinit()
{
    ST_TEM_USER_DATA stData;
    SS_HdmiConv_MonitorCmd_e enCmd;

    TemStop("hdmi_conv_monitor");
    enCmd = EN_HDMICONV_EXIT_MONITOR;
    stData.pUserData = (void *)&enCmd;
    stData.u32UserDataSize = sizeof(SS_HdmiConv_MonitorCmd_e);
    stData.u32BufferRealSize = 0;
    TemSend("hdmi_conv_monitor", stData);
    TemClose("hdmi_conv_monitor");

    ES8156_Deinit();
}
int main(int argc, char **argv)
{
    std::map<std::string, unsigned int> mapModId;
    std::map<std::string, Sys *> maskMap;
    std::vector<Sys *> vectVideoPipeLine;
    std::vector<Sys *> vectNoSignalVideoPipeLine;

    std::string objName;
    Vif *VifObj = NULL;
    Vpe *VpeObj = NULL;
    Venc *VencObj = NULL;
    Inject *InjectObj = NULL;
    Divp *DivpObj = NULL;
    Venc *Venc1Obj = NULL;
    Rtsp *RtspObj = NULL;

    char getC = 0;

    if (argc != 2)
    {
        printf("Usage: ./%s xxx_ini_path\n", argv[0]);

        return -1;
    }
    mapModId["SNR"] = E_SYS_MOD_SNR;
    mapModId["RTSP"] = E_SYS_MOD_RTSP;
    mapModId["VENC"] = E_SYS_MOD_VENC;
    mapModId["VPE"] = E_SYS_MOD_VPE;
    mapModId["VIF"] = E_SYS_MOD_VIF;
    mapModId["AI"] = E_SYS_MOD_AI;
    mapModId["FILE"] = E_SYS_MOD_FILE;
    mapModId["EMPTY"] = E_SYS_MOD_EMPTY;
    mapModId["INJECT"] = E_SYS_MOD_INJECT;
    Sys::CreateObj(argv[1], mapModId);

    //Start to set signal object
    objName = "VIF_CH0_DEV0";
    VifObj = dynamic_cast<Vif *>(Sys::GetInstance(objName));
    if (!VifObj)
    {
        printf("Obj error!\n");        
        Sys::DestroyObj();
        return -1;
    }
    vectVideoPipeLine.push_back(VifObj);
    maskMap[objName] = VifObj;

    objName = "VPE_CH0_DEV0";
    VpeObj = dynamic_cast<Vpe *>(Sys::GetInstance(objName));   
    if (!VpeObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return -1;
    }
    vectVideoPipeLine.push_back(VpeObj);
    maskMap[objName] = VpeObj;

    objName = "VENC_CH0_DEV0";
    VencObj = dynamic_cast<Venc *>(Sys::GetInstance(objName));   
    if (!VencObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return -1;
    }
    vectVideoPipeLine.push_back(VencObj);
    maskMap[objName] = VencObj;

    //Start to set No signal object
    objName = "INJECT_CH0_DEV0";
    InjectObj = dynamic_cast<Inject *>(Sys::GetInstance(objName));
    if (!InjectObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return -1;
    }
    vectNoSignalVideoPipeLine.push_back(InjectObj);

    objName = "DIVP_CH0_DEV0";
    DivpObj = dynamic_cast<Divp *>(Sys::GetInstance(objName));
    if (!DivpObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return -1;
    }
    vectNoSignalVideoPipeLine.push_back(DivpObj);

    objName = "VENC_CH1_DEV0";
    Venc1Obj = dynamic_cast<Venc *>(Sys::GetInstance(objName));
    if (!Venc1Obj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return -1;
    }
    vectNoSignalVideoPipeLine.push_back(Venc1Obj);

    // default show no signal video, so no need to mask inject/divp/venc1.

    objName = "RTSP";
    RtspObj = dynamic_cast<Rtsp *>(Sys::GetInstance(objName));
    if (!RtspObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return -1;
    }
    Sys::Begin(maskMap);
    HdmiConvInit(&vectVideoPipeLine, &vectNoSignalVideoPipeLine, (Sys *)RtspObj);
    do
    {
        printf("Press 'q' to exit!\n");
        getC = getchar();
    }while (getC != 'q');
    HdmiConvDeinit();
    Sys::End(maskMap);
    Sys::DestroyObj();

    return 0;
}
