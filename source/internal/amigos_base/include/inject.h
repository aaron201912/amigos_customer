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

#ifndef __INJECT_H__
#define __INJECT_H__

#include "sys.h"

typedef struct stInjectOutInfo_s
{
    unsigned int uintBackGroudColor;
    unsigned int uintVideoFmt;
    unsigned int uintVideoWidth;
    unsigned int uintVideoHeight;
    unsigned int bEnableOsd;

    std::string strInjectOsdSrcFile;
    unsigned int uintOsdDelay;
    unsigned int uintOsdColor;
    unsigned int uintOsdFmt;
    unsigned int uintOsdWidth;
    unsigned int uintOsdHeight;
    unsigned int uintOsdTargetPortId;
    unsigned int uintOsdTargetPortWid;
    unsigned int uintOsdTargetPortHei;
    unsigned int uintOsdShowFunction;
}stInjectOutInfo_t;

class Inject: public Sys
{
    public:
        Inject();
        virtual ~Inject();
    private:
        void LoadDb();
        virtual void Init();
        virtual void Deinit();
        virtual void Start();
        virtual void Stop();
        virtual void BindBlock(stModInputInfo_t & stIn);
        virtual void UnBindBlock(stModInputInfo_t & stIn);
        virtual int CreateSender(unsigned int outPortId);
        virtual int DestroySender(unsigned int outPortId);
        static void *SenderMonitor(ST_TEM_BUFFER stBuf);
        std::map<unsigned int, stInjectOutInfo_t> mapInjectOutInfo;
        std::map<unsigned int, void *> mapUserIdToBufHandle;
        static bool bInitRgn;
        static unsigned int uintFrameCnt;
};
#endif

