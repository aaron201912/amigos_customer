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

#ifndef __VIF_H__
#define __VIF_H__

#include "sys.h"

typedef struct stVifInfo_s
{
    int intSensorId;
    int intHdrType;
    int intWorkMode;
}stVifInfo_t;
typedef struct stVifOutInfo_s
{
    int intWidth;
    int intHeight;
    int intIsUseSnrFmt;
    int intUserFormat;
}stVifOutInfo_t;


class Vif: public Sys
{
    public:
        Vif();
        virtual ~Vif();
        void GetInfo(stVifInfo_t &info, std::map<unsigned int, stVifOutInfo_t> &out)
        {
            info = stVifInfo;
            out = mapVifOutInfo;
        }
        void UpdateInfo(stVifInfo_t &info, std::map<unsigned int, stVifOutInfo_t> &out)
        {
            stVifInfo = info;
            mapVifOutInfo = out;
        };
    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();
        stVifInfo_t stVifInfo;
        std::map<unsigned int, stVifOutInfo_t> mapVifOutInfo;
};
#endif


