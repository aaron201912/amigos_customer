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

#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <string.h>

#include "sys.h"


#define NALU_PACKET_SIZE            256*1024
#define U32VALUE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])


typedef struct stFileOutInfo_s
{
    std::string fileName;
    int intReadFd;
    int intFileOutType;
    int intFileFmt;
    int intFileOutWidth;
    int intFileOutHeight;
}stFileOutInfo_t;

typedef struct stFileInInfo_s
{
    std::string fileName;
    long intWriteFd;
}stFileInInfo_t;


class File: public Sys
{
    public:
        File();
        virtual ~File();
        void GetInfo(std::map<unsigned int, stFileInInfo_t> &in, std::map<unsigned int, stFileOutInfo_t> &out)
        {
            in = mapInputWrFile;
            out = mapOutputRdFile;
        }
        void UpdateInfo(std::map<unsigned int, stFileInInfo_t> &in, std::map<unsigned int, stFileOutInfo_t> &out)
        {
            std::map<unsigned int, stFileOutInfo_t>::iterator itFileOut;

            mapInputWrFile = in;
            mapOutputRdFile = out;
            for (itFileOut = mapOutputRdFile.begin(); itFileOut != mapOutputRdFile.end(); itFileOut++)
            {
                mapModOutputInfo[itFileOut->first].stStreanInfo.eStreamType = (E_STREAM_TYPE)itFileOut->second.intFileFmt;
                mapModOutputInfo[itFileOut->first].stStreanInfo.stFrameInfo.streamWidth = itFileOut->second.intFileOutWidth;
                mapModOutputInfo[itFileOut->first].stStreanInfo.stFrameInfo.streamHeight = itFileOut->second.intFileOutHeight;
            }
        };
    private:
        virtual void LoadDb();
        virtual void Init();
        virtual void Deinit();
        void BindBlock(stModInputInfo_t & stIn);
        void UnBindBlock(stModInputInfo_t & stIn);

    protected:
        virtual int CreateSender(unsigned int outPortId);
        virtual int StopSender(unsigned int outPortId);

        static void DataReceiver(void *pData, unsigned int dataSize, void *pUsrData,  unsigned char portId);
    private:
        static void *SenderMonitor(ST_TEM_BUFFER stBuf);

        unsigned char uConnection;
        std::map<unsigned int, stFileInInfo_t> mapInputWrFile;
        std::map<unsigned int, stFileOutInfo_t> mapOutputRdFile;
};
#endif

