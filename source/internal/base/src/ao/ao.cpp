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
#include "mi_ao.h"
#include "ao.h"

Ao::Ao()
{
}
Ao::~Ao()
{
}
void Ao::LoadDb()
{
    intAoDevVolume = GetIniInt(stModDesc.modKeyString, "VOLUME");
}
void Ao::Incoming(stStreamInfo_t *pInfo)
{
    MI_AUDIO_Attr_t stAttr;

    MI_AO_GetPubAttr((MI_AUDIO_DEV)stModDesc.devId, &stAttr);
    switch (stAttr.eBitwidth)
    {
        case E_MI_AUDIO_BIT_WIDTH_16:           
            if (pInfo->stPcmInfo.uintBitLength != 16)
                goto RESET_AO;
            break;
        case E_MI_AUDIO_BIT_WIDTH_24:
            if (pInfo->stPcmInfo.uintBitLength != 24)
                goto RESET_AO;
            break;
        default:
            ASSERT(0);
    }
    if (pInfo->stPcmInfo.uintBitRate != stAttr.eSamplerate)
        goto RESET_AO;

    return;

RESET_AO:
    printf("Ao attr is different neet to reset!\n");
    MI_AO_DisableChn((MI_AUDIO_DEV)stModDesc.devId, (MI_AO_CHN)stModDesc.chnId);
    MI_AO_Disable((MI_AUDIO_DEV)stModDesc.devId);
    switch (pInfo->stPcmInfo.uintBitLength)
    {
        case 16:
            stAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
            break;
        case 24:
            stAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_24;
            break;
        default:
            ASSERT(0);
    }
    stAttr.eSamplerate = (MI_AUDIO_SampleRate_e)pInfo->stPcmInfo.uintBitRate;
    MI_AO_SetPubAttr((MI_AUDIO_DEV)stModDesc.devId, &stAttr);
    MI_AO_Enable((MI_AUDIO_DEV)stModDesc.devId);
    MI_AO_EnableChn((MI_AUDIO_DEV)stModDesc.devId, (MI_AO_CHN)stModDesc.chnId);

}
void Ao::Outcoming()
{
    MI_AO_DisableChn((MI_AUDIO_DEV)stModDesc.devId, (MI_AO_CHN)stModDesc.chnId);
    MI_AO_Disable((MI_AUDIO_DEV)stModDesc.devId);
}
void Ao::Init()
{
    MI_AUDIO_Attr_t stAttr;

    stAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stAttr.WorkModeSetting.stI2sConfig.bSyncClock = TRUE;
    stAttr.WorkModeSetting.stI2sConfig.eFmt = E_MI_AUDIO_I2S_FMT_I2S_MSB;
    stAttr.WorkModeSetting.stI2sConfig.eMclk = E_MI_AUDIO_I2S_MCLK_0;
    stAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_48000;
    stAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
    stAttr.u32CodecChnCnt = 0; // useless
    stAttr.u32FrmNum = 6;  // useless
    stAttr.u32PtNumPerFrm = stAttr.eSamplerate / 32; // for aec
    stAttr.u32ChnCnt = 2;
    MI_AO_SetPubAttr((MI_AUDIO_DEV)stModDesc.devId, &stAttr);
    MI_AO_Enable((MI_AUDIO_DEV)stModDesc.devId);
    MI_AO_EnableChn((MI_AUDIO_DEV)stModDesc.devId, (MI_AO_CHN)stModDesc.chnId);
    MI_AO_SetVolume((MI_AUDIO_DEV)stModDesc.devId, intAoDevVolume);
}
void Ao::Deinit()
{
}

