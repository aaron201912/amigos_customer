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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "file.h"

File::File()
{
}
File::~File()
{
}
void File::LoadDb()
{
    stFileOutInfo_t stFileOutput;
    stFileInInfo_t stFileInInfo;
    std::map<unsigned int, stModInputInfo_t>::iterator itMapFileIn;
    std::map<unsigned int, stModOutputInfo_t>::iterator itMapFileOut;

    for (itMapFileIn = mapModInputInfo.begin(); itMapFileIn != mapModInputInfo.end(); itMapFileIn++)
    {
        stFileInInfo.fileName = GetIniString(itMapFileIn->second.curIoKeyString, "FILE_WRITE_PATH");
        mapInputWrFile[itMapFileIn->second.curPortId] = stFileInInfo;
    }
    for (itMapFileOut = mapModOutputInfo.begin(); itMapFileOut != mapModOutputInfo.end(); itMapFileOut++)
    {
        stFileOutput.fileName = GetIniString(itMapFileOut->second.curIoKeyString, "FILE_READ_PATH");
        stFileOutput.intFileOutType = GetIniInt(itMapFileOut->second.curIoKeyString, "DATA_TYPE");
        itMapFileOut->second.stStreanInfo.eStreamType = (E_STREAM_TYPE)stFileOutput.intFileOutType;
        stFileOutput.intFileFmt = GetIniInt(itMapFileOut->second.curIoKeyString, "DATA_FMT");
        switch (stFileOutput.intFileOutType)
        {
            case E_STREAM_VIDEO_RAW_DATA:
            {
                itMapFileOut->second.stStreanInfo.stFrameInfo.streamWidth = stFileOutput.intFileOutWidth = GetIniInt(itMapFileOut->second.curIoKeyString,  "VID_W");
                itMapFileOut->second.stStreanInfo.stFrameInfo.streamHeight = stFileOutput.intFileOutHeight = GetIniInt(itMapFileOut->second.curIoKeyString, "VID_H");
                itMapFileOut->second.stStreanInfo.stFrameInfo.enVideoRawFmt = (E_VIDEO_RAW_FORMAT)stFileOutput.intFileFmt;
            }
            break;
            case E_STREAM_VIDEO_CODEC_DATA:
            {
                itMapFileOut->second.stStreanInfo.stEsInfo.streamWidth = stFileOutput.intFileOutWidth = GetIniInt(itMapFileOut->second.curIoKeyString,  "VID_W");
                itMapFileOut->second.stStreanInfo.stEsInfo.streamHeight = stFileOutput.intFileOutHeight = GetIniInt(itMapFileOut->second.curIoKeyString, "VID_H");
                itMapFileOut->second.stStreanInfo.stEsInfo.enVideoCodecFmt = (E_VIDEO_CODEC_FORMAT)stFileOutput.intFileFmt;

            }
            break;
            case E_STREAM_AUDIO_CODEC_DATA:
            {
                //to do...
            }
            break;
            default:
                ASSERT(0);
        }
        mapOutputRdFile[itMapFileOut->second.curPortId] = stFileOutput;
        itMapFileOut->second.stStreanInfo.eStreamType = (E_STREAM_TYPE)stFileOutput.intFileOutType;

    }
}
void File::Init()
{
    std::map<unsigned int, stFileOutInfo_t>::iterator itFileOut;

    for (itFileOut = mapOutputRdFile.begin(); itFileOut != mapOutputRdFile.end(); itFileOut++)
    {
        if ((access(itFileOut->second.fileName.c_str(), F_OK)) ==-1)
        {
            AMIGOS_ERR("not access %s file!\n", itFileOut->second.fileName.c_str());
            itFileOut->second.intReadFd = -1;
        }

        itFileOut->second.intReadFd = open(itFileOut->second.fileName.c_str(), O_RDONLY);
        if(itFileOut->second.intReadFd < 0)
        {
            AMIGOS_ERR("read_file: %s. open fail\n", itFileOut->second.fileName.c_str());
        }
    }
}
void File::Deinit()
{
    std::map<unsigned int, stFileOutInfo_t>::iterator it;
    for(it=mapOutputRdFile.begin();it != mapOutputRdFile.end();++it)
    {
        if (it->second.intReadFd != -1)
        {
            close(it->second.intReadFd);
        }
    }
}
void File::BindBlock(stModInputInfo_t & stIn)
{
    stModDesc_t stPreDesc;

    GetInstance(stIn.stPrev.modKeyString)->GetModDesc(stPreDesc);

    std::map<unsigned int, stFileInInfo_t>::iterator it;

    //AMIGOS_INFO("Bind!! Cur %s modid %d chn %d dev %d port %d fps %d\n", stIn.curIoKeyString.c_str(), stModDesc.modId, stModDesc.chnId, stModDesc.devId, stIn.curPortId, stIn.curFrmRate);
    //AMIGOS_INFO("Pre %s modid %d chn %d dev %d port %d fps %d\n", stIn.stPrev.modKeyString.c_str(), stPreDesc.modId, stPreDesc.chnId, stPreDesc.devId, stIn.stPrev.portId, stIn.stPrev.frmRate);

    it = mapInputWrFile.find(stIn.curPortId);
    if (it != mapInputWrFile.end())
    {
        it->second.intWriteFd = open(it->second.fileName.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0777);
        if (it->second.intWriteFd < 0)
        {
            AMIGOS_INFO("dest_file: %s.\n", it->second.fileName.c_str());
            perror("open");
            return;
        }
        CreateReceiver(stIn.curPortId, DataReceiver, (void *)it->second.intWriteFd);
        StartReceiver(stIn.curPortId);
    }
}
void File::UnBindBlock(stModInputInfo_t & stIn)
{
    std::map<unsigned int, stFileInInfo_t>::iterator it;

    it = mapInputWrFile.find(stIn.curPortId);
    if (it != mapInputWrFile.end())
    {
        StopReceiver(stIn.curPortId);
        DestroyReceiver(stIn.curPortId);
        close(it->second.intWriteFd);
    }
}

void File::DataReceiver(void *pData, unsigned int dataSize, void *pUsrData, unsigned char portId)
{
    int intFd = 0;
    stStreamData_t *pStreamData = NULL;

    intFd = (int)pUsrData;
    if (sizeof(stStreamData_t) == dataSize)
    {
        pStreamData = (stStreamData_t*)pData;

        switch (pStreamData->stInfo.eStreamType)
        {
            case E_STREAM_VIDEO_RAW_DATA:
            {
                switch (pStreamData->stInfo.stFrameInfo.enVideoRawFmt)
                {
                    case E_STREAM_YUV422:
#ifndef SSTAR_CHIP_I2
                    case E_STREAM_YUV422_UYVY:
                    case E_STREAM_YUV422_YVYU:
                    case E_STREAM_YUV422_VYUY:
#endif
                    {
                        int yuv_size = pStreamData->stInfo.stFrameInfo.streamWidth * pStreamData->stInfo.stFrameInfo.streamHeight * 2;
                        char *dst_buf;

                        dst_buf = pStreamData->pYuvData;
                        if(write(intFd, dst_buf, yuv_size) != yuv_size)
                        {
                            AMIGOS_ERR("write yuv data err!\n");
                            return;
                        }
                    }
                    break;
                    case E_STREAM_YUV420:
                    {
                        int y_size = pStreamData->stInfo.stFrameInfo.streamWidth * pStreamData->stInfo.stFrameInfo.streamHeight;
                        int uv_size = y_size/2;
                        char *dst_buf;

                        dst_buf = pStreamData->stYuvSpData.pYdataAddr;
                        if(write(intFd, dst_buf, y_size) != y_size)
                        {
                            AMIGOS_ERR("write yuv data err!\n");
                            return;
                        }
                        dst_buf = pStreamData->stYuvSpData.pUvDataAddr;
                        if(write(intFd, dst_buf, uv_size) != uv_size)
                        {
                            AMIGOS_ERR("write uvdata err!\n");
                            return;
                        }
                    }
                    break;
                    default:
                        break;
                }
            }
            break;
            case E_STREAM_VIDEO_CODEC_DATA:
            {
                switch (pStreamData->stInfo.stEsInfo.enVideoCodecFmt)
                {
                    case E_STREAM_H264:
                    case E_STREAM_H265:
                    {
#if 0
                        MI_U8 au8Header[16];
                        static int cnt = 0;

                        if (cnt == 1200)
                            break;
                        for (MI_U8 i = 0; i < pStreamData->stEsData.uintPackCnt; i++)
                        {
                            memset(au8Header, 0, 16);
                            au8Header[0] = 0x1;
                            au8Header[4] = ((pStreamData->stEsData.pDataAddr[i].uintDataSize) >> 24) & 0xFF;
                            au8Header[5] = ((pStreamData->stEsData.pDataAddr[i].uintDataSize) >> 16) & 0xFF;
                            au8Header[6] = ((pStreamData->stEsData.pDataAddr[i].uintDataSize) >> 8)& 0xFF;
                            au8Header[7] = (pStreamData->stEsData.pDataAddr[i].uintDataSize) & 0xFF;
                            write(intFd, (void *)au8Header, 16);
                            write(intFd, (void *)pStreamData->stEsData.pDataAddr[i].pData, pStreamData->stEsData.pDataAddr[i].uintDataSize);
                            AMIGOS_INFO("Write fine frame count %d\n", cnt);
                        }
                        cnt++;
#else
                        for (MI_U8 i = 0; i < pStreamData->stEsData.uintPackCnt; i++)
                        {
                             write(intFd, (void *)pStreamData->stEsData.pDataAddr[i].pData, pStreamData->stEsData.pDataAddr[i].uintDataSize);
                        }
#endif
                    }
                    break;
                    default:
                        break;

                }
            }
            break;
            case E_STREAM_AUDIO_CODEC_DATA:
            {
                switch (pStreamData->stInfo.stPcmInfo.enAudioCodecFmt)
                {
                    case E_STREAM_PCM:
                    {
                        write(intFd, (void *)pStreamData->stPcmData.pData, pStreamData->stPcmData.uintSize);
                    }
                    break;
                    default:
                        //AMIGOS_ERR("Not support!!\n");
                        //assert(0);
                        break;
                }
            }
            break;
            default:
                break;
        }
    }
    else
    {
        assert(0);
    }
}
int File::StopSender(unsigned int outPortId)
{
    if (uConnection == 1)
    {
        uConnection = 0;
    }
    TemStop(mapModOutputInfo[outPortId].curIoKeyString.c_str());

    return 0;
}

int File::CreateSender(unsigned int outPortId)
{
    ST_TEM_ATTR stTemAttr;
    stReceiverDesc_t stDest;

    PTH_RET_CHK(pthread_attr_init(&stTemAttr.thread_attr));
    memset(&stTemAttr, 0, sizeof(ST_TEM_ATTR));
    stTemAttr.fpThreadDoSignal = NULL;
    stTemAttr.fpThreadWaitTimeOut = SenderMonitor;
    stTemAttr.u32ThreadTimeoutMs = 1000 / mapModOutputInfo[outPortId].curFrmRate;
    stTemAttr.bSignalResetTimer = 0;
    stTemAttr.stTemBuf.pTemBuffer = (void *)&(mapRecevier[outPortId]);
    stTemAttr.stTemBuf.u32TemBufferSize = 0;
    stTemAttr.maxEventCout = 30;
    stTemAttr.bDropEvent = FALSE;
    TemOpen(mapModOutputInfo[outPortId].curIoKeyString.c_str(), stTemAttr);

    return 0;
}
MI_S32 GetOneFrameYUV420(int srcFd, char *pYData, char *pUvData, int ySize, int uvSize)
{
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);

    if ((end - current) ==0)
    {
        lseek(srcFd, 0, SEEK_SET);
        current = lseek(srcFd,0L, SEEK_CUR);
        end = lseek(srcFd,0L, SEEK_END);
    }

    if((end - current) < (ySize + uvSize))
    {
        return -1;
    }

    lseek(srcFd, current, SEEK_SET);
    if (read(srcFd, pYData, ySize) == ySize
        && read(srcFd, pUvData, uvSize) == uvSize)
    {
        return 1;
    }

    return -1;
}

MI_S32 GetOneFrame(int srcFd, char *pYuvData, int intSize)
{
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);

    if ((end - current) ==0)
    {
        lseek(srcFd, 0, SEEK_SET);
        current = lseek(srcFd,0L, SEEK_CUR);
        end = lseek(srcFd,0L, SEEK_END);
    }

    if ((end - current) < intSize)
    {
        return -1;
    }

    lseek(srcFd, current, SEEK_SET);
    if (read(srcFd, pYuvData, intSize) == intSize)
    {
        return 1;
    }

    return 0;
}
void * File::SenderMonitor(ST_TEM_BUFFER stBuf)
{
    int readfp = -1;
    //MI_U32 u32Pos = 0;
    MI_S32 s32Len = 0;
    MI_U8 *pu8Buf = NULL;
    MI_U32 u32FrameLen = 0;
    MI_U8 au8Header[32] = {0};
    int ret = -1;
    int y_size = 0;
    int uv_size = 0;
    int dataSize = 0;
    char *pYdata = NULL;
    char *pUvdata = NULL;
    char *pData = NULL;
    stStreamData_t stFileStreamData;
    stEsPackage_t stEsPacket[1];


    stReceiverDesc_t *pReceiver = (stReceiverDesc_t *)stBuf.pTemBuffer;
    File *pSendClass = dynamic_cast<File *>(pReceiver->pSysClass);

    readfp = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intReadFd;
    if(readfp < 0)
    {
        AMIGOS_ERR("File[%s] send file handle is null!\n", pSendClass->mapOutputRdFile[pReceiver->uintPort].fileName.c_str());
        return NULL;
    }

    memset(&stFileStreamData, 0, sizeof(stStreamData_t));
    stFileStreamData.stInfo.eStreamType = (E_STREAM_TYPE)((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutType;

    switch (stFileStreamData.stInfo.eStreamType)
    {
        case E_STREAM_VIDEO_RAW_DATA:
        {
            stFileStreamData.stInfo.stFrameInfo.enVideoRawFmt = (E_VIDEO_RAW_FORMAT)((File *)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileFmt;
            switch (stFileStreamData.stInfo.stFrameInfo.enVideoRawFmt)
            {
                case E_STREAM_YUV420:
                {
                    stFileStreamData.stInfo.stFrameInfo.streamWidth = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutWidth;
                    stFileStreamData.stInfo.stFrameInfo.streamHeight = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutHeight;
                    y_size  = stFileStreamData.stInfo.stFrameInfo.streamWidth * stFileStreamData.stInfo.stFrameInfo.streamHeight;
                    uv_size  = y_size/2;
                    pYdata = (char*)malloc(y_size);
                    pUvdata = (char*)malloc(uv_size);
                    if(pYdata == NULL || pUvdata == NULL)
                    {
                        goto free_buf;
                    }
                    ret =  GetOneFrameYUV420(readfp, pYdata, pUvdata, y_size, uv_size);
                    if(ret == 1)
                    {
                        stFileStreamData.stYuvSpData.pYdataAddr  = pYdata;
                        stFileStreamData.stYuvSpData.pUvDataAddr = pUvdata;
                        if (pSendClass->uConnection == 0)
                        {
                            pSendClass->Connect(pReceiver->uintPort, &stFileStreamData.stInfo);
                            pSendClass->uConnection = 1;
                        }
                        pSendClass->Send(pReceiver->uintPort, &stFileStreamData, sizeof(stStreamData_t));
                    }
                }
                break;
                case E_STREAM_YUV422:
#ifndef SSTAR_CHIP_I2
                case E_STREAM_YUV422_UYVY:
                case E_STREAM_YUV422_YVYU:
                case E_STREAM_YUV422_VYUY:
#endif
                {
                    stFileStreamData.stInfo.stFrameInfo.streamWidth = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutWidth;
                    stFileStreamData.stInfo.stFrameInfo.streamHeight = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutHeight;
                    dataSize = stFileStreamData.stInfo.stFrameInfo.streamWidth * stFileStreamData.stInfo.stFrameInfo.streamHeight * 2;
                    pData = (char*)malloc(dataSize);
                    ret =  GetOneFrame(readfp, pData, dataSize);
                    if(ret == 1)
                    {
                        stFileStreamData.pYuvData  = pData;
                        if (pSendClass->uConnection == 0)
                        {
                            pSendClass->Connect(pReceiver->uintPort, &stFileStreamData.stInfo);
                            pSendClass->uConnection = 1;
                        }
                        pSendClass->Send(pReceiver->uintPort, &stFileStreamData, sizeof(stStreamData_t));
                    }
                }
                break;
#ifndef SSTAR_CHIP_I2
                case E_STREAM_ARGB8888:
                case E_STREAM_ABGR8888:
                case E_STREAM_BGRA8888:
                {
                    stFileStreamData.stInfo.stFrameInfo.streamWidth = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutWidth;
                    stFileStreamData.stInfo.stFrameInfo.streamHeight = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutHeight;
                    dataSize = stFileStreamData.stInfo.stFrameInfo.streamWidth * stFileStreamData.stInfo.stFrameInfo.streamHeight * 4;
                    pData = (char*)malloc(dataSize);
                    ret =  GetOneFrame(readfp, pData, dataSize);
                    if(ret == 1)
                    {
                        stFileStreamData.pYuvData  = pData;
                        if (pSendClass->uConnection == 0)
                        {
                            pSendClass->Connect(pReceiver->uintPort, &stFileStreamData.stInfo);
                            pSendClass->uConnection = 1;
                        }
                        pSendClass->Send(pReceiver->uintPort, &stFileStreamData, sizeof(stStreamData_t));
                    }
                }
                break;
                case E_STREAM_RGB_BAYER_BASE...E_STREAM_RGB_BAYER_MAX:
                {
                    stFileStreamData.stInfo.stFrameInfo.streamWidth = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutWidth;
                    stFileStreamData.stInfo.stFrameInfo.streamHeight = ((File*)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutHeight;
                    dataSize = stFileStreamData.stInfo.stFrameInfo.streamWidth * stFileStreamData.stInfo.stFrameInfo.streamHeight;
                    pData = (char*)malloc(dataSize);
                    ret =  GetOneFrame(readfp, pData, dataSize);
                    if(ret == 1)
                    {
                        stFileStreamData.pYuvData  = pData;
                        if (pSendClass->uConnection == 0)
                        {
                            pSendClass->Connect(pReceiver->uintPort, &stFileStreamData.stInfo);
                            pSendClass->uConnection = 1;
                        }
                        pSendClass->Send(pReceiver->uintPort, &stFileStreamData, sizeof(stStreamData_t));
                    }
                }
                break;
#endif
                default:
                    break;
            }
        }
        break;
        case E_STREAM_VIDEO_CODEC_DATA:
        {
            stFileStreamData.stInfo.stEsInfo.enVideoCodecFmt = (E_VIDEO_CODEC_FORMAT)((File *)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileFmt;
            switch (stFileStreamData.stInfo.stEsInfo.enVideoCodecFmt)
            {
                case E_STREAM_H264:
                case E_STREAM_H265:
                case E_STREAM_JPEG:
                {
                    stFileStreamData.stInfo.stEsInfo.streamWidth = ((File *)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutWidth;
                    stFileStreamData.stInfo.stEsInfo.streamHeight = ((File *)pSendClass)->mapOutputRdFile[pReceiver->uintPort].intFileOutHeight;
                    memset(&stEsPacket, 0, sizeof(stEsPacket));
                    memset(au8Header, 0, 16);
                    //u32Pos = lseek(readfp, 0, SEEK_CUR);
                    s32Len = read(readfp, au8Header, 16);
                    if (s32Len <= 0)
                    {
                        lseek(readfp, 0, SEEK_SET);
                        goto free_buf;
                    }
                    u32FrameLen = MI_U32VALUE(au8Header, 4);
                    pu8Buf = (MI_U8 *)malloc(u32FrameLen);
                    if (pu8Buf == NULL)
                    {
                        return NULL;
                    }
                    s32Len = read(readfp, pu8Buf, u32FrameLen);
                    if (s32Len <= 0)
                    {
                        lseek(readfp, 0, SEEK_SET);
                        goto free_buf;
                    }

                    stEsPacket[0].uintDataSize = s32Len;
                    stEsPacket[0].pData = (char *)pu8Buf;
                    stEsPacket[0].bSliceEnd = 1;

                    stFileStreamData.stEsData.uintPackCnt = 1;
                    stFileStreamData.stEsData.pDataAddr = stEsPacket;
                    if (pSendClass->uConnection == 0)
                    {
                        pSendClass->Connect(pReceiver->uintPort, &stFileStreamData.stInfo);
                        pSendClass->uConnection = 1;
                    }
                    pSendClass->Send(pReceiver->uintPort, &stFileStreamData, sizeof(stFileStreamData));
                }
                break;
                default:
                    break;
            }
        }
        break;
        default:
            break;
    }
free_buf:
    if(pYdata != NULL)
    {
        free(pYdata);
    }

    if(pUvdata != NULL)
    {
        free(pUvdata);
    }

    if(pData != NULL)
    {
        free(pData);
    }
    if(pu8Buf != NULL)
    {
        free(pu8Buf);
    }

    return NULL;
}

