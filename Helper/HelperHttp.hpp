/******************************************************************************
 *                                                                            *
 *  @file     HelperHttp.h                                                    *
 *  @brief    网络连接工具库，用于发送POST/GET请求                            *
 *  Details.                                                                  *
 *                                                                            *
 *  @author   Shiki                                                           *
 *  @email    mystring.Empty@gmail.com                                        *
 *  @version  1.0.0(版本号)                                                   *
 *  @date     2020/07/15                                                      *
 *  @license  GNU General Public License (GPL)                                *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *  Remark         : retcription                                              *
 *----------------------------------------------------------------------------*
 *  Change History :                                                          *
 *  <Date>     | <Version> | <Author>       | <retcription>                   *
 *----------------------------------------------------------------------------*
 *  2020/07/15 |   1.0.0   |  Shiki         | Create file                     *
 ******************************************************************************/
#pragma once
#include <string>
#include <string_view>
#include "AppInfo.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinInet.h>
#pragma comment(lib, "WinInet.lib")
#else
#include <curl/curl.h>
#endif

using std::string;
using strview = std::string_view;

inline unsigned char ToHex(const unsigned char x){
	return x > 9 ? x + 55 : x + 48;
}
inline unsigned char FromHex(const unsigned char x){
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else y = x - '0';
	return y;
}

/**
 * @brief 处理http请求的类
 **/
#ifdef _WIN32
class Http {
	string server;
	int port{ 443 };
	const HINTERNET hInternet = InternetOpenA(app_name, INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	const HINTERNET hConnect = InternetConnectA(hInternet, server.data(), port, nullptr, nullptr, INTERNET_SERVICE_HTTP, INTERNET_FLAG_KEEP_CONNECTION, 0);
	bool res = false;
public:
	enum class Errno { NIL, Unknown, QueryErr, RetCodeErr, NoRespon };
	Errno err{ Errno::NIL };
	string ret;
	bool Get(strview nameObject) {
		static const char* acceptTypes[] = { "*/*", nullptr };
		HINTERNET hRequest{ HttpOpenRequestA(hConnect, "GET", nameObject.data(), "HTTP/1.1", 
			nullptr, acceptTypes, INTERNET_FLAG_NO_CACHE_WRITE, 0) };
		res = HttpSendRequestA(hRequest, nullptr, 0, nullptr, 0);
		bool isError{ false };
		if (res) {
			DWORD dwRetCode = 0;
			DWORD dwBufferLength = sizeof(dwRetCode);
			if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwRetCode, &dwBufferLength, nullptr)) {
				err = Errno::QueryErr;
				ret = "QueryErr!" + getLastErrMsg();
				isError = true;
			}
			else if (dwRetCode != 200) {
				err = Errno::RetCodeErr;
				ret = std::to_string(dwRetCode);
				isError = true;
			}
			else if (DWORD preRcvCnt; !InternetQueryDataAvailable(hRequest, &preRcvCnt, 0, 0)) {
				ret = "InternetQueryDataUnAvailable!" + getLastErrMsg();
				isError = true;
			}
			else if (preRcvCnt == 0) {
				err = Errno::NoRespon;
				ret = "NoRespon";
				isError = true;
			}
			else {
				std::string finalRcvData;
				while (preRcvCnt) {
					char* rcvData = new char[preRcvCnt];
					DWORD rcvCnt;
					if (!InternetReadFile(hRequest, rcvData, preRcvCnt, &rcvCnt)) {
						ret = "InternetReadFileFail!" + getLastErrMsg();
						delete[] rcvData;
						isError = true;
						break;
					}
					if (rcvCnt != preRcvCnt) {
						err = Errno::Unknown;
						ret = "Unknown";
						delete[] rcvData;
						isError = true;
						break;
					}
					finalRcvData += std::string(rcvData, rcvCnt);
					if (!InternetQueryDataAvailable(hRequest, &preRcvCnt, 0, 0)) {
						ret = "InternetQueryDataAvailable!" + getLastErrMsg();
						delete[] rcvData;
						isError = true;
						break;
					}
					delete[] rcvData;
				}
				if(!isError)ret = finalRcvData;
			}
		}
		else {
			ret = "NoRes";
			isError = true;
		}
		InternetCloseHandle(hRequest);
		return !isError;
	}
	bool Post(strview nameObject, char* const frmdata) {
		const char* acceptTypes[] = { "*/*", nullptr };
		const char* header = "Content-Type: application/x-www-form-urlencoded";
		const HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", nameObject.data(), "HTTP/1.1", nullptr, acceptTypes, port == 443 ? INTERNET_FLAG_SECURE : 0, 0);
		const BOOL res = HttpSendRequestA(hRequest, header, strlen(header), frmdata, strlen(frmdata));

		bool isError{ false };
		if (res) {
			DWORD dwRetCode = 0;
			DWORD dwBufferLength = sizeof(dwRetCode);
			if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwRetCode, &dwBufferLength, nullptr)) {
				err = Errno::QueryErr;
				ret = "QueryErr!" + getLastErrMsg();
				isError = true;
			}
			else if (dwRetCode != 200) {
				err = Errno::RetCodeErr;
				ret = std::to_string(dwRetCode);
				isError = true;
			}
			else if (DWORD preRcvCnt; !InternetQueryDataAvailable(hRequest, &preRcvCnt, 0, 0)) {
				ret = "InternetQueryDataUnAvailable!" + getLastErrMsg();
				isError = true;
			}
			else if (preRcvCnt == 0) {
				err = Errno::NoRespon;
				ret = "NoRespon";
				isError = true;
			}
			else {
				std::string finalRcvData;
				while (preRcvCnt) {
					char* rcvData = new char[preRcvCnt];
					DWORD rcvCnt;

					if (!InternetReadFile(hRequest, rcvData, preRcvCnt, &rcvCnt)) {
						ret = "InternetReadFileFail!" + getLastErrMsg();
						delete[] rcvData;
						isError = true;
						break;
					}
					if (rcvCnt != preRcvCnt) {
						err = Errno::Unknown;
						ret = "Unknown";
						delete[] rcvData;
						isError = true;
						break;
					}
					finalRcvData += std::string(rcvData, rcvCnt);
					if (!InternetQueryDataAvailable(hRequest, &preRcvCnt, 0, 0)) {
						ret = "InternetQueryDataAvailable!" + getLastErrMsg();
						delete[] rcvData;
						isError = true;
						break;
					}
					delete[] rcvData;
				}
				if (!isError)ret = finalRcvData;
			}
		}
		else {
			ret = "NoRes";
			isError = true;
		}
		InternetCloseHandle(hRequest);
		return !isError;
	}
	string getLastErrMsg() {

		DWORD dwError = GetLastError();
		if (dwError == ERROR_INTERNET_EXTENDED_ERROR) {
			DWORD size = 512;
			char* szFormatBuffer = new char[size];
			if (InternetGetLastResponseInfoA(&dwError, szFormatBuffer, &size)) {
				std::string ret(szFormatBuffer);
				while (ret[ret.length() - 1] == '\n' || ret[ret.length() - 1] == '\r')ret.erase(ret.length() - 1);
				delete[] szFormatBuffer;
				return ret;
			}
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				delete[] szFormatBuffer;
				szFormatBuffer = new char[size];
				if (InternetGetLastResponseInfoA(&dwError, szFormatBuffer, &size)) {
					std::string ret(szFormatBuffer);
					while (ret[ret.length() - 1] == '\n' || ret[ret.length() - 1] == '\r')ret.erase(ret.length() - 1);
					delete[] szFormatBuffer;
					return ret;
				}
				delete[] szFormatBuffer;
				return "";

			}
			delete[] szFormatBuffer;
			return "";
		}
		char szFormatBuffer[512];
		const DWORD dwBaseLength = FormatMessageA(
			FORMAT_MESSAGE_FROM_HMODULE,             // dwFlags
			GetModuleHandleA("wininet.dll"),         // lpSource
			dwError,                                 // dwMessageId
			0,                                       // dwLanguageId
			szFormatBuffer,                          // lpBuffer
			sizeof(szFormatBuffer),                  // nSize
			nullptr);
		if (dwBaseLength) {
			std::string ret(szFormatBuffer);
			while (ret[ret.length() - 1] == '\n' || ret[ret.length() - 1] == '\r')ret.erase(ret.length() - 1);
			return ret;
		}
		return "";
	}
	Http(const string& serverName, int port = 443) :server(serverName), port(port) {

	}
	~Http() {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
	}
	static std::string UrlEncode(const std::string& str)
	{
		std::string strTemp;
		const size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum(static_cast<unsigned char>(str[i])) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex(static_cast<unsigned char>(str[i]) >> 4);
				strTemp += ToHex(static_cast<unsigned char>(str[i]) % 16);
			}
		}
		return strTemp;
	}

	static std::string UrlDecode(const std::string& str)
	{
		std::string strTemp;
		const size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				//assert(i + 2 < length);
				const unsigned char high = FromHex(static_cast<unsigned char>(str[++i]));
				const unsigned char low = FromHex(static_cast<unsigned char>(str[++i]));
				strTemp += static_cast<char>(high * 16 + low);
			}
			else strTemp += str[i];
		}
		return strTemp;
	}
};
#else

size_t curlWriteToString(void* contents, size_t size, size_t nmemb, std::string* s)
{
	size_t newLength = size * nmemb;
	s->append((char*)contents, newLength);
	return newLength;
}

class SKHttp
{
	string server;
	int port;
	CURLcode lastError;
public:
	enum class Errno { NIL, Unknown, QueryErr, RetCodeErr, NoRespon };
	Errno err{ Errno::NIL };
	string ret;
	bool Get(strview nameObject) {
		CURL* curl;
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, (std::string("http://") + server + ":" + std::to_string(port) + nameObject.data()).c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteToString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);

			lastError = curl_easy_perform(curl);
			if (lastError != CURLE_OK)
			{
				ret = getLastErrorMsg();
				err = Errno::Unknown;
			}

			curl_easy_cleanup(curl);
			return lastError == CURLE_OK;
		}
		return false;
	}

	bool Post(strview nameObject, char* const frmdata)
	{
		CURL* curl;
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, (std::string("http://") + server + ":" + std::to_string(port) + nameObject.data()).c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, frmdata);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteToString);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);

			lastError = curl_easy_perform(curl);
			if (lastError != CURLE_OK)
			{
				ret = getLastErrorMsg();
				err = Errno::Unknown;
			}

			curl_easy_cleanup(curl);
			return lastError == CURLE_OK;
		}
		return false;
	}

	std::string getLastErrorMsg()
	{
		return curl_easy_strerror(lastError);
	}
	SKHttp(const char* serverName = "shiki.stringempty.xyz", int port = 80) :server(serverName), port(port) {

	}
	~SKHttp() {

	}
};
#endif