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
#include <map>
#include <string>
#include <iostream>

#include "dynamic.h"

Dynamic *Dynamic::pCurrentIns = NULL;
dictionary *Dynamic::m_pstDict = NULL;

Dynamic::Dynamic()
{
    PREPARE_FUNCTION("RES_CHANGE", &Dynamic::DoResChange);
}
Dynamic::~Dynamic()
{
}
void Dynamic::Select(std::string &iniString)
{
    unsigned int uintTestCaseCnt = 0, i = 0;
    unsigned int uintChoice;
    char *pCaseName = NULL, *pCaseTittle = NULL, *pIniGet = NULL;
    char strCaseIndex[30];
    std::string strSysName;
    std::string strSubCaseName;
    std::string strFuncName;
    std::string strDefaultCase;
    Sys *pSysClass = NULL;
    std::map<std::string, fpCmdFunction>::iterator itMapFunction;
    char idx[8];

    if (!m_pstDict)
    {
        m_pstDict = iniparser_load(iniString.c_str());
        if (!m_pstDict)
        {
            AMIGOS_ERR("INI file: [%s] read error!\n", iniString.c_str());
            return;
        }
    }
    while (1)
    {
        uintTestCaseCnt = GetIniUnsignedInt("TEST_CASE", "CASE_CNT", 0);
        if (!uintTestCaseCnt)
        {
            AMIGOS_ERR("Case count is 0\n");
            iniparser_freedict(m_pstDict);
            m_pstDict = NULL;

            return;
        }
        memset(strCaseIndex, 0, 30);
        pCaseTittle = GetIniString("TEST_CASE", "TITTLE", NULL);
        if (pCaseTittle != NULL)
            std::cout << ">====================" << pCaseTittle << "====================<" << std::endl;
        if (!strDefaultCase.size())
            strDefaultCase = GetIniString("TEST_CASE", "DEFAULT_CASE", NULL);
        for (i = 0; i < uintTestCaseCnt; i++)
        {
            snprintf(strCaseIndex, 30, "CASE%d", i);
            pCaseName = GetIniString("TEST_CASE", strCaseIndex, NULL);
            if (pCaseName != NULL)
                std::cout << "CASE" << i << ": " << pCaseName << ((strDefaultCase != strCaseIndex) ? "" : "<=====default") << std::endl;
            else
                AMIGOS_ERR("Key %s parse error!\n", strCaseIndex);
        }
        std::cout << "Press 'q' to exit" << std::endl;
        std::cout << "Please select your choice:" << std::endl;
        fflush(stdin);
        scanf("%4s", idx);
        if (strncmp("q", idx, 1) == 0)
        {
            break;
        }
        if (strncmp("p", idx, 1) == 0)
        {
            continue;
        }
        uintChoice = atoi(idx);
        memset(strCaseIndex, 0, 30);
        snprintf(strCaseIndex, 30, "TEST_CASE%d", uintChoice);
        pIniGet = GetIniString(strCaseIndex, "TARGET", NULL);
        if (!pIniGet)
        {
            AMIGOS_ERR("TARGET parse error!\n");
            continue;
        }
        strSysName = pIniGet;
        pSysClass = Sys::GetInstance(strSysName);
        if (!pSysClass)
        {
            AMIGOS_ERR("Sys instance error!\n");
            continue;
        }
        pIniGet = GetIniString(strCaseIndex, "FUNCTION", NULL);
        if (!pIniGet)
        {
            AMIGOS_ERR("FUNCTION parse error!\n");
            continue;
        }
        strFuncName = pIniGet;
        itMapFunction = GetInstance()->mapCmdFunction.find(strFuncName);
        if (itMapFunction !=  GetInstance()->mapCmdFunction.end())
        {
            strSubCaseName = strCaseIndex;
            (GetInstance()->*(itMapFunction->second))(*pSysClass, strSubCaseName);
        }
        else
        {
            AMIGOS_ERR("Not found FUNCTION %s!\n", strFuncName.c_str());
            continue;
        }
        strDefaultCase = "CASE";
        strDefaultCase += idx;
    }
    DestroyInstance();
    iniparser_freedict(m_pstDict);
    m_pstDict = NULL;
}
int Dynamic::GetIniInt(std::string section, std::string key, int intDefault)
{
    std::string strTmp;
    int intRet = intDefault;

    if (!m_pstDict)
    {
        AMIGOS_ERR("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    //AMIGOS_INFO("GET STR[%s]\n", strTmp.c_str());
    intRet = iniparser_getint(m_pstDict, strTmp.c_str(), intDefault);
    if (intRet == intDefault && intRet == -1)
    {
        AMIGOS_ERR("Get SECTION: %s KEY %s error!\n", section.c_str(), key.c_str());
    }

    return intRet;
}
unsigned int Dynamic::GetIniUnsignedInt(std::string section, std::string key, unsigned int uintDefault)
{
    std::string strTmp;
    unsigned int uintRet = uintDefault;

    if (!m_pstDict)
    {
        AMIGOS_ERR("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    //AMIGOS_INFO("GET STR[%s]\n", strTmp.c_str());
    uintRet = iniparser_getunsignedint(m_pstDict, strTmp.c_str(), uintDefault);
    if (uintRet == uintDefault && uintRet == (unsigned int)-1)
    {
        AMIGOS_ERR("Get SECTION: %s KEY %s error!\n", section.c_str(), key.c_str());
    }

    return uintRet;
}
char* Dynamic::GetIniString(std::string section, std::string key, char *pDefaultStr)
{
    std::string strTmp;
    char *pRet = pDefaultStr;

    if (!m_pstDict)
    {
        AMIGOS_ERR("INI file not found!\n");
        assert(NULL);
    }
    strTmp = section + ':' + key;

    //AMIGOS_INFO("GET STR[%s]\n", strTmp.c_str());
    pRet = iniparser_getstring(m_pstDict, strTmp.c_str(), pDefaultStr);
    if (pDefaultStr == pRet && pDefaultStr == NULL)
    {
        AMIGOS_ERR("Get SECTION: %s KEY %s error!\n", section.c_str(), key.c_str());
    }

    return pRet;
}

void Dynamic::DoResChange(Sys &sysClass, std::string &strSection)
{
    AMIGOS_INFO("SECTION : %s To do...\n", strSection.c_str());
}
