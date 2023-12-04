/**
 * 字符编码转换
 * 将框架抽象后的部分
 */
#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define CP_GBK (54936)
#define PAGE_NATIVE CP_GBK
 //#define CP_UTF8 (65001)
#else
#define PAGE_NATIVE CP_UTF8
#endif // WIN32
#include <string>
using std::string;
using std::wstring;

inline bool isANSI(const char* chIn) {
	const char* p(chIn);
	while (*p) {
		if (*(p++) < 0)return false;
	}
	return true;
}

//multi->multi
inline string charcvtToNative(const char* chIn) {
	if (!chIn)return {};
#ifdef _WIN32
	if (!chIn[0] || isANSI(chIn))return chIn;
	const int UTF16len = MultiByteToWideChar(CP_UTF8, 0, chIn, -1, nullptr, 0);
	auto* const strUTF16 = new wchar_t[UTF16len];
	MultiByteToWideChar(CP_UTF8, 0, chIn, -1, strUTF16, UTF16len);
	const int lenOut = WideCharToMultiByte(CP_GBK, 0, strUTF16, -1, nullptr, 0, nullptr, nullptr);
	auto* const chOut = new char[lenOut];
	WideCharToMultiByte(CP_GBK, 0, strUTF16, -1, chOut, lenOut, nullptr, nullptr);
	std::string strNew(chOut);
	delete[] strUTF16;
	delete[] chOut;
	return strNew;
#else
	return chIn;
#endif // WIN32
};
inline string charcvtToNative(const string& strIn) {
	return charcvtToNative(strIn.c_str());
}
inline string charcvtFromNative(const char* chIn) {
	if (!chIn)return {};
#ifdef _WIN32
	if (!chIn[0] || isANSI(chIn))return chIn;
	const int UTF16len = MultiByteToWideChar(CP_GBK, 0, chIn, -1, nullptr, 0);
	auto* const strUTF16 = new wchar_t[UTF16len];
	MultiByteToWideChar(CP_GBK, 0, chIn, -1, strUTF16, UTF16len);
	const int lenOut = WideCharToMultiByte(CP_UTF8, 0, strUTF16, -1, nullptr, 0, nullptr, nullptr);
	auto* const chOut = new char[lenOut];
	WideCharToMultiByte(CP_UTF8, 0, strUTF16, -1, chOut, lenOut, nullptr, nullptr);
	std::string strNew(chOut);
	delete[] strUTF16;
	delete[] chOut;
	return strNew;
#else
	return chIn;
#endif // WIN32
};
inline string charcvtFromNative(const string& strIn) {
	return charcvtFromNative(strIn.c_str());
}

//宽字符->多字节
inline string charcvtToNative(const wchar_t* chIn) {
	if (!chIn)return {};
	const int lenOut = WideCharToMultiByte(PAGE_NATIVE, 0, chIn, -1, nullptr, 0, nullptr, nullptr);
	auto* const chOut = new char[lenOut];
	WideCharToMultiByte(PAGE_NATIVE, 0, chIn, -1, chOut, lenOut, nullptr, nullptr);
	std::string strNew(chOut);
	delete[] chOut;
	return strNew;
}
inline string charcvtToNative(const wstring& strIn) {
	return charcvtToNative(strIn.c_str());
}

//多字节->宽字符
template<UINT pageMB>
wstring charcvt(const char* chIn) {
	if (!chIn)return {};
	const int UTF16len = MultiByteToWideChar(pageMB, 0, chIn, -1, nullptr, 0);
	auto* const strUTF16 = new wchar_t[UTF16len];
	MultiByteToWideChar(pageMB, 0, chIn, -1, strUTF16, UTF16len);
	wstring wcsNew(strUTF16);
	delete[] strUTF16;
	return wcsNew;
}
template<UINT pageMB>
wstring charcvt(const string& strIn) {
	return charcvt<pageMB>(strIn.c_str());
}
inline string filterNonUtf8(string strUTF8) {
    size_t i = 0, len{ strUTF8.size() };
	int num = 0;
	while (i < strUTF8.length()) {
		if ((strUTF8[i] & 0x80) == 0x00) {
			i++;
			continue;
		}
		else if ((strUTF8[i] & 0xc0) == 0xc0 && (strUTF8[i] & 0xfe) != 0xfe) {
			// 110X_XXXX 10XX_XXXX
			// 1110_XXXX 10XX_XXXX 10XX_XXXX
			// 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
			// 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
			// 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
			unsigned char mask = 0x80;
			for (num = 0; num < 8; ++num) {
				if ((strUTF8[i] & mask) == mask) {
					mask = mask >> 1;
				}
				else
					break;
			}
			for (int j = 0; j < num - 1; j++) {
				if ((strUTF8[++i] & 0xc0) != 0x80) {
					strUTF8[i] = '?';
				}
			}
			++i;
		}
		else {
			strUTF8[i] = '?';
		}
	}
	return strUTF8;
}

inline bool isSpace(char ch) {
	return isspace(static_cast<unsigned char>(ch));
}
