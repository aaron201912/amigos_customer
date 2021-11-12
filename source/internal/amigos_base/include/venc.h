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

#ifndef __VENC_H__
#define __VENC_H__

#include "sys.h"

typedef struct stVencVbrConfig_s
{
    int intGop;
    int intMinQp;
    int intMaxQp;
    int intBitRate;
}stVencVbrConfig_t;
typedef struct stVencCbrConfig_s
{
    int intGop;
    int intBitRate;
}stVencCbrConfig_t;
typedef struct stVencFixQpConfig_s
{
    int intGop;
    int intIQp;
    int intPQp;
}stVencFixQpConfig_t;

typedef struct stVencInfo_s
{
    int intWidth;
    int intHeight;
    int intEncodeType;
    int intRcMode; //0 cbr, 1 vbr, 2 vbr fixqp
    union
    {
        stVencVbrConfig_t stVbrCfg;
        stVencCbrConfig_t stCbrCfg;
        stVencFixQpConfig_t stFixQpCfg;
    };
    int intEncodeFps;
    int intMultiSlice;
    int intSliceRowCnt;
}stVencInfo_t;
class Venc: public Sys
{
    public:
        Venc();
        virtual ~Venc();
        void GetInfo(stVencInfo_t &info)
        {
            info = stVencInfo;
        }
        void UpdateInfo(stVencInfo_t &info)
        {
            stVencInfo = info;
            if (mapModOutputInfo.find(0) != mapModOutputInfo.end())
            {
                mapModOutputInfo[0].stStreanInfo.eStreamType = (E_STREAM_TYPE)stVencInfo.intEncodeType;
                mapModOutputInfo[0].stStreanInfo.stFrameInfo.streamWidth = stVencInfo.intWidth;
                mapModOutputInfo[0].stStreanInfo.stFrameInfo.streamHeight = stVencInfo.intHeight;
            }
        };
    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();
        virtual void ResetOut(unsigned int outPortId, stStreamInfo_t *pInfo);
        stVencInfo_t stVencInfo;
};
#endif

