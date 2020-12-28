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

#include "uac.h"
#include <stdio.h>

Uac::Uac()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
Uac::~Uac()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
void Uac::LoadDb()
{
}
void Uac::BindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    AMIGOS_INFO("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
    CreateReceiver(stIn.curPortId, DataReceiver, this);

}
void Uac::UnBindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    AMIGOS_INFO("UnBind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
    DestroyReceiver(stIn.curPortId);
}

void Uac::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    stStreamData_t *pStreamData = NULL;
    Uac *pThisClass = NULL;

    ASSERT(dataSize == sizeof(stStreamData_t));
    pThisClass = dynamic_cast<Uac *>((Sys *)pUsrData);;
    pStreamData = (stStreamData_t*)pData;
    if (portId == 0)
    {
        ASSERT(pStreamData->stInfo.eStreamType == E_STREAM_AUDIO_CODEC_DATA);
        //Uac do send pcm data.
    }
    else
    {
        AMIGOS_ERR("Uac not support current port!\n");
    }
}
int Uac::StartSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Uac::StopSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Uac::CreateSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Uac::DestroySender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
void Uac::Init()
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());
    
}
void Uac::Deinit()
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());
}

