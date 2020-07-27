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
#include "stdio.h"
#include <stdlib.h>

#include "ssclient.h"

int main(int argc, char **argv)
{
    SsClient *pSstarClient = NULL;
    char getC = 0;

    if (argc != 5)
    {
        printf("Usage: %s [url] [ini path] [width] [height]\n", argv[0]);

        return -1;
    }

    pSstarClient = new SsClient(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
    pSstarClient->Play();
    do
    {
        printf("Press 'q' to exit!\n");
        getC = getchar();
    }while (getC != 'q');
    pSstarClient->Stop();
    delete pSstarClient;

    return 0;
}
