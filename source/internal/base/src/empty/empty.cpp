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

#include "empty.h"
#include "mi_sys.h"
#include <stdio.h>

Empty::Empty()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
Empty::~Empty()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}
void Empty::BindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;
    MI_SYS_ChnPort_t stChnPort;

    memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    stChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId;
    stChnPort.u32ChnId = (MI_U32)stPreDesc.chnId;
    stChnPort.u32PortId = (MI_U32)stIn.stPrev.portId;
    stChnPort.u32DevId = (MI_U32)stPreDesc.devId;
    MI_SYS_SetChnOutputPortDepth(&stChnPort, 2, 3);
    AMIGOS_INFO("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
    CreateReceiver(stIn.curPortId, DataReceiver, this);

}
void Empty::UnBindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    AMIGOS_INFO("UnBind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
    DestroyReceiver(stIn.curPortId);
}

void Empty::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{

}

int Empty::StartSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Empty::StopSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Empty::CreateSender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}
int Empty::DestroySender(unsigned int outPortId)
{
    AMIGOS_INFO("func: %s, Mod %s\n", __FUNCTION__, stModDesc.modKeyString.c_str());

    return 0;
}

void Empty::Init()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
    
}

void Empty::Deinit()
{
    AMIGOS_INFO("func: %s\n", __FUNCTION__);
}

