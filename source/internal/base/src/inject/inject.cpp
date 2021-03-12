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

#include "inject.h"
#include "mi_sys.h"
#if INTERFACE_RGN
#include "mi_rgn.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <stdio.h>

typedef struct stInjectBufHandler_s
{
    unsigned int uintOutPortId;
    MI_SYS_BUF_HANDLE sysBufHandle;
    MI_SYS_BufInfo_t stBufInfo;
    unsigned int bUpdateRgn;
    unsigned char bShowOsd;
}stInjectBufHandler_t;

#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((( (val)+(alignment)-1)/(alignment))*(alignment))
#endif
#if INTERFACE_RGN
#define RGN_PIXELFMT_BITCOUNT(pixelType, bits)     \
    do                                             \
    {                                              \
        switch (pixelType)                         \
        {                                          \
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:       \
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:       \
        case E_MI_RGN_PIXEL_FORMAT_RGB565:         \
            bits = 16;                             \
            break;                                 \
        case E_MI_RGN_PIXEL_FORMAT_I2:             \
            bits = 2;                              \
            break;                                 \
        case E_MI_RGN_PIXEL_FORMAT_I4:             \
            bits = 4;                              \
            break;                                 \
        case E_MI_RGN_PIXEL_FORMAT_I8:             \
            bits = 8;                              \
            break;                                 \
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888:       \
            bits = 32;                             \
            break;                                 \
        default:                                   \
            printf("wrong pixel type\n");          \
            bits = 16;                             \
            break;                                 \
        }                                          \
    } while (0);
#endif
bool Inject::bInitRgn = false;
unsigned int Inject::uintFrameCnt = 0;

Inject::Inject()
{
    bInitRgn = false;
}
Inject::~Inject()
{
    mapInjectOutInfo.clear();
}
void Inject::LoadDb()
{
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapInjectOut;

    for (itMapInjectOut = mapModOutputInfo.begin(); itMapInjectOut != mapModOutputInfo.end(); itMapInjectOut++)
    {
        mapInjectOutInfo[itMapInjectOut->first].uintBackGroudColor = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "BG_COLOR");
        mapInjectOutInfo[itMapInjectOut->first].uintVideoWidth = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "VID_W");
        mapInjectOutInfo[itMapInjectOut->first].uintVideoHeight = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "VID_H");
        mapInjectOutInfo[itMapInjectOut->first].uintVideoFmt = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "VID_FMT");
        mapInjectOutInfo[itMapInjectOut->first].bEnableOsd = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "IS_ENABLE_OSD");
        if (mapInjectOutInfo[itMapInjectOut->first].bEnableOsd)
        {
            mapInjectOutInfo[itMapInjectOut->first].strInjectOsdSrcFile = GetIniString(itMapInjectOut->second.curIoKeyString, "OSD_SRC_FILE");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdDelay = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "OSD_SHOW_DELAY");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdFmt = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "OSD_FMT");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdColor = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "OSD_COLOR");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdWidth = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "OSD_WIDTH");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdHeight = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "OSD_HEIGHT");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdShowFunction = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "OSD_SHOW_FUNCTION");
            mapInjectOutInfo[itMapInjectOut->first].uintOsdTargetPordId = GetIniUnsignedInt(itMapInjectOut->second.curIoKeyString, "OSD_ATTACH_PORT");
        }
    }
}
void Inject::BindBlock(stModInputInfo_t & stIn)
{

}
void Inject::UnBindBlock(stModInputInfo_t & stIn)
{

}

void * Inject::SenderMonitor(ST_TEM_BUFFER stBuf)
{
    std::map<unsigned int, void *>::iterator itMapUserIdToBufHandle;
#ifndef SSTAR_CHIP_I2
#ifndef SSTAR_CHIP_I2M
    MI_SYS_BUF_HANDLE sysDupBufHandle;
#endif
#endif
    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)stBuf.pTemBuffer;
    Inject *pThisClass = (Inject *)pReceiver->pSysClass;
    stInjectBufHandler_t *pInjectBufHandler = NULL;
    unsigned char bShowOsd = FALSE;
    unsigned uintOsdHandleId = 0;

    if (uintFrameCnt == pThisClass->mapModOutputInfo[pReceiver->uintPort].curFrmRate * pThisClass->mapInjectOutInfo[pReceiver->uintPort].uintOsdDelay)
    {
        bShowOsd = TRUE;
    }
    else
    {
        uintFrameCnt++;
    }
    for (itMapUserIdToBufHandle = pThisClass->mapUserIdToBufHandle.begin(); itMapUserIdToBufHandle != pThisClass->mapUserIdToBufHandle.end(); itMapUserIdToBufHandle++)
    {
        pInjectBufHandler = (stInjectBufHandler_t *)itMapUserIdToBufHandle->second;
        if (bShowOsd && !pInjectBufHandler->bUpdateRgn)
        {
#if INTERFACE_RGN
            MI_U32 u32CopyWidthBytes = 0;
            MI_U8 u8BitCount = 0;
            MI_RGN_CanvasInfo_t stCanvasInfo;
            int intOsdFd = 0;

            MI_RGN_GetCanvasInfo((MI_RGN_HANDLE)uintOsdHandleId, &stCanvasInfo);
            intOsdFd = open(pThisClass->mapInjectOutInfo[pReceiver->uintPort].strInjectOsdSrcFile.c_str(), O_RDONLY);
            {
                if (intOsdFd < 0)
                {
                    AMIGOS_ERR("Open osd src file [%s] error!\n", pThisClass->mapInjectOutInfo[pReceiver->uintPort].strInjectOsdSrcFile.c_str());
                    continue;
                }
            }
            RGN_PIXELFMT_BITCOUNT((MI_RGN_PixelFormat_e)pThisClass->mapInjectOutInfo[pReceiver->uintPort].uintOsdFmt, u8BitCount);
            u32CopyWidthBytes = ALIGN_UP(u8BitCount * pThisClass->mapInjectOutInfo[pReceiver->uintPort].uintOsdWidth , 8) / 8;
            for (MI_U32 i = 0; i < stCanvasInfo.stSize.u32Height; i++)
            {
                read(intOsdFd, (char *)(stCanvasInfo.virtAddr + i * stCanvasInfo.u32Stride), u32CopyWidthBytes);
            }
            close(intOsdFd);
            MI_RGN_UpdateCanvas((MI_RGN_HANDLE)uintOsdHandleId);
            pInjectBufHandler->bUpdateRgn = FALSE;
#endif
        }
#ifndef SSTAR_CHIP_I2
#ifndef SSTAR_CHIP_I2M
        MI_SYS_DupBuf(pInjectBufHandler->sysBufHandle, &sysDupBufHandle);
        MI_SYS_ChnInputPortPutBuf(sysDupBufHandle, &pInjectBufHandler->stBufInfo, FALSE);
#endif
#endif
        uintOsdHandleId++;
    }

    return NULL;
}

int Inject::CreateSender(unsigned int outPortId)
{
    ST_TEM_ATTR stTemAttr;
    stReceiverDesc_t stDest;

    uintFrameCnt = 0;
    PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
    memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
    stTemAttr.fpThreadDoSignal = NULL;
    stTemAttr.fpThreadWaitTimeOut = SenderMonitor;
    stTemAttr.u32ThreadTimeoutMs = (2000 + mapModOutputInfo[outPortId].curFrmRate ) / (2 * mapModOutputInfo[outPortId].curFrmRate);
    stTemAttr.bSignalResetTimer = 0;
    stTemAttr.stTemBuf.pTemBuffer = (void *)&(mapRecevier[outPortId]);
    stTemAttr.stTemBuf.u32TemBufferSize = 0;
    stTemAttr.maxEventCout = 30;
    stTemAttr.bDropEvent = FALSE;
    TemOpen(mapModOutputInfo[outPortId].curIoKeyString.c_str(), stTemAttr);

    return 0;
}
int Inject::DestroySender(unsigned int outPortId)
{
    std::map<unsigned int, void *>::iterator itMapUserIdToBufHandle;
    stInjectBufHandler_t *pInjectBufHandler = NULL;
    unsigned uintInjectId = 0;

    TemClose(mapModOutputInfo[outPortId].curIoKeyString.c_str());
    for (itMapUserIdToBufHandle = mapUserIdToBufHandle.begin(); itMapUserIdToBufHandle != mapUserIdToBufHandle.end(); itMapUserIdToBufHandle++)
    {
        pInjectBufHandler = (stInjectBufHandler_t *)itMapUserIdToBufHandle->second;
        if (pInjectBufHandler->uintOutPortId != outPortId)
        {
            uintInjectId++;
            continue;
        }
        MI_SYS_ChnInputPortPutBuf(pInjectBufHandler->sysBufHandle, &pInjectBufHandler->stBufInfo, FALSE);
#if INTERFACE_RGN
        if (mapInjectOutInfo[outPortId].bEnableOsd)
        {
            MI_RGN_Destroy((MI_RGN_HANDLE)uintInjectId);
        }
#endif
        uintInjectId++;
    }


    return 0;
}

void Inject::Init()
{
}
void Inject::Deinit()
{
    std::map<unsigned int, void *>::iterator itMapUserIdToBufHandle;
    stInjectBufHandler_t *pInjectBufHandler = NULL;

    for (itMapUserIdToBufHandle = mapUserIdToBufHandle.begin(); itMapUserIdToBufHandle != mapUserIdToBufHandle.end(); itMapUserIdToBufHandle++)
    {
        pInjectBufHandler = (stInjectBufHandler_t *)itMapUserIdToBufHandle->second;
        free(pInjectBufHandler);
        itMapUserIdToBufHandle->second = NULL;
    }
    if (bInitRgn)
    {
#if INTERFACE_RGN
        MI_RGN_DeInit();
#endif
        bInitRgn = false;
    }
    mapUserIdToBufHandle.clear();
}
#define RM  0x00FF0000
#define GM  0x0000FF00
#define BM  0x000000FF
#define Rval(data) ( (data & RM) >> 16)
#define Gval(data) ( (data & GM) >> 8)
#define Bval(data) ( (data & BM) )
void Inject::Start()
{
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BUF_HANDLE hHandle;
    MI_U8 *pCharHolder = NULL;
    MI_U8 *pDataTo[2] = {NULL, NULL};
    MI_S32 s32Ret = 0;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapInjectOut;
    std::map<unsigned int, stInjectOutInfo_t>::iterator itMapInjectOutInfo;
    std::vector<stModIoInfo_t>::iterator itVectNext;
    Sys *pNextClass;
    stModDesc_t stDesc;
    unsigned uintInjectId = 0;

    for (itMapInjectOutInfo = mapInjectOutInfo.begin(); itMapInjectOutInfo != mapInjectOutInfo.end(); itMapInjectOutInfo++)
    {
        itMapInjectOut = mapModOutputInfo.find(itMapInjectOutInfo->first);
        if (itMapInjectOut == mapModOutputInfo.end())
        {
            ASSERT(0);
        }
        for (itVectNext = itMapInjectOut->second.vectNext.begin(); itVectNext != itMapInjectOut->second.vectNext.end(); itVectNext++)
        {
            pNextClass = GetInstance(itVectNext->modKeyString);
            if (!pNextClass)
            {
                ASSERT(0);
            }
            // set bufconf
            memset(&stBufConf, 0, sizeof(MI_SYS_BufConf_t));
            memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
            memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
            MI_SYS_GetCurPts(&stBufConf.u64TargetPts);
            stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
            stBufConf.stFrameCfg.eFormat = (MI_SYS_PixelFormat_e)itMapInjectOutInfo->second.uintVideoFmt;
            stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
            stBufConf.stFrameCfg.u16Width = (MI_U16)itMapInjectOutInfo->second.uintVideoWidth;
            stBufConf.stFrameCfg.u16Height = (MI_U16)itMapInjectOutInfo->second.uintVideoHeight;
            pNextClass->GetModDesc(stDesc);
            stChnPort.eModId = (MI_ModuleId_e)stDesc.modId;
            stChnPort.u32DevId = (MI_U32)stDesc.devId;
            stChnPort.u32ChnId = (MI_U32)stDesc.chnId;
            stChnPort.u32PortId = (MI_U32)itVectNext->portId;
            s32Ret = MI_SYS_ChnInputPortGetBuf(&stChnPort, &stBufConf, &stBufInfo, &hHandle, 3000);
            if (s32Ret == MI_SUCCESS)
            {
                MI_FLOAT fY,fU,fV;
                MI_U32 u32Y,u32Cr,u32Cb;
                stInjectBufHandler_t *pInjectBufInfo = NULL;

                fY = (0.299 * Rval(itMapInjectOutInfo->second.uintBackGroudColor))
                     + (0.587 * Gval(itMapInjectOutInfo->second.uintBackGroudColor))
                     + (0.114 * Bval(itMapInjectOutInfo->second.uintBackGroudColor));
                fU = (-0.168736 * Rval(itMapInjectOutInfo->second.uintBackGroudColor))
                     - (0.331264 * Gval(itMapInjectOutInfo->second.uintBackGroudColor))
                     + (0.5000 * Bval(itMapInjectOutInfo->second.uintBackGroudColor)) + 128;
                fV = (0.5000 * Rval(itMapInjectOutInfo->second.uintBackGroudColor))
                     - (0.418688 * Gval(itMapInjectOutInfo->second.uintBackGroudColor))
                     - (0.081312 * Bval(itMapInjectOutInfo->second.uintBackGroudColor)) + 128;
                u32Y = fY;
                u32Cb = fU;
                u32Cr = fV;

                if (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == stBufConf.stFrameCfg.eFormat)
                {
                    // Copy y data
                    pDataTo[0] = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0];
                    for (MI_U32 i = 0; i < stBufInfo.stFrameData.u16Height; i++)
                    {
                        memset(pDataTo[0] + i * stBufInfo.stFrameData.u32Stride[0], u32Y, stBufConf.stFrameCfg.u16Width);
                    }
                    // Copy uv data
                    pDataTo[1] = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[1];
                    for (MI_U32 i = 0; i < stBufInfo.stFrameData.u16Height / 2; i++)
                    {
                        for (MI_U32 j = 0; j < stBufConf.stFrameCfg.u16Width; j += 2)
                        {
                            pCharHolder = pDataTo[1] + i * stBufInfo.stFrameData.u32Stride[1] + j;
                            pCharHolder[0] = (MI_U8)u32Cb;
                            pCharHolder[1] = (MI_U8)u32Cr;
                        }
                    }
                }
                else if (E_MI_SYS_PIXEL_FRAME_YUV422_YUYV == stBufConf.stFrameCfg.eFormat)
                {
                    pDataTo[0] = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0];
                    for (MI_U32 i = 0; i < stBufInfo.stFrameData.u16Height; i++)
                    {
                        for (MI_U32 j = 0; j < stBufConf.stFrameCfg.u16Width * 2; j += 4)
                        {
                            pCharHolder = pDataTo[0] + i * stBufInfo.stFrameData.u32Stride[0] + j;
                            pCharHolder[0] = (MI_U8)u32Y;
                            pCharHolder[1] = (MI_U8)u32Cb;
                            pCharHolder[2] = (MI_U8)u32Y;
                            pCharHolder[3] = (MI_U8)u32Cr;
                        }
                    }
                }
#ifndef SSTAR_CHIP_I2
                else if (E_MI_SYS_PIXEL_FRAME_YUV422_UYVY == stBufConf.stFrameCfg.eFormat)
                {
                    pDataTo[0] = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0];
                    for (MI_U32 i = 0; i < stBufInfo.stFrameData.u16Height; i++)
                    {
                        for (MI_U32 j = 0; j < stBufConf.stFrameCfg.u16Width * 2; j += 4)
                        {
                            pCharHolder = pDataTo[0] + i * stBufInfo.stFrameData.u32Stride[0] + j;
                            pCharHolder[0] = (MI_U8)u32Cb;
                            pCharHolder[1] = (MI_U8)u32Y;
                            pCharHolder[2] = (MI_U8)u32Cr;
                            pCharHolder[3] = (MI_U8)u32Y;
                        }
                    }
                }
                else if (E_MI_SYS_PIXEL_FRAME_YUV422_YVYU == stBufConf.stFrameCfg.eFormat)
                {
                    pDataTo[0] = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0];
                    for (MI_U32 i = 0; i < stBufInfo.stFrameData.u16Height; i++)
                    {
                        for (MI_U32 j = 0; j < stBufConf.stFrameCfg.u16Width * 2; j += 4)
                        {
                            pCharHolder = pDataTo[0] + i * stBufInfo.stFrameData.u32Stride[0] + j;
                            pCharHolder[0] = (MI_U8)u32Y;
                            pCharHolder[1] = (MI_U8)u32Cr;
                            pCharHolder[2] = (MI_U8)u32Y;
                            pCharHolder[3] = (MI_U8)u32Cb;
                        }
                    }
                }
                else if (E_MI_SYS_PIXEL_FRAME_YUV422_VYUY == stBufConf.stFrameCfg.eFormat)
                {
                    pDataTo[0] = (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0];
                    for (MI_U32 i = 0; i < stBufInfo.stFrameData.u16Height; i++)
                    {
                        for (MI_U32 j = 0; j < stBufConf.stFrameCfg.u16Width * 2; j += 4)
                        {
                            pCharHolder = pDataTo[0] + i * stBufInfo.stFrameData.u32Stride[0] + j;
                            pCharHolder[0] = (MI_U8)u32Cr;
                            pCharHolder[1] = (MI_U8)u32Y;
                            pCharHolder[2] = (MI_U8)u32Cb;
                            pCharHolder[3] = (MI_U8)u32Y;
                        }
                    }
                }
#endif
                pInjectBufInfo = (stInjectBufHandler_t *)malloc(sizeof(stInjectBufHandler_t));
                ASSERT(pInjectBufInfo);
                memset(pInjectBufInfo, 0, sizeof(stInjectBufHandler_t));
                mapUserIdToBufHandle[uintInjectId] = (void *)pInjectBufInfo;
                pInjectBufInfo->sysBufHandle = hHandle;
                pInjectBufInfo->stBufInfo = stBufInfo;
                pInjectBufInfo->uintOutPortId = itMapInjectOutInfo->first;
#if INTERFACE_RGN
                if (itMapInjectOutInfo->second.bEnableOsd)
                {
                    MI_RGN_Attr_t stRegion;
                    MI_RGN_ChnPort_t stChnPort;
                    MI_RGN_ChnPortParam_t stChnAttr;

                    if (!bInitRgn)
                    {
                        MI_RGN_PaletteTable_t stRgnPaletteTable;

                        memset(&stRgnPaletteTable, 0, sizeof(MI_RGN_PaletteTable_t));
                        stRgnPaletteTable.astElement[1].u8Alpha = 0xFF;
                        stRgnPaletteTable.astElement[1].u8Red = Rval(itMapInjectOutInfo->second.uintOsdColor);
                        stRgnPaletteTable.astElement[1].u8Green = Gval(itMapInjectOutInfo->second.uintOsdColor);
                        stRgnPaletteTable.astElement[1].u8Blue = Bval(itMapInjectOutInfo->second.uintOsdColor);
                        MI_RGN_Init(&stRgnPaletteTable);

                        bInitRgn = true;
                    }
                    stRegion.eType = E_MI_RGN_TYPE_OSD;
                    stRegion.stOsdInitParam.ePixelFmt = (MI_RGN_PixelFormat_e)itMapInjectOutInfo->second.uintOsdFmt;
                    stRegion.stOsdInitParam.stSize.u32Width = (MI_U32)itMapInjectOutInfo->second.uintOsdWidth;
                    stRegion.stOsdInitParam.stSize.u32Height = (MI_U32)itMapInjectOutInfo->second.uintOsdHeight;
                    MI_RGN_Create((MI_RGN_HANDLE)uintInjectId, &stRegion);
                    memset(&stChnAttr, 0, sizeof(MI_RGN_ChnPortParam_t));
                    stChnPort.eModId = (stDesc.modId == E_MI_MODULE_ID_DIVP) ? E_MI_RGN_MODID_DIVP : E_MI_RGN_MODID_VPE;
                    stChnPort.s32DevId = (MI_S32)stDesc.devId;
                    stChnPort.s32ChnId = (MI_S32)stDesc.chnId;
                    stChnPort.s32OutputPortId = itMapInjectOutInfo->second.uintOsdTargetPordId;
                    stChnAttr.bShow = TRUE;
                    stChnAttr.stPoint.u32X = ((itMapInjectOutInfo->second.uintVideoWidth > itMapInjectOutInfo->second.uintOsdWidth)?
                                             (itMapInjectOutInfo->second.uintVideoWidth - itMapInjectOutInfo->second.uintOsdWidth): itMapInjectOutInfo->second.uintVideoWidth) / 2;
                    stChnAttr.stPoint.u32Y = ((itMapInjectOutInfo->second.uintVideoHeight > itMapInjectOutInfo->second.uintOsdHeight)?
                                             (itMapInjectOutInfo->second.uintVideoHeight - itMapInjectOutInfo->second.uintOsdHeight): itMapInjectOutInfo->second.uintVideoHeight) / 2;
#ifndef SSTAR_CHIP_I2
                    stChnAttr.unPara.stOsdChnPort.u32Layer = 0;
                    stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.bEnableColorInv = 0;
                    stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.eInvertColorMode = (MI_RGN_InvertColorMode_e)0;
                    stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.u16LumaThreshold = 0;
                    stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.u16WDivNum = 0;
                    stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.u16HDivNum = 0;
                    stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
                    stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
                    stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xFF;
#endif
                    MI_RGN_AttachToChn((MI_RGN_HANDLE)uintInjectId, &stChnPort, &stChnAttr);
                }
#endif
                uintInjectId++;
            }
        }
    }
}

void Inject::Stop()
{
}

