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

#ifndef __VDISP_H__
#define __VDISP_H__

#include "sys.h"


typedef struct stVdispInputInfo_s
{
    int intPortId;
    int intChnId;
    int intFreeRun;
    int intVdispInX;
    int intVdispInY;
    int intVdispInWidth;
    int intVdispInHeight;
}stVdispInputInfo_t;

typedef struct stVdispOutputInfo_s
{
    int intPortId;
    int intVdispOutWidth;
    int intVdispOutHeight;
    int intVdispOutPts;
    int intVdispOutFormat;
    int intVdispOutBkColor;
    int intVdispOutFrameRate;
}stVdispOutputInfo_t;


class Vdisp: public Sys
{
    public:
        Vdisp();
        virtual ~Vdisp();
        void GetInfo(std::vector<stVdispInputInfo_t> &in, std::vector<stVdispOutputInfo_t> &out)
        {
            in = vVdispInputInfo;
            out = vVdispOutputInfo;
        }
        void UpdateInfo(std::vector<stVdispInputInfo_t> &in, std::vector<stVdispOutputInfo_t> &out)
        {
            vVdispInputInfo = in;
            vVdispOutputInfo = out;
            for (unsigned int i = 0; i < vVdispOutputInfo.size(); i++)
            {
                if (mapModOutputInfo.find(vVdispOutputInfo[i].intPortId) != mapModOutputInfo.end())
                {
                    mapModOutputInfo[vVdispOutputInfo[i].intPortId].stStreanInfo.eStreamType = (E_STREAM_TYPE)vVdispOutputInfo[i].intVdispOutFormat;
                    mapModOutputInfo[vVdispOutputInfo[i].intPortId].stStreanInfo.stFrameInfo.streamWidth = vVdispOutputInfo[i].intVdispOutWidth;
                    mapModOutputInfo[vVdispOutputInfo[i].intPortId].stStreanInfo.stFrameInfo.streamHeight = vVdispOutputInfo[i].intVdispOutHeight;
                }
            }
        };

    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void PrevIntBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc);
        virtual void PrevIntUnBind(stModInputInfo_t & stIn, stModDesc_t &stPreDesc);
        virtual void Deinit();
        std::vector<stVdispInputInfo_t> vVdispInputInfo;
        std::vector<stVdispOutputInfo_t> vVdispOutputInfo;
};
#endif


