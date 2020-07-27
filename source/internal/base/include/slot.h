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

#ifndef __SLOT_H__
#define __SLOT_H__

#include "sys.h"

typedef struct stSlotInputInfo_s
{
    unsigned int uintDstBindMod;
    unsigned int uintDstBindDev;
    unsigned int uintDstBindChannel;
    unsigned int uintDstBindPort;
}stSlotInputInfo_t;

class Slot: public Sys
{
    public:
        Slot();
        virtual ~Slot();
    private:
        virtual void BindBlock(stModInputInfo_t & stIn);
        virtual void UnBindBlock(stModInputInfo_t & stIn);
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();
        std::map<unsigned int, stSlotInputInfo_t> mapSlotInputInfo;
};
#endif

