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

#ifndef __DYNAMIC_H__
#define __DYNAMIC_H__
#include <assert.h>
#include <map>
#include <string>
#include <iostream>
#include "iniparser.h"
#include "sys.h"

#define PREPARE_FUNCTION(cmdStr, fpFunction)    do{ \
    mapCmdFunction[cmdStr] = (fpFunction);    \
}while (0);

#include "sys.h"
class Dynamic
{
    public:
        Dynamic();
        virtual ~Dynamic();
        static void Select(std::string &iniString);

    private:
        
        typedef void (Dynamic::*fpCmdFunction)(Sys &, std::string &);
        //INI operation
        static int GetIniInt(std::string section, std::string key, int intDefault = -1);
        static unsigned int GetIniUnsignedInt(std::string section, std::string key, unsigned int uintDefault = -1);
        static char *GetIniString(std::string section, std::string key, char *pDefaultStr = NULL);

        static Dynamic *GetInstance()
        {
            if (!pCurrentIns)
            {
                pCurrentIns = new (std::nothrow)Dynamic;
                assert(pCurrentIns);
            }
            return pCurrentIns;
        }
        static void DestroyInstance()
        {
            if (pCurrentIns)
                delete pCurrentIns;
            pCurrentIns = NULL;
        }
        static Dynamic *pCurrentIns;
        static dictionary *m_pstDict;
        void DoResChange(Sys &sysClass, std::string &strSection);
        std::map<std::string, fpCmdFunction> mapCmdFunction;
};
#endif
