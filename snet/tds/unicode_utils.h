#ifndef NANODBC_UNICODE_UTILS_H
#define NANODBC_UNICODE_UTILS_H

#include "nanodbc.h"

#include <codecvt>
#include <locale>
#include <string>

#ifdef NANODBC_USE_UNICODE
    #undef NANODBC_TEXT
    #define NANODBC_TEXT(s) u ## s

    inline nanodbc::string_type convert(std::string const& in)
    {
        std::u16string out;
        // Workaround for confirmed bug in VS2015.
        // See: https://social.msdn.microsoft.com/Forums/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error
        #if defined(_MSC_VER) && (_MSC_VER == 1900)
            auto s = std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t>().from_bytes(in);
            auto p = reinterpret_cast<char16_t const*>(s.data());
            out.assign(p, p + s.size());
        #else
            out = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().from_bytes(in);
        #endif
        return out;
    }

    inline std::string convert(nanodbc::string_type const& in)
    {
        std::string out;
        // See above for details about this workaround.
        #if defined(_MSC_VER) && (_MSC_VER == 1900)
            std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
            auto p = reinterpret_cast<const int16_t *>(in.data());
            out = convert.to_bytes(p, p + in.size());
        #else
            out = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>().to_bytes(in);
        #endif
        return out;
    }
#else
    #undef NANODBC_TEXT
    #define NANODBC_TEXT(s) s

    inline nanodbc::string_type convert(std::string const& in)
    {
        return in;
    }
#endif

template <typename T>
inline std::string any_to_string(T const& t)
{
    return std::to_string(t);
}

template <>
inline std::string any_to_string<nanodbc::string_type>(nanodbc::string_type const& t)
{
    return convert(t);
}

std::vector<std::string> split_string(const std::string& src,
                                      const std::string& delim,
                                      size_t maxParts = size_t(-1)) {
    if(maxParts == 0) {
        maxParts = size_t(-1);
    }
    size_t lastPos = 0;
    size_t pos = 0;
    size_t size = src.size();

    std::vector<std::string> tokens;

    while(pos < size) {
        pos = lastPos;
        while(pos < size && delim.find_first_of(src[pos]) == std::string::npos) {
            ++pos;
        }

        if(pos - lastPos > 0) {
            if(tokens.size() == maxParts - 1) {
                tokens.push_back(src.substr(lastPos));
                break;
            }
            else {
                tokens.push_back(src.substr(lastPos, pos - lastPos));
            }
        }

        lastPos = pos + 1;
    }

    return tokens;
}

std::string GBKToUTF8(const char* strGBK) {
#ifdef _WIN32
    int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len+1];
    memset(wstr, 0, len+1);
    MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len+1];
    memset(str, 0, len+1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    std::string strTemp = str;
    if(wstr) delete[] wstr;
    if(str) delete[] str;
    return strTemp;
#else
    return "";
#endif
}

std::string UTF8ToGBK(const char* strUTF8) {
#ifdef _WIN32
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len+1];
    memset(wszGBK, 0, len*2+2);
    MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len+1];
    memset(szGBK, 0, len+1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    std::string strTemp(szGBK);
    if(wszGBK) delete[] wszGBK;
    if(szGBK) delete[] szGBK;
    return strTemp;
#else
    return "";
#endif
}

#endif // NANODBC_UNICODE_UTILS_H
