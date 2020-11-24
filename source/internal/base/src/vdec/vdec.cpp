#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <vector>
#include <string>

#include "vdec.h"
#include "mi_sys.h"
#include "mi_vdec.h"
#include "mi_common.h"


Vdec::Vdec()
{
}
Vdec::~Vdec()
{
}
void Vdec::LoadDb()
{
    std::map<unsigned int, stModOutputInfo_t>::iterator itVdecOut;
    stDecOutInfo_t stDecOutInfo;

    stVdecInfo.dpBufMode = GetIniInt(stModDesc.modKeyString,"BUF_MODE");
    stVdecInfo.refFrameNum = GetIniInt(stModDesc.modKeyString,"REF_FRAME");
    stVdecInfo.bitstreamSize = GetIniInt(stModDesc.modKeyString,"BIT_STREAM_BUFFER");
    stVdecInfo.uintBufWidth = GetIniInt(stModDesc.modKeyString,"BUF_WIDTH");
    stVdecInfo.uintBufHeight = GetIniInt(stModDesc.modKeyString,"BUF_HEIGHT");
    for (itVdecOut = mapModOutputInfo.begin(); itVdecOut != mapModOutputInfo.end(); itVdecOut++)
    {
        memset(&stDecOutInfo, 0, sizeof(stDecOutInfo_t));
        stDecOutInfo.intPortId = itVdecOut->second.curPortId;
        stDecOutInfo.uintDecOutWidth = GetIniInt(itVdecOut->second.curIoKeyString, "VID_W");
        stDecOutInfo.uintDecOutHeight = GetIniInt(itVdecOut->second.curIoKeyString, "VID_H");
        vDecOutInfo.push_back(stDecOutInfo);
        itVdecOut->second.stStreanInfo.eStreamType = E_STREAM_YUV420;
        itVdecOut->second.stStreanInfo.stFrameInfo.streamWidth = stDecOutInfo.uintDecOutWidth;
        itVdecOut->second.stStreanInfo.stFrameInfo.streamHeight = stDecOutInfo.uintDecOutHeight;
    }
}
void Vdec::Incoming(stStreamInfo_t *pInfo)
{
    MI_VDEC_CodecType_e eCodecType;
    MI_VDEC_ChnAttr_t stVdecChnAttr;

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    MI_VDEC_GetChnAttr((MI_VDEC_CHN)stModDesc.chnId, &stVdecChnAttr);

    switch (pInfo->eStreamType)
    {
        case E_STREAM_H264:
            eCodecType = E_MI_VDEC_CODEC_TYPE_H264;
            break;
        case E_STREAM_H265:
            eCodecType = E_MI_VDEC_CODEC_TYPE_H265;
            break;
        case E_STREAM_JPEG:
            eCodecType = E_MI_VDEC_CODEC_TYPE_JPEG;
            break;
        default:
            AMIGOS_ERR("Fmt error %d\n", pInfo->eStreamType);
            ASSERT(0);
    }
    if (eCodecType != stVdecChnAttr.eCodecType)
    {
#ifndef SSTAR_CHIP_I2
        MI_VDEC_OutputPortAttr_t stOutputPortAttr;
#endif
        std::vector<stDecOutInfo_t>::iterator itVdecOut;

        AMIGOS_INFO("Codec type is different need reset to %d\n", eCodecType);
        MI_VDEC_StopChn((MI_VDEC_CHN)stModDesc.chnId);
        MI_VDEC_DestroyChn((MI_VDEC_CHN)stModDesc.chnId);
        stVdecChnAttr.eCodecType = eCodecType;
        MI_VDEC_CreateChn(stModDesc.chnId, &stVdecChnAttr);
        MI_VDEC_StartChn(stModDesc.chnId);
        for (itVdecOut = vDecOutInfo.begin(); itVdecOut != vDecOutInfo.end(); itVdecOut++)
        {
#ifndef SSTAR_CHIP_I2
            memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
            stOutputPortAttr.u16Width = itVdecOut->uintDecOutWidth;
            stOutputPortAttr.u16Height = itVdecOut->uintDecOutHeight;
            MI_VDEC_SetOutputPortAttr((MI_VDEC_CHN)stModDesc.chnId, &stOutputPortAttr);
#endif
            mapModOutputInfo[itVdecOut->intPortId].stStreanInfo.stFrameInfo.streamWidth = itVdecOut->uintDecOutWidth;
            mapModOutputInfo[itVdecOut->intPortId].stStreanInfo.stFrameInfo.streamHeight = itVdecOut->uintDecOutHeight;
        }
    }
}
void Vdec::Outcoming()
{
    MI_VDEC_StopChn((MI_VDEC_CHN)stModDesc.chnId);
    MI_VDEC_DestroyChn((MI_VDEC_CHN)stModDesc.chnId);
}
void Vdec::ResetOut(unsigned int outPortId, stStreamInfo_t *pInfo)
{
#ifndef SSTAR_CHIP_I2
    std::vector<stDecOutInfo_t>::iterator itVdecOut;
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;

    for (itVdecOut = vDecOutInfo.begin(); itVdecOut != vDecOutInfo.end(); itVdecOut++)
    {
        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        stOutputPortAttr.u16Width = pInfo->stFrameInfo.streamWidth;
        stOutputPortAttr.u16Height = pInfo->stFrameInfo.streamHeight;
        MI_VDEC_SetOutputPortAttr((MI_VDEC_CHN)stModDesc.chnId, &stOutputPortAttr);
    }
#endif
    AMIGOS_INFO("Vdec reset out to w %d h %d format %d\n", pInfo->stFrameInfo.streamWidth, pInfo->stFrameInfo.streamHeight, pInfo->eStreamType);
}

void Vdec::Init()
{
    MI_VDEC_ChnAttr_t stVdecChnAttr;
#ifndef SSTAR_CHIP_I2
    MI_VDEC_OutputPortAttr_t stOutputPortAttr;
#endif
    std::vector<stDecOutInfo_t>::iterator itVdecOut;

    memset(&stVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
    stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = stVdecInfo.refFrameNum;
    stVdecChnAttr.eVideoMode    = E_MI_VDEC_VIDEO_MODE_FRAME;
    stVdecChnAttr.u32BufSize    = stVdecInfo.bitstreamSize * 1024 * 1024;
    stVdecChnAttr.u32PicWidth   = stVdecInfo.uintBufWidth;
    stVdecChnAttr.u32PicHeight  = stVdecInfo.uintBufHeight;
    stVdecChnAttr.u32Priority   = 0;
    stVdecChnAttr.eCodecType = E_MI_VDEC_CODEC_TYPE_H265;
#ifndef SSTAR_CHIP_I2
    stVdecChnAttr.eDpbBufMode = (MI_VDEC_DPB_BufMode_e)stVdecInfo.dpBufMode;
#endif
    MI_VDEC_CreateChn(stModDesc.chnId, &stVdecChnAttr);
    MI_VDEC_StartChn(stModDesc.chnId);
    for (itVdecOut = vDecOutInfo.begin(); itVdecOut != vDecOutInfo.end(); itVdecOut++)
    {
        MI_SYS_ChnPort_t stChnPort;

#ifndef SSTAR_CHIP_I2
        memset(&stOutputPortAttr, 0, sizeof(MI_VDEC_OutputPortAttr_t));
        stOutputPortAttr.u16Width = itVdecOut->uintDecOutWidth;
        stOutputPortAttr.u16Height = itVdecOut->uintDecOutHeight;
        MI_VDEC_SetOutputPortAttr((MI_VDEC_CHN)stModDesc.chnId, &stOutputPortAttr);
#endif
        stChnPort.eModId = E_MI_MODULE_ID_VDEC;
        stChnPort.u32ChnId = stModDesc.chnId;
        stChnPort.u32DevId = stModDesc.devId;
        stChnPort.u32PortId = itVdecOut->intPortId;
        MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 5);
    }

}
void Vdec::Deinit()
{
}
