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
#ifndef __AMIGOS__H__
#define __AMIGOS__H__

#include <stdio.h>
#include "sys.h"
#include "dynamic.h"

std::map<std::string, void (*)(const std::string &)> Sys::AmigosModuleInit::initMap;
#define AMIGOS_MODULE_SETUP(__class)                                                                            \
        extern void __amigos_early_init_preload_##__class(void);                                                \
        static Sys::AmigosModuleInit __amigos_eraly_class_##__class(__amigos_early_init_preload_##__class)

static inline void DynamicChange(std::vector<std::string> &vectDynamicIni)
{
    char idx[8];
    unsigned int sel = 0;

    do
    {
        for (unsigned int i = 0; i < vectDynamicIni.size(); i++)
        {
            printf("Press '%d' to run: %s\n", i, vectDynamicIni[i].c_str());
        }
        printf("Press 'q' to return back!\n");
        fflush(stdin);
        scanf("%4s", idx);
        if (strncmp("q", idx, 1) == 0)
        {
            return;
        }
        else if (strncmp("p", idx, 1) == 0)
        {
            continue;
        }
        sel = atoi(idx);
        if ( sel < vectDynamicIni.size())
        {
            Dynamic::Select(vectDynamicIni[sel]);
        }
        else
        {
            printf("Input select %s error!\n", idx);
        }

    }while (1);
}
static inline int amigos_setup_ui(int argc, char **argv)
{
    std::map<std::string, Sys *> maskMap;
    std::vector<Sys *> objVect;
    std::vector<std::string> vectIniFiles;
    std::vector<std::string> vectDynamicIniFiles;

    unsigned int i = 0, sel = 0, isInit = 0;
    char idx[8];
    int result;

    while ((result = getopt(argc, argv, "id")) != -1)
    {
        switch (result)
        {
            case 'i':
            {
                for (i = 0; i < (unsigned int)argc - optind; i++)
                {
                    if (argv[optind + i][0] == '-')
                    {
                        break;
                    }
                    vectIniFiles.push_back(argv[optind + i]);
                }
            }
            break;
            case 'd':
            {
                for (i = 0; i < (unsigned int)argc - optind; i++)
                {
                    if (argv[optind + i][0] == '-')
                    {
                        break;
                    }
                    vectDynamicIniFiles.push_back(argv[optind + i]);
                }
            }
            break;
            default:
                break;
        }
    }
    if (vectIniFiles.size() == 0)
    {
        printf("Usage: ./%s -i aaa.ini bbb.ini -d xxx_dynamic0.ini xxx_dynamic1.ini\n", argv[0]);

        return -1;
    }
    do
    {
        memset(idx, 0, 8);
        for (i = 0; i < vectIniFiles.size(); i++)
        {
            printf("Press '%d' to run: %s\n", i, vectIniFiles[i].c_str());
        }
        printf("Press 'd' to enter dynamic change menu!\n");
        printf("Press 'q' to exit!\n");
        fflush(stdin);
        scanf("%4s", idx);
        if (strncmp("q", idx, 1) == 0)
        {
            if (isInit)
            {
                Sys::DeinitSys();
            }
            break;
        }
        else if (strncmp("p", idx, 1) == 0)
        {
            continue;
        }
        else if (strncmp("d", idx, 1) == 0)
        {
            DynamicChange(vectDynamicIniFiles);
            continue;
        }
        sel = atoi(idx);
        if ( sel < vectIniFiles.size())
        {

            if (isInit == 1)
            {
                Sys::DeinitSys();
            }
            else
            {
                isInit = 1;
            }
            Sys::InitSys(vectIniFiles[sel]);
        }
        else
        {
            printf("Input select %s error!\n", idx);
        }
    }while (1);

    return 0;
}

#endif //__AMIGOS__H__