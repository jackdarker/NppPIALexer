/*
 copy from https://github.com/tike/GOnpp/blob/master/GOnpp/StringUtils/WcharMbcsConverter.h
*/


#include "base.h"

class WcharMbcsConverter {
public:
	
	static std::vector<wchar_t> char2wchar(const char* mbStr){
		int len = ::MultiByteToWideChar(CP_UTF8, 0, mbStr, -1, NULL, 0);
		if (len <= 0) {
			return std::vector<wchar_t>(1, 0);
		}
		std::vector<wchar_t> buf(len);
		MultiByteToWideChar(CP_UTF8, 0, mbStr, -1, &buf[0], len);
		return buf;
	};
	static tstr char2wcharStr(const char* mbStr){
		int len = ::MultiByteToWideChar(CP_UTF8, 0, mbStr, -1, NULL, 0);
		if (len <= 0) {
			return tstr(_T(""));
		}
		tstr buf(len,0);
		MultiByteToWideChar(CP_UTF8, 0, mbStr, -1, &buf[0], len);
		buf.resize(len-1); //strip off 00
		return buf;
	};
	static wchar_t char2wchar(const char mbStr){
		std::vector<wchar_t> buf(1);
		MultiByteToWideChar(CP_UTF8, 0, &mbStr, -1, &buf[0], 1);
		return buf.at(0);
	};

	static std::vector<char>    wchar2char(const wchar_t* wcStr){
		int len = WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, NULL, 0, NULL, NULL);
		if (len <= 0) {
			return std::vector<char>(1, 0);
		}
		std::vector<char> buf(len);
		WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, &buf[0], len, NULL, NULL);
		return buf;
	};
	
	static str wchar2charStr(const wchar_t* wcStr){
		int len = WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, NULL, 0, NULL, NULL);
		if (len <= 0) {
			return str("");
		}
		str buf(len,0); 
		WideCharToMultiByte(CP_UTF8, 0, wcStr, -1, &buf[0], len, NULL, NULL);
		buf.resize(len-1); //strip off 00
		return buf;
	};

	static std::vector<TCHAR>   char2tchar(const char* mbStr){
		#ifdef _UNICODE
			return char2wchar(mbStr);
		#else
		#error not tested
			return std::vector<TCHAR>(mbStr, mbStr + strlen(mbStr) + 1);
		#endif
	};
	static std::vector<char>    tchar2char(const TCHAR* tStr){
		#ifdef _UNICODE
			return wchar2char(tStr);
		#else
		#error not tested
			return std::vector<TCHAR>(tStr, tStr + _tcslen(tStr) + 1);
		#endif
	};
};
