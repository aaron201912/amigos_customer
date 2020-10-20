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
#include "sys.h"
#include "rtsp.h"
#include "venc.h"
#include "vpe.h"
#include "vif.h"
#include "divp.h"
#include "dla.h"
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

void Sys::Implement(std::string &strKey)
{
    unsigned int intId = 0;

    //printf("Connect key str %s\n", strKey.c_str());
    intId = Sys::FindBlockId(strKey);
    if (intId == (unsigned int)-1)
    {
        printf("Can't find key str %s\n", strKey.c_str());
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
            case E_SYS_MOD_DLA:
            {
                SysChild<Dla> Dla(strKey);
            }
            break;
#if INTERFACE_RGN
            case E_SYS_MOD_UI:
            {
                SysChild<Ui> Fdfr(strKey);
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
            default:
                return;
        }
        GetInstance(strKey)->BuildModTree();
    }

    return;
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
    mapModId["DLA"] = E_SYS_MOD_DLA;
    mapModId["UI"] = E_SYS_MOD_UI;
    mapModId["IQ"] = E_SYS_MOD_IQ;
    mapModId["FILE"] = E_SYS_MOD_FILE;
    mapModId["AI"] = E_SYS_MOD_AI;
    mapModId["AO"] = E_SYS_MOD_AO;
    mapModId["SLOT"] = E_SYS_MOD_SLOT;
    mapModId["SNR"] = E_SYS_MOD_SNR;
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
        if (strncmp("p", idx, 1) == 0)
        {
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
