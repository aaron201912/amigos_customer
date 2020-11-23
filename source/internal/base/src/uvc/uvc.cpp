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

#include "uvc.h"
#include <stdio.h>

Uvc::Uvc()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
Uvc::~Uvc()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
void Uvc::LoadDb()
{
}
void Uvc::BindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    AMIGOS_INFO("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
    CreateReceiver(stIn.curPortId, DataReceiver, this);

}
void Uvc::UnBindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    AMIGOS_INFO("UnBind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
    DestroyReceiver(stIn.curPortId);
}

void Uvc::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    stStreamData_t *pStreamData = NULL;
    Uvc *pThisClass = NULL;

    ASSERT(dataSize == sizeof(stStreamData_t));
    pThisClass = dynamic_cast<Uvc *>((Sys *)pUsrData);;
    pStreamData = (stStreamData_t*)pData;
    if (portId == 0 && (pStreamData->stInfo.eStreamType == E_STREAM_H264 || pStreamData->stInfo.eStreamType == E_STREAM_H265 || pStreamData->stInfo.eStreamType == E_STREAM_JPEG))
    {
        //Uvc do send es data.
    }
    else if (portId == 1 && (pStreamData->stInfo.eStreamType == E_STREAM_YUV422 || pStreamData->stInfo.eStreamType == E_STREAM_YUV420))
    {
        //uvc send yuv data
    }
    else
    {
        AMIGOS_ERR("Uac not support current port!\n");
    }
}
int Uvc::StartSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Uvc::StopSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Uvc::CreateSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Uvc::DestroySender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}

void Uvc::Init()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
    
}
void Uvc::Deinit()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}

