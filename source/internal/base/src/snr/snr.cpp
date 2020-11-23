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

#include "snr.h"
#include <stdio.h>
#include "mi_common.h"
#include "mi_sensor.h"

Snr::Snr()
{
}
Snr::~Snr()
{
}
void Snr::LoadDb()
{
    stSnrInfo.intHdrType = GetIniInt(stModDesc.modKeyString,"HDR_TYPE");
    stSnrInfo.intSensorId = GetIniInt(stModDesc.modKeyString,"SNR_ID");
    stSnrInfo.intSensorRes = GetIniInt(stModDesc.modKeyString,"SNR_RES");
}
void Snr::Init()
{
    MI_U32 u32ResCount =0;
    MI_U8 u8ResIndex =0;
    MI_SNR_Res_t stRes;

    memset(&stRes, 0x0, sizeof(MI_SNR_Res_t));

    if(stSnrInfo.intHdrType > 0)
        MI_SNR_SetPlaneMode((MI_SNR_PAD_ID_e)stSnrInfo.intSensorId, TRUE);
    else
        MI_SNR_SetPlaneMode((MI_SNR_PAD_ID_e)stSnrInfo.intSensorId, FALSE);
    
    MI_SNR_QueryResCount((MI_SNR_PAD_ID_e)stSnrInfo.intSensorId, &u32ResCount);
    for(u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
    {
        MI_SNR_GetRes((MI_SNR_PAD_ID_e)stSnrInfo.intSensorId, u8ResIndex, &stRes);
        AMIGOS_INFO("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
        u8ResIndex,
        stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width,stRes.stCropRect.u16Height,
        stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height,
        stRes.u32MaxFps,stRes.u32MinFps,
        stRes.strResDesc);
    }
    if(stSnrInfo.intSensorRes >= (int)u32ResCount)
    {
        AMIGOS_ERR("choice err res %d > =cnt %d\n", stSnrInfo.intSensorRes, u32ResCount);
        assert(0);
    }
    AMIGOS_INFO("You choose sensor res is %d\n", stSnrInfo.intSensorRes);
    MI_SNR_SetRes((MI_SNR_PAD_ID_e)stSnrInfo.intSensorId, (MI_U32)stSnrInfo.intSensorRes);
    MI_SNR_Enable((MI_SNR_PAD_ID_e)stSnrInfo.intSensorId);
}
void Snr::Deinit()
{
    MI_SNR_Disable((MI_SNR_PAD_ID_e)stSnrInfo.intSensorId);
}

