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
#include <assert.h>

#include <vector>
#include <string>

#include "mi_venc.h"
#include "venc.h"



static MI_S32 Venc_CreateChannel(MI_VENC_CHN VencChn, MI_VENC_ChnAttr_t *pstAttr)
{
    MI_U32 u32DevId = -1;

    if (pstAttr == NULL)
    {
        AMIGOS_INFO("invalid param\n");
        return -1;
    }

    ExecFunc(MI_VENC_CreateChn(VencChn, pstAttr), MI_SUCCESS);
    if (pstAttr->stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        MI_VENC_ParamJpeg_t stParamJpeg;

        memset(&stParamJpeg, 0, sizeof(stParamJpeg));
        ExecFunc(MI_VENC_GetJpegParam(VencChn, &stParamJpeg), MI_SUCCESS);

        AMIGOS_INFO("Get u32Qfactor:%d\n", stParamJpeg.u32Qfactor);

        stParamJpeg.u32Qfactor = 50;

        ExecFunc(MI_VENC_SetJpegParam(VencChn, &stParamJpeg), MI_SUCCESS);
    }

    ExecFunc(MI_VENC_GetChnDevid(VencChn, &u32DevId), MI_SUCCESS);
    return MI_SUCCESS;
}

static MI_S32 Venc_DestoryChannel(MI_VENC_CHN VencChn)
{
    ExecFunc(MI_VENC_StopRecvPic(VencChn), MI_SUCCESS);
    ExecFunc(MI_VENC_DestroyChn(VencChn), MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 Venc_StartChannel(MI_VENC_CHN VencChn)
{
    ExecFunc(MI_VENC_StartRecvPic(VencChn), MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 Venc_StopChannel(MI_VENC_CHN VencChn)
{
    ExecFunc(MI_VENC_StopRecvPic(VencChn), MI_SUCCESS);

    return MI_SUCCESS;
}

Venc::Venc()
{
}
Venc::~Venc()
{
}
void Venc::LoadDb()
{
    stVencInfo.intWidth = GetIniInt(stModDesc.modKeyString,"STREAM_W");
    stVencInfo.intHeight = GetIniInt(stModDesc.modKeyString,"STREAM_H");
    stVencInfo.intBitRate = GetIniInt(stModDesc.modKeyString,"BIT_RATE");
    stVencInfo.intEncodeType = GetIniInt(stModDesc.modKeyString,"EN_TYPE");
    stVencInfo.intEncodeFps = GetIniInt(stModDesc.modKeyString,"EN_FPS");
    stVencInfo.intMultiSlice = GetIniInt(stModDesc.modKeyString,"MULTI_SLICE");
    stVencInfo.intSliceRowCnt = GetIniInt(stModDesc.modKeyString,"SLICE_ROW_CNT");
    mapModOutputInfo[0].stStreanInfo.eStreamType = (E_STREAM_TYPE)stVencInfo.intEncodeType;
    mapModOutputInfo[0].stStreanInfo.stFrameInfo.streamWidth = stVencInfo.intWidth;
    mapModOutputInfo[0].stStreanInfo.stFrameInfo.streamHeight = stVencInfo.intHeight;
}
void Venc::Init()
{
    MI_VENC_ChnAttr_t stChnAttr;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;

    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));
    switch (stVencInfo.intEncodeType)
    {
        case E_STREAM_H264:
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = (MI_U32)stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = (MI_U32)stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = (MI_U32)stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = (MI_U32)stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrH264e.bByFrame = (stVencInfo.intMultiSlice != -1) ? !stVencInfo.intMultiSlice : TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264VBR;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = ((MI_U32)stVencInfo.intBitRate) * 1024 * 1024;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = (MI_U32)((stVencInfo.intEncodeFps != -1) ? stVencInfo.intEncodeFps: 30);
            stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32Gop = stChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum * 2;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32StatTime = 0;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp = 48;
            stChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp = 20;
        }
        break;
        case E_STREAM_H265:
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = (MI_U32)stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = (MI_U32)stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = (MI_U32)stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = (MI_U32)stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrH265e.bByFrame = (stVencInfo.intMultiSlice != -1) ? !stVencInfo.intMultiSlice : TRUE;
            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxBitRate = ((MI_U32)stVencInfo.intBitRate) * 1024 * 1024;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = (MI_U32)((stVencInfo.intEncodeFps != -1) ? stVencInfo.intEncodeFps: 30);
            stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32Gop = stChnAttr.stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum * 2;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32StatTime = 0;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MaxQp = 48;
            stChnAttr.stRcAttr.stAttrH265Vbr.u32MinQp = 20;
        }
        break;
        case E_STREAM_JPEG:
        {
            stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = (MI_U32)stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = (MI_U32)stVencInfo.intHeight;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = (MI_U32)stVencInfo.intWidth;
            stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = (MI_U32)stVencInfo.intHeight;

            stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
#ifndef SSTAR_CHIP_I2
            stChnAttr.stRcAttr.stAttrMjpegCbr.u32BitRate = 30;
            stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateNum = (MI_U32)((stVencInfo.intEncodeFps != -1) ? stVencInfo.intEncodeFps: 30);
            stChnAttr.stRcAttr.stAttrMjpegCbr.u32SrcFrmRateDen = 1;
#endif
        }
        break;
        default:
            assert(0);
    }
    Venc_CreateChannel((MI_VENC_CHN)stModDesc.chnId, &stChnAttr);
    MI_VENC_GetChnDevid((MI_VENC_CHN)stModDesc.chnId, &stModDesc.devId);
    if ((stVencInfo.intMultiSlice != -1) ? stVencInfo.intMultiSlice : FALSE)
    {
        switch (stVencInfo.intEncodeType)
        {
            case E_STREAM_H264:
                MI_VENC_ParamH264SliceSplit_t stH264SliceSplit;

                stH264SliceSplit.bSplitEnable = stVencInfo.intMultiSlice;
                stH264SliceSplit.u32SliceRowCount = stVencInfo.intSliceRowCnt;
                MI_VENC_SetH264SliceSplit((MI_VENC_CHN)stModDesc.chnId, &stH264SliceSplit);
                break;
            case E_STREAM_H265:
                MI_VENC_ParamH265SliceSplit_t stH265SliceSplit;

                stH265SliceSplit.bSplitEnable = stVencInfo.intMultiSlice;
                stH265SliceSplit.u32SliceRowCount = stVencInfo.intSliceRowCnt;
                MI_VENC_SetH265SliceSplit((MI_VENC_CHN)stModDesc.chnId, &stH265SliceSplit);
                break;
            default:
                break;
        }
    }

#ifndef SSTAR_CHIP_I2
    MI_VENC_InputSourceConfig_t stVenInSrc;
    MI_SYS_BindType_e eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    MI_U32 u32BindParam = 0;
    MI_S32 s32Ret = 0;

    memset(&stVenInSrc, 0, sizeof(MI_VENC_InputSourceConfig_t));
    for (itMapIn = mapModInputInfo.begin(); itMapIn != mapModInputInfo.end(); ++itMapIn)
    {
        eBindType = (MI_SYS_BindType_e)GetIniInt(itMapIn->second.curIoKeyString, "BIND_TYPE");
        if (eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
        {
            u32BindParam = GetIniInt(itMapIn->second.curIoKeyString, "BIND_PARAM");
            if (u32BindParam == (MI_U32)stVencInfo.intHeight)
            {
                stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_ONE_FRM;
                s32Ret = MI_VENC_SetInputSourceConfig((MI_VENC_CHN)stModDesc.chnId, &stVenInSrc);
                AMIGOS_INFO("Set ring one frame mode! Chn %d height %d ret %d\n", stModDesc.chnId, stVencInfo.intHeight, s32Ret);
            }
            else
            {
                stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_HALF_FRM;
                s32Ret = MI_VENC_SetInputSourceConfig((MI_VENC_CHN)stModDesc.chnId, &stVenInSrc);
                AMIGOS_INFO("Set ring half frame mode! Chn %d height %d ret %d\n", stModDesc.chnId, stVencInfo.intHeight, s32Ret);
            }
        }
        else
        {
            stVenInSrc.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_NORMAL_FRMBASE;
            s32Ret = MI_VENC_SetInputSourceConfig((MI_VENC_CHN)stModDesc.chnId, &stVenInSrc);
            AMIGOS_INFO("Set frame mode! ret %d\n", s32Ret);
        }
    }
#endif
    Venc_StartChannel((MI_VENC_CHN)stModDesc.chnId);
}
void Venc::ResetOut(unsigned int outPortId, stStreamInfo_t *pInfo)
{
    stStreamInfo_t stPreStreamInfo;

    GetInputStreamInfo(0, &stPreStreamInfo);
    stPreStreamInfo.stFrameInfo.streamWidth = pInfo->stFrameInfo.streamWidth;
    stPreStreamInfo.stFrameInfo.streamHeight = pInfo->stFrameInfo.streamHeight;
    UpdateInputStreamInfo(0, &stPreStreamInfo);
    stVencInfo.intEncodeType = pInfo->eStreamType;
    Deinit();
    Init();
    AMIGOS_INFO("Venc reset out to w %d h %d format %d\n", pInfo->stFrameInfo.streamWidth, pInfo->stFrameInfo.streamHeight, pInfo->eStreamType);
}

void Venc::Deinit()
{
    Venc_StopChannel((MI_VENC_CHN)stModDesc.chnId);
    Venc_DestoryChannel((MI_VENC_CHN)stModDesc.chnId);
}
