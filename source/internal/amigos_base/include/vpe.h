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

#ifndef __VPE_H__
#define __VPE_H__

#include "sys.h"

typedef struct stVpeInfo_s
{
    int intHdrType;
    int intbUseSnrFmt;
    int intSensorId;
    int intInputFmt;
    int intRotation;
    int int3dNrLevel;
    int intRunningMode;
    int intChnPortMode;
    int intVidWidth;
    int intVidHeight;
}stVpeInfo_t;

typedef struct stVpeOutInfo_s
{
    int intPortId;
    int intVpeOutFmt;
    int intVpeOutWidth;
    int intVpeOutHeight;
}stVpeOutInfo_t;

class Vpe: public Sys
{
    public:
        Vpe();
        virtual ~Vpe();
        void GetInfo(stVpeInfo_t &info, std::vector<stVpeOutInfo_t> &out)
        {
            info = stVpeInfo;
            out = vVpeOutInfo;
        }
        void UpdateInfo(stVpeInfo_t &info, std::vector<stVpeOutInfo_t> &out)
        {
            stVpeInfo = info;
            vVpeOutInfo = out;
            for (unsigned int i = 0; i < vVpeOutInfo.size(); i++)
            {
                if (mapModOutputInfo.find(vVpeOutInfo[i].intPortId) != mapModOutputInfo.end())
                {
                    mapModOutputInfo[vVpeOutInfo[i].intPortId].stStreanInfo.eStreamType = (E_STREAM_TYPE)vVpeOutInfo[i].intVpeOutFmt;
                    mapModOutputInfo[vVpeOutInfo[i].intPortId].stStreanInfo.stFrameInfo.streamWidth = vVpeOutInfo[i].intVpeOutWidth;
                    mapModOutputInfo[vVpeOutInfo[i].intPortId].stStreanInfo.stFrameInfo.streamHeight = vVpeOutInfo[i].intVpeOutHeight;
                }
            }
        };

    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();
        virtual void Stop();
        virtual void ResetOut(unsigned int outPortId, stStreamInfo_t *pInfo);

        stVpeInfo_t stVpeInfo;
        std::vector<stVpeOutInfo_t> vVpeOutInfo;
};
#endif

