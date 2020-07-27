#ifndef __DISP_H__
#define __DISP_H__

#include "sys.h"


typedef struct stDispInfo_s
{
    int intDeviceType; //0: panel 1: hdmi: 2: vga
    int intPanelLinkType; //0: mipi 11: ttl
    int intBackGroundColor;
}stDispInfo_t;

typedef struct stDispInputLayerPortInfo_s
{
    unsigned int uintSrcWidth;
    unsigned int uintSrcHeight;
    unsigned int uintDstWidth;
    unsigned int uintDstHeight;
    unsigned int uintDstXpos;
    unsigned int uintDstYpos;
}stDispLayerInputPortInfo_t;
typedef struct stDispLayerInfo_s
{
    unsigned int uintRot;
    unsigned int uintWidth;
    unsigned int uintHeight;
    unsigned int uintDispWidth;
    unsigned int uintDispHeight;
    unsigned int uintDispXpos;
    unsigned int uintDispYpos;
    std::map<unsigned int, stDispLayerInputPortInfo_t> mapLayerInputPortInfo;
}stDispLayerInfo_t;


class Disp: public Sys
{
    public:
        Disp();
        virtual ~Disp();
        void GetInfo(stDispInfo_t &info, std::map<unsigned int, stDispLayerInfo_t> &layerInfo)
        {
            info = stDispInfo;
            layerInfo = mapLayerInfo;
        }
        void UpdateInfo(stDispInfo_t &info, std::map<unsigned int, stDispLayerInfo_t> &layerInfo)
        {
            stDispInfo = info;
            mapLayerInfo = layerInfo;
        };

    private:
        virtual void Init();
        virtual void Deinit();
        virtual void LoadDb();

        stDispInfo_t stDispInfo;
        std::map<unsigned int, stDispLayerInfo_t> mapLayerInfo;
};
#endif

