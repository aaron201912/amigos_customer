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
#include <mi_sys_datatype.h>
#include <mi_sys.h>
#include <mi_vdisp_datatype.h>
#include <mi_vdisp.h>
#include "vdisp.h"

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

Vdisp::Vdisp()
{
}

Vdisp::~Vdisp()
{
}
void Vdisp::LoadDb()
{
    stVdispInputInfo_t stVdispInputInfo;
    stVdispOutputInfo_t stVdispOutputInfo;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapVdispIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapVdispOut;

    for (itMapVdispIn = mapModInputInfo.begin(); itMapVdispIn != mapModInputInfo.end(); itMapVdispIn++)
    {
        memset(&stVdispInputInfo, 0, sizeof(stVdispInputInfo_t));
#ifdef SSTAR_CHIP_I2
		stVdispInputInfo.intPortId = itMapVdispIn->second.curPortId;
#else
		stVdispInputInfo.intChnId = itMapVdispIn->second.curPortId;
#endif
        stVdispInputInfo.intFreeRun = GetIniInt(itMapVdispIn->second.curIoKeyString, "FREE_RUN");
        stVdispInputInfo.intVdispInX = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_X");
        stVdispInputInfo.intVdispInY = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_Y");
        
        stVdispInputInfo.intVdispInWidth = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_W");
        stVdispInputInfo.intVdispInHeight = GetIniInt(itMapVdispIn->second.curIoKeyString, "VDISP_H");
        vVdispInputInfo.push_back(stVdispInputInfo);
    }
    for (itMapVdispOut = mapModOutputInfo.begin(); itMapVdispOut != mapModOutputInfo.end(); itMapVdispOut++)
    {
        memset(&stVdispOutputInfo, 0, sizeof(stVdispOutputInfo_t));
        stVdispOutputInfo.intPortId = itMapVdispOut->second.curPortId;
        stVdispOutputInfo.intVdispOutFrameRate = itMapVdispOut->second.curFrmRate;
        stVdispOutputInfo.intVdispOutWidth = GetIniInt(itMapVdispOut->second.curIoKeyString, "VID_W");
        stVdispOutputInfo.intVdispOutHeight = GetIniInt(itMapVdispOut->second.curIoKeyString, "VID_H");
        stVdispOutputInfo.intVdispOutPts = GetIniInt(itMapVdispOut->second.curIoKeyString, "PTS");
        stVdispOutputInfo.intVdispOutFormat = GetIniInt(itMapVdispOut->second.curIoKeyString, "VID_FMT");
        stVdispOutputInfo.intVdispOutBkColor = GetIniInt(itMapVdispOut->second.curIoKeyString, "BK_COLOR");
        vVdispOutputInfo.push_back(stVdispOutputInfo);
        itMapVdispOut->second.stStreanInfo.eStreamType = (E_STREAM_TYPE)stVdispOutputInfo.intVdispOutFormat;
        itMapVdispOut->second.stStreanInfo.stFrameInfo.streamWidth = stVdispOutputInfo.intVdispOutWidth;
        itMapVdispOut->second.stStreanInfo.stFrameInfo.streamHeight = stVdispOutputInfo.intVdispOutHeight;
    }
}
void Vdisp::Init()
{
#ifdef SSTAR_CHIP_I2
    MI_VDISP_InputPortAttr_t stInputPortAttr;
#else
    MI_VDISP_InputChnAttr_t stInputChnAttr;
#endif
    MI_VDISP_OutputPortAttr_t stOutputPortAttr;
    std::vector<stVdispInputInfo_t>::iterator itVdispIn;
    std::vector<stVdispOutputInfo_t>::iterator itVdispOut;

    MI_VDISP_Init();
    MI_VDISP_OpenDevice(stModDesc.devId);

    //set input port attr
    for (itVdispIn = vVdispInputInfo.begin(); itVdispIn != vVdispInputInfo.end(); itVdispIn++)
    {
#ifdef SSTAR_CHIP_I2
        if(itVdispIn->intFreeRun == 1)
        {
            stInputPortAttr.s32IsFreeRun = TRUE;
        }
        else
        {
            stInputPortAttr.s32IsFreeRun = FALSE;
        }

        stInputPortAttr.u32OutX = ALIGN16_DOWN(itVdispIn->intVdispInX);
        stInputPortAttr.u32OutY = itVdispIn->intVdispInY;

        stInputPortAttr.u32OutWidth = itVdispIn->intVdispInWidth;
        stInputPortAttr.u32OutHeight = itVdispIn->intVdispInHeight;
        MI_VDISP_SetInputPortAttr(stModDesc.devId, (MI_VDISP_PORT)itVdispIn->intPortId, &stInputPortAttr);
        MI_VDISP_EnableInputPort(stModDesc.devId, (MI_VDISP_PORT)itVdispIn->intPortId);
#else
        if(itVdispIn->intFreeRun == 1)
        {
            stInputChnAttr.s32IsFreeRun = TRUE;
        }
        else
        {
            stInputChnAttr.s32IsFreeRun = FALSE;
        }

        stInputChnAttr.u32OutX = itVdispIn->intVdispInX;
        stInputChnAttr.u32OutY = itVdispIn->intVdispInY;

        stInputChnAttr.u32OutWidth = itVdispIn->intVdispInWidth;
        stInputChnAttr.u32OutHeight = itVdispIn->intVdispInHeight;

        MI_VDISP_SetInputChannelAttr(stModDesc.devId, (MI_VDISP_CHN)itVdispIn->intChnId, &stInputChnAttr);
        MI_VDISP_EnableInputChannel(stModDesc.devId, (MI_VDISP_CHN)itVdispIn->intChnId);
#endif
    }

    for (itVdispOut = vVdispOutputInfo.begin(); itVdispOut != vVdispOutputInfo.end(); itVdispOut++)
    {
        
        memset(&stOutputPortAttr, 0, sizeof(MI_VDISP_OutputPortAttr_t));
        stOutputPortAttr.u32FrmRate = itVdispOut->intVdispOutFrameRate;
        stOutputPortAttr.u32Height = itVdispOut->intVdispOutHeight;
        stOutputPortAttr.u32Width = itVdispOut->intVdispOutWidth;
        stOutputPortAttr.u64pts = itVdispOut->intVdispOutPts;
        stOutputPortAttr.ePixelFormat = (MI_SYS_PixelFormat_e)itVdispOut->intVdispOutFormat;
        switch(itVdispOut->intVdispOutBkColor)
        {
            case 0:
                stOutputPortAttr.u32BgColor = YUYV_BLACK;
                break;
            case 1:
                stOutputPortAttr.u32BgColor = YUYV_WHITE;
                break;
            case 2:
                stOutputPortAttr.u32BgColor = YUYV_RED;
                break;
            case 3:
                stOutputPortAttr.u32BgColor = YUYV_GREEN;
                break;
            case 4:
                stOutputPortAttr.u32BgColor = YUYV_BLUE;
                break;
            default:
                stOutputPortAttr.u32BgColor = YUYV_BLACK;
                break;
        }

        MI_VDISP_SetOutputPortAttr(stModDesc.devId, 0, &stOutputPortAttr);
    }   
    MI_VDISP_StartDev(stModDesc.devId);
}
void Vdisp::PrevIntBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc)
{
    stSys_BindInfo_T stBindInfo;

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
void Vdisp::PrevIntUnBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc)
{
    stSys_BindInfo_T stBindInfo;

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

void Vdisp::Deinit()
{
    std::vector<stVdispInputInfo_t>::iterator itVdispIn;

    MI_VDISP_StopDev(stModDesc.devId);
    for(itVdispIn = vVdispInputInfo.begin(); itVdispIn != vVdispInputInfo.end(); itVdispIn++)
    {
#ifdef SSTAR_CHIP_I2
        MI_VDISP_DisableInputPort(stModDesc.devId, (MI_VDISP_PORT)(itVdispIn->intPortId));
#else
        MI_VDISP_DisableInputChannel(stModDesc.devId, (MI_VDISP_CHN)(itVdispIn->intChnId));
#endif
    }
    MI_VDISP_CloseDevice(stModDesc.devId);
    MI_VDISP_Exit();
}
