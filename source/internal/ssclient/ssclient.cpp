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

#include "sys.h"
#include "slot.h"
#include "rtsp.h"
#include "file.h"
#if INTERFACE_VDEC
#include "vdec.h"
#endif
#if INTERFACE_DIVP
#include "divp.h"
#endif
#if INTERFACE_AO
#include "ao.h"
#endif
#if INTERFACE_AO
#include "disp.h"
#endif
#ifdef INTERFACE_VDISP
#include "vdisp.h"
#endif
#include "ssclient.h"

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
#ifdef INTERFACE_VDEC
            case E_SYS_MOD_VDEC:
            {
                SysChild<Vdec> Vdec(strKey);
            }
            break;
#endif
            case E_SYS_MOD_RTSP:
            {
                SysChild<Rtsp> Rtsp(strKey);
            }
            break;
#ifdef INTERFACE_DIVP
            case E_SYS_MOD_DIVP:
            {
                SysChild<Divp> Divp(strKey);
            }
            break;
#endif
#ifdef INTERFACE_VDISP
            case E_SYS_MOD_VDISP:
            {
                SysChild<Vdisp> Vdisp(strKey);
            }
            break;
#endif
            case E_SYS_MOD_FILE:
            {
                SysChild<File> File(strKey);
            }
            break;
            case E_SYS_MOD_SLOT:
            {
                SysChild<Slot> Slot(strKey);
            }
            break;
#ifdef INTERFACE_DISP
            case E_SYS_MOD_DISP:
            {
                SysChild<Disp> Disp(strKey);
            }
            break;
#endif
#ifdef INTERFACE_AO
            case E_SYS_MOD_AO:
            {
                SysChild<Ao> Ao(strKey);
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

SsClient::SsClient(const char *liv555Url, const char *configIni, unsigned int width, unsigned int height)
{
    std::map<std::string, unsigned int> mapModId;
    std::string objName;
    Rtsp *RtspObj;
    Vdec *VdecObj;
    std::map<std::string, stRtspInputInfo_t> info;
    unsigned char isOpenOnvif;
    std::map<unsigned int, stRtspOutConfig_t> outCfg;
    std::map<unsigned int, stRtspOutConfig_t>::iterator itOutCfg;
    stVdecInfo_t VdecInfo;
    std::vector<stDecOutInfo_t> vectVdecOut;
    std::vector<stDecOutInfo_t>::iterator itVdecOut;

#ifdef INTERFACE_VDEC
    mapModId["VDEC"] = E_SYS_MOD_VDEC;
#endif
#ifdef INTERFACE_DIVP
    mapModId["DIVP"] = E_SYS_MOD_DIVP;
#endif
#ifdef INTERFACE_AO
    mapModId["AO"] = E_SYS_MOD_AO;
#endif
#ifdef INTERFACE_DISP
    mapModId["DISP"] = E_SYS_MOD_DISP;
#endif
#ifdef INTERFACE_VDISP
    mapModId["VDISP"] = E_SYS_MOD_VDISP;
#endif
    mapModId["RTSP"] = E_SYS_MOD_RTSP;
    mapModId["FILE"] = E_SYS_MOD_FILE;
    mapModId["SLOT"] = E_SYS_MOD_SLOT;

    Sys::CreateObj(configIni, mapModId);

    objName = "RTSP_CLIENT";
    RtspObj = dynamic_cast<Rtsp *>(Sys::GetInstance(objName));
    if (!RtspObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return;
    }
    RtspObj->GetInfo(info, isOpenOnvif, outCfg);
    for (itOutCfg = outCfg.begin(); itOutCfg != outCfg.end(); ++itOutCfg)
    {
        itOutCfg->second.url = liv555Url;
    }
    RtspObj->UpdateInfo(info, isOpenOnvif, outCfg);
#ifdef INTERFACE_VDEC
    objName = "VDEC_CH0_DEV0";
    VdecObj = dynamic_cast<Vdec *>(Sys::GetInstance(objName));
    if (!VdecObj)
    {
        printf("%s: Obj error!\n", objName.c_str());
        Sys::DestroyObj();
        return;
    }
    VdecObj->GetInfo(VdecInfo, vectVdecOut);
    for (itVdecOut = vectVdecOut.begin(); itVdecOut != vectVdecOut.end(); ++itVdecOut)
    {
        itVdecOut->uintDecOutWidth = width;
        itVdecOut->uintDecOutHeight = height;
    }
    VdecObj->UpdateInfo(VdecInfo, vectVdecOut);
#endif
}
SsClient::~SsClient()
{
    Sys::DestroyObj();
}

void SsClient::Play()
{
    std::map<std::string, Sys *> maskMap;

    Sys::Begin(maskMap);

}
void SsClient::Stop()
{
    std::map<std::string, Sys *> maskMap;

    Sys::End(maskMap);
}
