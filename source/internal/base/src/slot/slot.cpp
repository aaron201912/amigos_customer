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
#include "mi_sys.h"
#include "slot.h"

typedef struct stSys_BindInfo_s
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
#ifndef SSTAR_CHIP_I2
    MI_SYS_BindType_e eBindType;
    MI_U32 u32BindParam;
#endif
} stSys_BindInfo_T;

Slot::Slot()
{
}
Slot::~Slot()
{
}
void Slot::LoadDb()
{
    std::map<unsigned int, stModInputInfo_t>::iterator itMapSlotIn;
    for (itMapSlotIn = mapModInputInfo.begin(); itMapSlotIn != mapModInputInfo.end(); itMapSlotIn++)
    {
        mapSlotInputInfo[itMapSlotIn->second.curPortId].uintDstBindMod = GetIniUnsignedInt(itMapSlotIn->second.curIoKeyString, "DST_MOD");
        mapSlotInputInfo[itMapSlotIn->second.curPortId].uintDstBindDev = GetIniUnsignedInt(itMapSlotIn->second.curIoKeyString, "DST_DEV");
        mapSlotInputInfo[itMapSlotIn->second.curPortId].uintDstBindChannel = GetIniUnsignedInt(itMapSlotIn->second.curIoKeyString, "DST_CHN");
        mapSlotInputInfo[itMapSlotIn->second.curPortId].uintDstBindPort = GetIniUnsignedInt(itMapSlotIn->second.curIoKeyString, "DST_PORT");
    }
}
void Slot::BindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;
    stSys_BindInfo_T stBindInfo;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    if (mapSysModuleType[stPreDesc.modId] == E_STREAM_OUT_DATA_IN_KERNEL_MODULE)
    {
        memset(&stBindInfo, 0x0, sizeof(stSys_BindInfo_T));
#ifndef SSTAR_CHIP_I2
        stBindInfo.eBindType = (MI_SYS_BindType_e)GetIniInt(stIn.curIoKeyString, "BIND_TYPE");
        stBindInfo.u32BindParam = GetIniInt(stIn.curIoKeyString, "BIND_PARAM");
#endif
        stBindInfo.stSrcChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId ;
        stBindInfo.stSrcChnPort.u32DevId = stPreDesc.devId;
        stBindInfo.stSrcChnPort.u32ChnId = stPreDesc.chnId;
        stBindInfo.stSrcChnPort.u32PortId = stIn.stPrev.portId;
        stBindInfo.u32SrcFrmrate = stIn.stPrev.frmRate;
        stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)mapSlotInputInfo[stIn.curPortId].uintDstBindMod;
        stBindInfo.stDstChnPort.u32DevId = mapSlotInputInfo[stIn.curPortId].uintDstBindDev;
        stBindInfo.stDstChnPort.u32ChnId = mapSlotInputInfo[stIn.curPortId].uintDstBindChannel;
        stBindInfo.stDstChnPort.u32PortId = mapSlotInputInfo[stIn.curPortId].uintDstBindPort;
        stBindInfo.u32DstFrmrate = stIn.curFrmRate;

#ifndef SSTAR_CHIP_I2
        if (stModDesc.modId == E_SYS_MOD_VDISP || stModDesc.modId == E_SYS_MOD_DISP)
        {
            stBindInfo.stDstChnPort.u32ChnId = stIn.curPortId;
            stBindInfo.stDstChnPort.u32PortId = 0;
            MI_SYS_BindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate);

        }
        else
        {
            stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
            stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
            MI_SYS_BindChnPort2(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate, stBindInfo.eBindType, stBindInfo.u32BindParam);
        }
#else
        stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
        stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
        MI_SYS_BindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate);
#endif
    }
    else
    {
        printf("Slot not support!\n");
    }
}
void Slot::UnBindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;
    stSys_BindInfo_T stBindInfo;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);
    if (mapSysModuleType[stPreDesc.modId] == E_STREAM_OUT_DATA_IN_KERNEL_MODULE)
    {
        //printf("UnBind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
        //printf("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);
        memset(&stBindInfo, 0x0, sizeof(stSys_BindInfo_T));
        stBindInfo.stSrcChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId ;
        stBindInfo.stSrcChnPort.u32DevId = stPreDesc.devId;
        stBindInfo.stSrcChnPort.u32ChnId = stPreDesc.chnId;
        stBindInfo.stSrcChnPort.u32PortId = stIn.stPrev.portId;
        stBindInfo.u32SrcFrmrate = stIn.stPrev.frmRate;
        stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)mapSlotInputInfo[stIn.curPortId].uintDstBindMod;
        stBindInfo.stDstChnPort.u32DevId = mapSlotInputInfo[stIn.curPortId].uintDstBindDev;
        stBindInfo.stDstChnPort.u32ChnId = mapSlotInputInfo[stIn.curPortId].uintDstBindChannel;
        stBindInfo.stDstChnPort.u32PortId = mapSlotInputInfo[stIn.curPortId].uintDstBindPort;
        stBindInfo.u32DstFrmrate = stIn.curFrmRate;
#ifndef SSTAR_CHIP_I2
         if (stModDesc.modId == E_SYS_MOD_VDISP || stModDesc.modId == E_SYS_MOD_DISP)
         {
             stBindInfo.stDstChnPort.u32ChnId = stIn.curPortId;
             stBindInfo.stDstChnPort.u32PortId = 0;
         }
         else
         {
             stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
             stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
         }
#else
        stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
        stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
#endif
        MI_SYS_UnBindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort);
    }
}
void Slot::Init()
{
}
void Slot::Deinit()
{
}

