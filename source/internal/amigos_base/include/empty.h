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

#ifndef __EMPTY_H__
#define __EMPTY_H__

#include "sys.h"

class Empty: public Sys
{
    public:
        Empty();
        virtual ~Empty();
    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();
        virtual void BindBlock(stModInputInfo_t & stIn);
        virtual void UnBindBlock(stModInputInfo_t & stIn);
        virtual int CreateSender(unsigned int outPortId);
        virtual int DestroySender(unsigned int outPortId);
        virtual int StartSender(unsigned int outPortId);
        virtual int StopSender(unsigned int outPortId);
        static void DataReceiver(void *pData, unsigned int dataSize, void *pUsrData,  unsigned char portId);
        std::map<unsigned int, unsigned int> mapEmptyInInfo;
};
#endif

