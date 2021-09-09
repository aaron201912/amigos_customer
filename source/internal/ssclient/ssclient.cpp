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
#include <map>
#include <string>
#include <vector>
#include "sys.h"
#include "amigos.h"
#include "ssclient.h"
#include "rtsp.h"
#include "vdec.h"
AMIGOS_MODULE_SETUP(Ao);
AMIGOS_MODULE_SETUP(Rtsp);
AMIGOS_MODULE_SETUP(File);
AMIGOS_MODULE_SETUP(Vdec);
AMIGOS_MODULE_SETUP(Disp);
AMIGOS_MODULE_SETUP(Slot);
AMIGOS_MODULE_SETUP(Divp);
#ifdef INTERFACE_VDISP
AMIGOS_MODULE_SETUP(Vdisp);
#endif
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

    Sys::CreateObj(configIni);
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
