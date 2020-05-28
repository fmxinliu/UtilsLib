#include "StdAfx.h"
#include "IniHelper.h"
#include <assert.h>
#include <sstream>     // std::stringstream

#ifndef USE_STRINGUTILS
#include "StringUtils.h"
namespace UtilTools
{
    bool IniHelper::read(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, int defaultValue, int &value)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, NULL, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        bool convsuccess = StringUtils::isNumeric(mDataStr);
        value = convsuccess ? _ttoi(mDataStr) : defaultValue;
        return convsuccess;
    }

    bool IniHelper::read(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, double defaultValue, double &value)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, NULL, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        bool convsuccess = StringUtils::isNumeric(mDataStr);
        value = convsuccess ? _ttof(mDataStr) : defaultValue;
        return convsuccess;
    }

    int IniHelper::read(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, int defaultValue, bool *pRet)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, NULL, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        bool convsuccess = StringUtils::isNumeric(mDataStr);
        int value = convsuccess ? _ttoi(mDataStr) : defaultValue;
        if (pRet && convsuccess) {
            *pRet = convsuccess;
        }
        return value;
    }

    double IniHelper::read(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, double defaultValue, bool *pRet)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, NULL, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        bool convsuccess = StringUtils::isNumeric(mDataStr);
        double value = convsuccess ? _ttoi(mDataStr) : defaultValue;
        if (pRet && convsuccess) {
            *pRet = convsuccess;
        }
        return value;
    }
}
#endif

namespace UtilTools
{
    /// 读取无符号整型
    UINT readInt(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, int defaultValue)
    {
        return ::GetPrivateProfileInt(lpSection, lpKey, defaultValue, lpIniFileName);
    }

    /// 读取字符串
    String readString(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR defaultValue)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, defaultValue, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        return mDataStr;
    }
}

namespace UtilTools
{
    int IniHelper::readInt(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, NULL, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        int cfgOfInt = _ttoi(mDataStr);
        return cfgOfInt;
    }

    double IniHelper::readDouble(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, NULL, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        double cfgOfInt = _ttof(mDataStr);
        return cfgOfInt;
    }

    String IniHelper::readString(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey)
    {
        TCHAR mDataStr[MAX_PATH];
        DWORD strLength = ::GetPrivateProfileString(lpSection, lpKey, NULL, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        return mDataStr;
    }
}

namespace UtilTools
{
    template<class T>
    String toString(T value)
    {
#ifdef UNICODE
        std::wstringstream ss;
#else
        std::stringstream ss;
#endif
        ss << value;
        return ss.str();
    }

    bool IniHelper::write(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, const int &value)
    {
        String s = toString(value);
        return !!WritePrivateProfileString(lpSection, lpKey, s.c_str(), lpIniFileName);
    }

    bool IniHelper::write(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, const double &value)
    {
        String s = toString(value);
        return !!WritePrivateProfileString(lpSection, lpKey, s.c_str(), lpIniFileName);
    }

    bool IniHelper::write(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, const String &value)
    {
        return !!WritePrivateProfileString(lpSection, lpKey, value.c_str(), lpIniFileName);
    }

    bool IniHelper::read(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, LPVOID lpStruct, UINT uSizeStruct)
    {
        return !!GetPrivateProfileStruct(lpSection, lpKey, lpStruct, uSizeStruct, lpIniFileName);
    }

    bool IniHelper::write(LPCTSTR lpIniFileName, LPCTSTR lpSection, LPCTSTR lpKey, LPVOID lpStruct, UINT uSizeStruct)
    {
        return !!WritePrivateProfileStruct(lpSection, lpKey, lpStruct, uSizeStruct, lpIniFileName);
    }
}

namespace UtilTools
{
    using namespace std;
    vector<String> IniHelper::readSectionNames(LPCTSTR lpIniFileName)
    {
        vector<String> sectionNames;
        TCHAR mDataStr[2049];
        DWORD length = GetPrivateProfileSectionNames(mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        size_t startpos = 0;
        for (int i = 0; i < length; i++) {
            if ('\0' == mDataStr[i]) {
                sectionNames.push_back(String(mDataStr + startpos));
                startpos = i + 1;
            }
        }
        return sectionNames;
    }

    vector<String> IniHelper::readKeys(LPCTSTR lpIniFileName, LPCTSTR lpSection)
    {
        vector<String> keys;
        TCHAR mDataStr[2049];
        DWORD length = GetPrivateProfileSection(lpSection, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        size_t startpos = 0;
        for (int i = 0; i < length; i++) {
            if ('\0' == mDataStr[i]) {
                String feild = mDataStr + startpos;
                size_t endpos = feild.find_first_of('=');
                if (string::npos != endpos) {
                    keys.push_back(feild.substr(0, endpos));
                }

                startpos = i + 1;
            }
        }
        return keys;
    }

    vector<String> IniHelper::readFields(LPCTSTR lpIniFileName, LPCTSTR lpSection)
    {
        vector<String> fields;
        TCHAR mDataStr[2049];
        DWORD length = GetPrivateProfileSection(lpSection, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        size_t startpos = 0;
        for (int i = 0; i < length; i++) {
            if ('\0' == mDataStr[i]) {
                fields.push_back(String(mDataStr + startpos));
                startpos = i + 1;
            }
        }
        return fields;
    }

    map<String, String> IniHelper::parseFields(LPCTSTR lpIniFileName, LPCTSTR lpSection)
    {
        map<String, String> sections;
        TCHAR mDataStr[2049];
        DWORD length = GetPrivateProfileSection(lpSection, mDataStr, sizeof(mDataStr)-1, lpIniFileName);
        size_t startpos = 0;
        for (int i = 0; i < length; i++) {
            if ('\0' == mDataStr[i]) {
                String feild = mDataStr + startpos;
                size_t endpos = feild.find_first_of('=');
                if (string::npos != endpos) {
                    String key = feild.substr(0, endpos);
                    String value = feild.substr(endpos + 1);
                    if (0 == sections.count(key)) {
                        sections.insert(make_pair(key, value));
                    } else {
                        assert(false); // Key 以存在
                    }
                }

                startpos = i + 1;
            }
        }
        return sections;
    }
}
