#ifndef __VDEC_H__
#define __VDEC_H__

#include "sys.h"


typedef struct stVdecInfo_s
{
    int dpBufMode;
    int refFrameNum;
    unsigned int bitstreamSize;
    unsigned int uintBufWidth;
    unsigned int uintBufHeight;
}stVdecInfo_t;

typedef struct stDecOutInfo_s
{
    int intPortId;
    unsigned int uintDecOutWidth;
    unsigned int uintDecOutHeight;
}stDecOutInfo_t;

class Vdec: public Sys
{
public:
    Vdec();
    virtual ~Vdec();
    void GetInfo(stVdecInfo_t &info, std::vector<stDecOutInfo_t> &out)
    {
       info = stVdecInfo;
       out = vDecOutInfo;
    }
    void UpdateInfo(stVdecInfo_t &info, std::vector<stDecOutInfo_t> &out)
    {
        stVdecInfo = info;
        vDecOutInfo = out;
    };

private:
    virtual void LoadDb();
    virtual void Init();
    virtual void Deinit();
    virtual void Incoming(stStreamInfo_t *pInfo);
    virtual void Outcoming();
    stVdecInfo_t stVdecInfo;
    std::vector<stDecOutInfo_t> vDecOutInfo;
};
#endif

