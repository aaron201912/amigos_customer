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

#include <vector>
#include <string>

#include "mi_sys.h"
#ifdef INTERFACE_PANEL
#include "mi_panel.h"
#endif
#ifdef INTERFACE_HDMI
#include "mi_hdmi.h"
#endif
#include "mi_disp.h"

#ifdef INTERFACE_PANEL
#include "SAT070CP50_1024x600.h"
#endif

#include "disp.h"

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

Disp::Disp()
{
}
Disp::~Disp()
{
}
void Disp::LoadDb()
{
    std::map<unsigned int, stModInputInfo_t>::iterator itMapIn;
    std::string strInputCnt;
    int intLayerCnt;
    int intLayerId;
    int i = 0;
    char strLayerName[30];
    std::string strLayerKey;

    stDispInfo.intDeviceType = GetIniInt(stModDesc.modKeyString, "DEV_TYPE");
    stDispInfo.intBackGroundColor = GetIniInt(stModDesc.modKeyString, "BK_COLOR");
    stDispInfo.intPanelLinkType = GetIniInt(stModDesc.modKeyString, "PNL_LINK_TYPE");
    stDispInfo.intOutTiming = GetIniInt(stModDesc.modKeyString, "DISP_OUT_TIMING", E_MI_DISP_OUTPUT_1080P60);
    intLayerCnt = GetIniInt(stModDesc.modKeyString, "IN_LAYER_CNT");
    AMIGOS_INFO("DEV_TYPE : %d\n", stDispInfo.intDeviceType);
    AMIGOS_INFO("BK_COLOR : %d\n", stDispInfo.intBackGroundColor);
    AMIGOS_INFO("PNL_LINK_TYPE : %d\n", stDispInfo.intPanelLinkType);
    AMIGOS_INFO("LAYER_CNT : %d\n", intLayerCnt);
    if (intLayerCnt != -1)
    {
        for (i = 0; i < intLayerCnt; i++)
        {
            snprintf(strLayerName,30, "IN_LAYER_%d", i);
            strLayerKey = GetIniString(stModDesc.modKeyString, strLayerName);
            AMIGOS_INFO("layer key %s\n", strLayerKey.c_str());
            mapLayerInfo[i].uintId = GetIniUnsignedInt(strLayerKey, "LAYER_ID", 0);
            mapLayerInfo[i].uintRot = GetIniUnsignedInt(strLayerKey, "LAYER_ROT");
            mapLayerInfo[i].uintWidth = GetIniUnsignedInt(strLayerKey, "LAYER_WIDTH");
            mapLayerInfo[i].uintHeight = GetIniUnsignedInt(strLayerKey, "LAYER_HEIGHT");
            mapLayerInfo[i].uintDispWidth = GetIniUnsignedInt(strLayerKey, "LAYER_DISP_WIDTH");
            mapLayerInfo[i].uintDispHeight = GetIniUnsignedInt(strLayerKey, "LAYER_DISP_HEIGHT");
            mapLayerInfo[i].uintDispXpos = GetIniUnsignedInt(strLayerKey, "LAYER_DISP_XPOS");
            mapLayerInfo[i].uintDispYpos = GetIniUnsignedInt(strLayerKey, "LAYER_DISP_YPOS");
            AMIGOS_INFO("Layer %d info : LAYER_ROT %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintRot);
            AMIGOS_INFO("Layer %d info : LAYER_WIDTH %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintWidth);
            AMIGOS_INFO("Layer %d info : LAYER_HEIGHT %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintHeight);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_WIDTH %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintDispWidth);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_HEIGHT %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintDispHeight);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_XPOS %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintDispXpos);
            AMIGOS_INFO("Layer %d info : LAYER_DISP_YPOS %d\n", mapLayerInfo[i].uintId, mapLayerInfo[i].uintDispYpos);
        }
    }
    for (itMapIn = mapModInputInfo.begin(); itMapIn != mapModInputInfo.end(); ++itMapIn)
    {
        intLayerId = GetIniInt(itMapIn->second.curIoKeyString, "IN_LAYER_ID");
        if (intLayerId != -1)
        {
            if (mapLayerInfo.find(intLayerId) != mapLayerInfo.end())
            {
                mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintSrcWidth = GetIniUnsignedInt(itMapIn->second.curIoKeyString, "SRC_WIDTH");
                mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintSrcHeight = GetIniUnsignedInt(itMapIn->second.curIoKeyString, "SRC_HEIGHT");
                mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstWidth = GetIniUnsignedInt(itMapIn->second.curIoKeyString, "DST_WIDTH");
                mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstHeight = GetIniUnsignedInt(itMapIn->second.curIoKeyString, "DST_HEIGHT");
                mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstXpos = GetIniUnsignedInt(itMapIn->second.curIoKeyString, "DST_XPOS");
                mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstYpos = GetIniUnsignedInt(itMapIn->second.curIoKeyString, "DST_YPOS");
                AMIGOS_INFO("In Layer %d port %d info : SRC_WIDTH %d\n", intLayerId, itMapIn->second.curPortId, mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintSrcWidth);
                AMIGOS_INFO("In Layer %d port %d info : SRC_HEIGHT %d\n", intLayerId, itMapIn->second.curPortId, mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintSrcHeight);
                AMIGOS_INFO("In Layer %d port %d info : DST_WIDTH %d\n", intLayerId, itMapIn->second.curPortId, mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstWidth);
                AMIGOS_INFO("In Layer %d port %d info : DST_HEIGHT %d\n", intLayerId, itMapIn->second.curPortId, mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstHeight);
                AMIGOS_INFO("In Layer %d port %d info : DST_XPOS %d\n", intLayerId, itMapIn->second.curPortId, mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstXpos);
                AMIGOS_INFO("In Layer %d port %d info : DST_YPOS %d\n", intLayerId, itMapIn->second.curPortId, mapLayerInfo[intLayerId].mapLayerInputPortInfo[itMapIn->second.curPortId].uintDstYpos);
            }
            else
            {
                AMIGOS_ERR("Layer did not create, pls check your config.\n");
            }
        }
    }
}
#if INTERFACE_HDMI
static int HdmiCb(MI_HDMI_DeviceId_e eHdmi, MI_HDMI_EventType_e Event, void *pEventParam, void *pUsrParam)
{
    switch (Event)
    {
        case E_MI_HDMI_EVENT_HOTPLUG:
            AMIGOS_INFO("E_MI_HDMI_EVENT_HOTPLUG.\n");
            break;
        case E_MI_HDMI_EVENT_NO_PLUG:
            AMIGOS_INFO("E_MI_HDMI_EVENT_NO_PLUG.\n");
            break;
        default:
            AMIGOS_ERR("Unsupport event.\n");
            break;
    }

    return MI_SUCCESS;
}
#endif

void Disp::Init()
{
    MI_U8 u8LayerId = 0;
    MI_DISP_PubAttr_t stPubAttr;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    MI_DISP_InputPortAttr_t stInputPortAttr;
#ifndef SSTAR_CHIP_I2
    MI_DISP_RotateConfig_t stRotateConfig;
#endif
    std::map<unsigned int, stDispLayerInfo_t>::iterator itMapLayerInfo;
    std::map<unsigned int, stDispLayerInputPortInfo_t>::iterator itMapLayerInportInfo;
    //pub attr
    memset(&stPubAttr, 0, sizeof(MI_DISP_PubAttr_t));
    memset(&stLayerAttr, 0, sizeof(MI_DISP_VideoLayerAttr_t));

    switch(stDispInfo.intBackGroundColor)
    {
        case 0:
            stPubAttr.u32BgColor = YUYV_BLACK;
            break;
        case 1:
            stPubAttr.u32BgColor = YUYV_WHITE;
            break;
        case 2:
            stPubAttr.u32BgColor = YUYV_RED;
            break;
        case 3:
            stPubAttr.u32BgColor = YUYV_GREEN;
            break;
        case 4:
            stPubAttr.u32BgColor = YUYV_BLUE;
            break;
        default:
            stPubAttr.u32BgColor = YUYV_BLACK;
            break;
    }

    if (stDispInfo.intDeviceType == 0)
    {
#ifdef INTERFACE_PANEL
        MI_PANEL_Init(stPanelParam.eLinkType);
        MI_PANEL_SetPanelParam(&stPanelParam);
        if(stPanelParam.eLinkType == E_MI_PNL_LINK_MIPI_DSI)
        {
            MI_PANEL_SetMipiDsiConfig(&stMipiDsiConfig);
        }
        stPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
        stPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
        stPubAttr.stSyncInfo.u16Vact = stPanelParam.u16Height;
        stPubAttr.stSyncInfo.u16Vbb = stPanelParam.u16VSyncBackPorch;
        stPubAttr.stSyncInfo.u16Vfb = stPanelParam.u16VTotal - (stPanelParam.u16VSyncWidth + stPanelParam.u16Height + stPanelParam.u16VSyncBackPorch);
        stPubAttr.stSyncInfo.u16Hact = stPanelParam.u16Width;
        stPubAttr.stSyncInfo.u16Hbb = stPanelParam.u16HSyncBackPorch;
        stPubAttr.stSyncInfo.u16Hfb = stPanelParam.u16HTotal - (stPanelParam.u16HSyncWidth + stPanelParam.u16Width + stPanelParam.u16HSyncBackPorch);
        stPubAttr.stSyncInfo.u16Bvact = 0;
        stPubAttr.stSyncInfo.u16Bvbb = 0;
        stPubAttr.stSyncInfo.u16Bvfb = 0;
        stPubAttr.stSyncInfo.u16Hpw = stPanelParam.u16HSyncWidth;
        stPubAttr.stSyncInfo.u16Vpw = stPanelParam.u16VSyncWidth;
        stPubAttr.stSyncInfo.u32FrameRate = stPanelParam.u16DCLK*1000000/(stPanelParam.u16HTotal*stPanelParam.u16VTotal);
#endif
    }
    else if (stDispInfo.intDeviceType == 1)
    {
#ifdef INTERFACE_HDMI
        MI_HDMI_InitParam_t stInitParam;
        MI_HDMI_Attr_t stAttr;

        stInitParam.pCallBackArgs = NULL;
        stInitParam.pfnHdmiEventCallback = HdmiCb;
        MI_HDMI_Init(&stInitParam);
        MI_HDMI_Open(E_MI_HDMI_ID_0);

        memset(&stAttr, 0, sizeof(MI_HDMI_Attr_t));
        stAttr.stEnInfoFrame.bEnableAudInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableAviInfoFrame  = FALSE;
        stAttr.stEnInfoFrame.bEnableSpdInfoFrame  = FALSE;
        stAttr.stAudioAttr.bEnableAudio = TRUE;
        stAttr.stAudioAttr.bIsMultiChannel = 0;
        stAttr.stAudioAttr.eBitDepth = E_MI_HDMI_BIT_DEPTH_16;
        stAttr.stAudioAttr.eCodeType = E_MI_HDMI_ACODE_PCM;
        stAttr.stAudioAttr.eSampleRate = E_MI_HDMI_AUDIO_SAMPLERATE_48K;
        stAttr.stVideoAttr.bEnableVideo = TRUE;
        stAttr.stVideoAttr.eColorType = E_MI_HDMI_COLOR_TYPE_RGB444;//default color type
        stAttr.stVideoAttr.eDeepColorMode = E_MI_HDMI_DEEP_COLOR_MAX;
        switch ((MI_DISP_OutputTiming_e)stDispInfo.intOutTiming)
        {
            case E_MI_DISP_OUTPUT_NTSC:
            case E_MI_DISP_OUTPUT_480P60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_480_60P;
                break;
            case E_MI_DISP_OUTPUT_PAL:
            case E_MI_DISP_OUTPUT_576P50:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_576_50P;
                break;
            case E_MI_DISP_OUTPUT_720P50:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_720_50P;
                break;
            case E_MI_DISP_OUTPUT_720P60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_720_60P;
                break;
            case E_MI_DISP_OUTPUT_1080P60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
                break;
            case E_MI_DISP_OUTPUT_3840x2160_30:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_30P;
                break;
            case E_MI_DISP_OUTPUT_3840x2160_60:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_4K2K_60P;
                break;
            default:
                stAttr.stVideoAttr.eTimingType = E_MI_HDMI_TIMING_1080_60P;
                break;
        }
        stAttr.stVideoAttr.eOutputMode = E_MI_HDMI_OUTPUT_MODE_HDMI;
        MI_HDMI_SetAttr(E_MI_HDMI_ID_0, &stAttr);
        MI_HDMI_Start(E_MI_HDMI_ID_0);
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
        stPubAttr.eIntfSync = (MI_DISP_OutputTiming_e)stDispInfo.intOutTiming;
#endif
    }
    else if (stDispInfo.intDeviceType == 2)
    {
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
        stPubAttr.eIntfSync = (MI_DISP_OutputTiming_e)stDispInfo.intOutTiming;
    }
    else if (stDispInfo.intDeviceType == 3)
    {
        stPubAttr.eIntfType = E_MI_DISP_INTF_CVBS;
        stPubAttr.eIntfSync = (MI_DISP_OutputTiming_e)stDispInfo.intOutTiming;
    }
    else
    {
        AMIGOS_ERR("Not support current device type!\n");

        return;
    }

    //set disp pub
    MI_DISP_SetPubAttr((MI_DISP_DEV)stModDesc.devId,  &stPubAttr);
    MI_DISP_Enable((MI_DISP_DEV)stModDesc.devId);

    //set inputport
    for (itMapLayerInfo = mapLayerInfo.begin(); itMapLayerInfo != mapLayerInfo.end(); ++itMapLayerInfo)
    {
        u8LayerId = itMapLayerInfo->second.uintId;
        //set layer
        MI_DISP_BindVideoLayer((MI_DISP_LAYER)u8LayerId, (MI_DISP_DEV)stModDesc.devId);
        stLayerAttr.ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_MST_420;
        stLayerAttr.stVidLayerSize.u16Width = itMapLayerInfo->second.uintWidth;
        stLayerAttr.stVidLayerSize.u16Height = itMapLayerInfo->second.uintHeight;
        stLayerAttr.stVidLayerDispWin.u16Width = itMapLayerInfo->second.uintDispWidth;
        stLayerAttr.stVidLayerDispWin.u16Height = itMapLayerInfo->second.uintDispHeight;
        stLayerAttr.stVidLayerDispWin.u16X = itMapLayerInfo->second.uintDispXpos;
        stLayerAttr.stVidLayerDispWin.u16Y = itMapLayerInfo->second.uintDispYpos;
        MI_DISP_SetVideoLayerAttr((MI_DISP_LAYER)u8LayerId, &stLayerAttr);
#ifndef SSTAR_CHIP_I2
        //rotate
        stRotateConfig.eRotateMode = (MI_DISP_RotateMode_e)itMapLayerInfo->second.uintRot;
        MI_DISP_SetVideoLayerRotateMode((MI_DISP_LAYER)u8LayerId, &stRotateConfig);
#endif
        MI_DISP_EnableVideoLayer((MI_DISP_LAYER)u8LayerId);
        for (itMapLayerInportInfo = itMapLayerInfo->second.mapLayerInputPortInfo.begin(); itMapLayerInportInfo != itMapLayerInfo->second.mapLayerInputPortInfo.end(); ++itMapLayerInportInfo)
        {
            memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
#ifndef SSTAR_CHIP_I2
            stInputPortAttr.u16SrcWidth = itMapLayerInportInfo->second.uintSrcWidth;
            stInputPortAttr.u16SrcHeight = itMapLayerInportInfo->second.uintSrcHeight;
#endif
            stInputPortAttr.stDispWin.u16X = itMapLayerInportInfo->second.uintDstXpos;
            stInputPortAttr.stDispWin.u16Y = itMapLayerInportInfo->second.uintDstYpos;
            stInputPortAttr.stDispWin.u16Width = itMapLayerInportInfo->second.uintDstWidth;
            stInputPortAttr.stDispWin.u16Height = itMapLayerInportInfo->second.uintDstHeight;
            MI_DISP_SetInputPortAttr((MI_DISP_LAYER)u8LayerId, (MI_DISP_INPUTPORT)itMapLayerInportInfo->first, &stInputPortAttr);
            //enable inputport
            MI_DISP_EnableInputPort((MI_DISP_LAYER)u8LayerId, (MI_DISP_INPUTPORT)itMapLayerInportInfo->first);
            MI_DISP_SetInputPortSyncMode((MI_DISP_LAYER)u8LayerId, (MI_DISP_INPUTPORT)itMapLayerInportInfo->first, E_MI_DISP_SYNC_MODE_FREE_RUN);
        }
    }
}
void Disp::PrevIntBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc)
{
    stSys_BindInfo_T stBindInfo;

    if (stPreDesc.modId == E_SYS_MOD_DISP)
    {
        MI_DISP_DeviceAttach(stPreDesc.devId, stModDesc.devId);
    }
    else
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
        stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)stModDesc.modId;
        stBindInfo.stDstChnPort.u32DevId = stModDesc.devId;
        stBindInfo.u32DstFrmrate = stIn.curFrmRate;
#ifndef SSTAR_CHIP_I2
        stBindInfo.stDstChnPort.u32ChnId = stIn.curPortId;
        stBindInfo.stDstChnPort.u32PortId = 0;
        MI_SYS_BindChnPort2(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate, stBindInfo.eBindType, stBindInfo.u32BindParam);
#else
		stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
        stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
        MI_SYS_BindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort, stBindInfo.u32SrcFrmrate, stBindInfo.u32DstFrmrate);
#endif
    }
}
void Disp::PrevIntUnBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc)
{
    stSys_BindInfo_T stBindInfo;

    if (stPreDesc.modId == E_SYS_MOD_DISP)
    {
        MI_DISP_DeviceDetach(stPreDesc.devId, stModDesc.devId);
    }
    else
    {
        memset(&stBindInfo, 0x0, sizeof(stSys_BindInfo_T));
#ifndef SSTAR_CHIP_I2
        stBindInfo.stSrcChnPort.eModId = (MI_ModuleId_e)stPreDesc.modId ;
        stBindInfo.stSrcChnPort.u32DevId = stPreDesc.devId;
#endif
		stBindInfo.stSrcChnPort.u32ChnId = stPreDesc.chnId;
        stBindInfo.stSrcChnPort.u32PortId = stIn.stPrev.portId;
        stBindInfo.u32SrcFrmrate = stIn.stPrev.frmRate;
        stBindInfo.stDstChnPort.eModId = (MI_ModuleId_e)stModDesc.modId;
        stBindInfo.stDstChnPort.u32DevId = stModDesc.devId;
        stBindInfo.u32DstFrmrate = stIn.curFrmRate;
#ifndef SSTAR_CHIP_I2
        stBindInfo.stDstChnPort.u32ChnId = stIn.curPortId;
        stBindInfo.stDstChnPort.u32PortId = 0;
#else
		stBindInfo.stDstChnPort.u32ChnId = stModDesc.chnId;
        stBindInfo.stDstChnPort.u32PortId = stIn.curPortId;
#endif
        MI_SYS_UnBindChnPort(&stBindInfo.stSrcChnPort, &stBindInfo.stDstChnPort);
    }
}
void Disp::Deinit()
{
    MI_U8 u8LayerId = 0;
    std::map<unsigned int, stDispLayerInfo_t>::iterator itMapLayerInfo;
    std::map<unsigned int, stDispLayerInputPortInfo_t>::iterator itMapLayerInportInfo;

    for (itMapLayerInfo = mapLayerInfo.begin(); itMapLayerInfo != mapLayerInfo.end(); ++itMapLayerInfo)
    {
        u8LayerId = itMapLayerInfo->second.uintId;
        for (itMapLayerInportInfo = itMapLayerInfo->second.mapLayerInputPortInfo.begin(); itMapLayerInportInfo != itMapLayerInfo->second.mapLayerInputPortInfo.end(); ++itMapLayerInportInfo)
        {
            MI_DISP_DisableInputPort(u8LayerId, (MI_DISP_INPUTPORT)itMapLayerInportInfo->first);
        }
        MI_DISP_DisableVideoLayer(u8LayerId);
        MI_DISP_UnBindVideoLayer(u8LayerId, (MI_DISP_DEV)stModDesc.devId);
    }
    MI_DISP_Disable((MI_DISP_DEV)stModDesc.devId);
    if (stDispInfo.intDeviceType == 0)
    {
#ifdef INTERFACE_PANEL

       MI_PANEL_DeInit();
#endif
    }
    else if (stDispInfo.intDeviceType == 1)
    {
#ifdef INTERFACE_HDMI
        MI_HDMI_Stop(E_MI_HDMI_ID_0);
        MI_HDMI_Close(E_MI_HDMI_ID_0);
        MI_HDMI_DeInit();
#endif
    }
    else if (stDispInfo.intDeviceType == 2)
    {
    }
    else
    {
        AMIGOS_ERR("Not support current device type!\n");

        return;
    }
}
