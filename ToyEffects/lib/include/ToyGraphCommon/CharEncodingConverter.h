/*
    ×Ö·û´®±àÂë×ª»»Æ÷¡£
*/

#pragma once
#include "ToyGraphCommon/EngineCompileOptions.h"
#include <string>

#if TG_OS_IS_WINDOWS

class CharEncodingConverter {
public:
    static std::string utf8ToGb(const std::string& utf8);


private:
    CharEncodingConverter() {};
    CharEncodingConverter(const CharEncodingConverter&) {};
};

#endif
