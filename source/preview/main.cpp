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
#include <iostream>

#include <stdio.h>
#include "sys.h"
#include "rtsp.h"
#include "venc.h"
#include "vpe.h"
#include "vif.h"
#include "divp.h"
#include "empty.h"
#include "ui.h"
#include "iq.h"
#include "file.h"
#include "vdec.h"
#include "disp.h"
#include "vdisp.h"
#include "ai.h"
#include "ao.h"
#include "slot.h"
#include "snr.h"
#include "uac.h"
#include "uvc.h"
#include "inject.h"

void Sys::Implement(std::string &strKey)
{
    unsigned int intId = 0;

    //printf("Connect key str %s\n", strKey.c_str());
    intId = Sys::FindBlockId(strKey);
    if (intId == (unsigned int)-1)
    {
        AMIGOS_ERR("Can't find key str %s\n", strKey.c_str());
        return;
    }
    if (!Sys::FindBlock(strKey))
    {
        switch (intId)
        {
#if INTERFACE_VDEC
            case E_SYS_MOD_VDEC:
            {
                SysChild<Vdec> Vdec(strKey);
            }
            break;
#endif
#if INTERFACE_DISP
            case E_SYS_MOD_DISP:
            {
                SysChild<Disp> Disp(strKey);
            }
            break;
#endif
            case E_SYS_MOD_RTSP:
            {
                SysChild<Rtsp> Rtsp(strKey);
            }
            break;
#if INTERFACE_VENC
            case E_SYS_MOD_VENC:
            {
                SysChild<Venc> Venc(strKey);
            }
            break;
#endif
#if INTERFACE_VPE
            case E_SYS_MOD_VPE:
            {
                SysChild<Vpe> Vpe(strKey);
            }
            break;
#endif
#if INTERFACE_VIF
            case E_SYS_MOD_VIF:
            {
                SysChild<Vif> Vif(strKey);
            }
            break;
#endif
#if INTERFACE_DIVP
            case E_SYS_MOD_DIVP:
            {
                SysChild<Divp> Divp(strKey);
            }
            break;
#endif
            case E_SYS_MOD_EMPTY:
            {
                SysChild<Empty> Empty(strKey);
            }
            break;
#if INTERFACE_RGN
            case E_SYS_MOD_UI:
            {
                SysChild<Ui> Ui(strKey);
            }
            break;
#endif
#if INTERFACE_IQSERVER
            case E_SYS_MOD_IQ:
            {
                SysChild<Iq> Iq(strKey);
            }
            break;
#endif
            case E_SYS_MOD_FILE:
            {
                SysChild<File> File(strKey);
            }
            break;
#if INTERFACE_VDISP
            case E_SYS_MOD_VDISP:
            {
                SysChild<Vdisp> Vdisp(strKey);
            }
            break;
#endif
#if INTERFACE_AI
            case E_SYS_MOD_AI:
            {
                SysChild<Ai> Ai(strKey);
            }
                break;
#endif
#if INTERFACE_AO
            case E_SYS_MOD_AO:
            {
                SysChild<Ao> Ao(strKey);
            }
            break;
#endif
            case E_SYS_MOD_SLOT:
            {
                SysChild<Slot> Slot(strKey);
            }
            break;
#if INTERFACE_SENSOR
            case E_SYS_MOD_SNR:
            {
                SysChild<Snr> Snr(strKey);
            }
            break;
#endif
            case E_SYS_MOD_UAC:
            {
                SysChild<Uac> Uac(strKey);
            }
            break;
            case E_SYS_MOD_UVC:
            {
                SysChild<Uvc> Uvc(strKey);
            }
            break;
            case E_SYS_MOD_INJECT:
            {
                SysChild<Inject> Inject(strKey);
            }
            break;
            default:
                return;
        }
        GetInstance(strKey)->BuildModTree();
    }

    return;
}
static void ResChange()
{
    unsigned int uintInport = 0;
    unsigned int uintResChoice = 0;
    char strBlockName[32];
    char strGet[32];
    Sys *pClass = NULL;
    stModDesc_t stModDesc;
    stStreamInfo_t stStreamInfo;
    struct astTempSize_s
    {
        unsigned int uintWidth;
        unsigned int uintHeight;
    } astTempSize[] = {{352, 288}, {640, 360}, {640, 480}, {720, 480}, {720, 576}, {960, 540}, {1280, 720}, {960, 1080},
                       {1600, 1200}, {1920, 1080}, {2048, 1536}, {2592, 1944}, {3072, 2048}, {3840, 2160}};

    while (1)
    {
        std::string strBlock;
        memset(strBlockName, 0, 32);
        memset(strGet, 0, 32);
        std::cout << ">====================Resolution change main menu ====================<" << std::endl;
        std::cout << "Please type block name and input port (name:x)." << std::endl;
        std::cout << "Type 'q' to exit resolution change memu." << std::endl;
        std::cout << ">====================Resolution change main menu ====================<" << std::endl;
        fflush(stdin);
        scanf("%s", strGet);
        if (strncmp("p", strGet, 1) == 0)
        {
            continue;
        }
        else if (!strncmp("q", strGet, 1))
        {
            break;
        }
        sscanf(strGet, "%[^:] : %d", strBlockName, &uintInport);
        strBlock = strBlockName;
        pClass = Sys::GetInstance(strBlock);
        if (!pClass)
        {
            std::cout << "Not found " << strBlockName << std::endl;
            continue;
        }
        while (1)
        {
            pClass->GetInputStreamInfo(uintInport, &stStreamInfo);
            std::cout << ">====================Resolution change sub menu ====================<" << std::endl;
            pClass->GetModDesc(stModDesc);
            std::cout << "Current mod is " << stModDesc.modKeyString << std::endl;
            std::cout << "Mod id is " << stModDesc.modId << std::endl;
            std::cout << "Channel id is " << stModDesc.chnId << std::endl;
            std::cout << "Device id is " << stModDesc.devId << std::endl;
            for (unsigned int i = 0; i < sizeof(astTempSize) / sizeof(struct astTempSize_s); i++)
            {
                std::cout << "Type " << i << " to change resolution to " << astTempSize[i].uintWidth << 'x' << astTempSize[i].uintHeight     \
                    << ((stStreamInfo.stFrameInfo.streamWidth == astTempSize[i].uintWidth && stStreamInfo.stFrameInfo.streamHeight == astTempSize[i].uintHeight) ? (".<===") : ".") << std::endl;
            }
            std::cout << "Type 'q' to exit resolution change memu." << std::endl;
            std::cout << ">====================Resolution change sub menu ====================<" << std::endl;
            fflush(stdin);
            scanf("%s", strGet);
            if (strncmp("p", strGet, 1) == 0)
            {
                continue;
            }
            else if (!strncmp("q", strGet, 1))
            {
                break;
            }
            uintResChoice = atoi(strGet);
            if (uintResChoice < sizeof(astTempSize) / sizeof(struct astTempSize_s))
            {
                stStreamInfo.stFrameInfo.streamWidth = astTempSize[uintResChoice].uintWidth;
                stStreamInfo.stFrameInfo.streamHeight = astTempSize[uintResChoice].uintHeight;
                pClass->UpdateInputStreamInfo(uintInport, &stStreamInfo);
            }
        }
    }
}
void SelectIni(std::string &strIni, std::map<std::string, unsigned int> &mapModId)
{
    Sys::InitSys(strIni, mapModId);
}
int main(int argc, char **argv)
{
    std::map<std::string, unsigned int> mapModId;
    std::map<std::string, Sys *> maskMap;
    std::vector<Sys *> objVect;
    std::vector<std::string> vectIniFiles;
    unsigned int i = 0, sel = 0, isInit = 0;
    char idx[8];

    if (argc < 2)
    {
        printf("Usage: ./%s xxx_ini_1 xxx_ini_2\n", argv[0]);

        return -1;
    }
    mapModId["RTSP"] = E_SYS_MOD_RTSP;
    mapModId["VENC"] = E_SYS_MOD_VENC;
    mapModId["VPE"] = E_SYS_MOD_VPE;
    mapModId["DIVP"] = E_SYS_MOD_DIVP;
    mapModId["DISP"] = E_SYS_MOD_DISP;
    mapModId["VDEC"] = E_SYS_MOD_VDEC;
    mapModId["VIF"] = E_SYS_MOD_VIF;
    mapModId["VDISP"] = E_SYS_MOD_VDISP;
    mapModId["LDC"] = E_SYS_MOD_LDC;
    mapModId["EMPTY"] = E_SYS_MOD_EMPTY;
    mapModId["UI"] = E_SYS_MOD_UI;
    mapModId["IQ"] = E_SYS_MOD_IQ;
    mapModId["FILE"] = E_SYS_MOD_FILE;
    mapModId["AI"] = E_SYS_MOD_AI;
    mapModId["AO"] = E_SYS_MOD_AO;
    mapModId["SLOT"] = E_SYS_MOD_SLOT;
#ifndef SSTAR_CHIP_I2
    mapModId["SNR"] = E_SYS_MOD_SNR;
#endif
    mapModId["INJECT"] = E_SYS_MOD_INJECT;

    for (i = 1; i < (unsigned int)argc; i++)
    {
        vectIniFiles.push_back(argv[i]);
    }
    do
    {
        memset(idx, 0, 8);
        for (i = 0; i < vectIniFiles.size(); i++)
        {
            printf("Press '%d' to run: %s\n", i, vectIniFiles[i].c_str());
        }
        printf("Press 'm' to enter resolution change menu!\n");
        printf("Press 'q' to exit!\n");
        fflush(stdin);
        scanf("%4s", idx);
        if (strncmp("q", idx, 1) == 0)
        {           
            if (isInit)
            {
                Sys::DeinitSys();
            }
            break;
        }
        else if (strncmp("p", idx, 1) == 0)
        {
            continue;
        }
        else if (strncmp("m", idx, 1) == 0)
        {
            ResChange();
            continue;
        }
        sel = atoi(idx);
        if ( sel < vectIniFiles.size())
        {
            
            if (isInit == 1)
            {
                Sys::DeinitSys();
            }
            else
            {
                isInit = 1;
            }
            Sys::InitSys(vectIniFiles[sel], mapModId);
        }
        else
        {
            printf("Input select %s error!\n", idx);
        }
    }while (1);

    return 0;
}
