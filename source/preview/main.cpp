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
#include "amigos.h"

#ifdef INTERFACE_VDEC
AMIGOS_MODULE_SETUP(Vdec);
#endif
#ifdef INTERFACE_DISP
AMIGOS_MODULE_SETUP(Disp);
#endif
AMIGOS_MODULE_SETUP(Rtsp);
#ifdef INTERFACE_VENC
AMIGOS_MODULE_SETUP(Venc);
#endif
#ifdef INTERFACE_VPE
AMIGOS_MODULE_SETUP(Vpe);
#endif
#ifdef INTERFACE_VIF
AMIGOS_MODULE_SETUP(Vif);
#endif
#ifdef INTERFACE_DIVP
AMIGOS_MODULE_SETUP(Divp);
#endif
#ifdef INTERFACE_RGN
AMIGOS_MODULE_SETUP(Ui);
#endif
#ifdef INTERFACE_IQSERVER
AMIGOS_MODULE_SETUP(Iq);
#endif
AMIGOS_MODULE_SETUP(File);
#ifdef INTERFACE_VDISP
AMIGOS_MODULE_SETUP(Vdisp);
#endif
#ifdef INTERFACE_AI
AMIGOS_MODULE_SETUP(Ai);
#endif
#ifdef INTERFACE_AO
AMIGOS_MODULE_SETUP(Ao);
#endif
#ifdef INTERFACE_SENSOR
AMIGOS_MODULE_SETUP(Snr);
#endif
#ifdef INTERFACE_SYS
AMIGOS_MODULE_SETUP(Uac);
AMIGOS_MODULE_SETUP(Uvc);
AMIGOS_MODULE_SETUP(Empty);
AMIGOS_MODULE_SETUP(Inject);
AMIGOS_MODULE_SETUP(Slot);
#endif
int main(int argc, char **argv)
{
    amigos_setup_ui(argc, argv);
    return 0;
}
