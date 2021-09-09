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

#ifndef __SNR_H__
#define __SNR_H__

#include "sys.h"

typedef struct stSnrInfo_s
{
    int intSensorId;
    int intSensorRes;
    int intHdrType;
}stSnrInfo_t;

class Snr: public Sys
{
    public:
        Snr();
        virtual ~Snr();
        virtual void LoadDb();
        void GetInfo(stSnrInfo_t &info)
        {
            info = stSnrInfo;
        }
        void UpdateInfo(stSnrInfo_t &info)
        {
            stSnrInfo = info;
        };
    private:
        virtual void Init();
        virtual void Deinit();
        stSnrInfo_t stSnrInfo;
};
#endif

