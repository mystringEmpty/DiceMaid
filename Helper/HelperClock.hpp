#pragma once
#include <string>
inline std::string printDuringSecond(long long seconds) {
    if (seconds < 0) {
        return "N/A";
    }
    else if (seconds < 60) {
        return std::to_string(seconds) + "秒";
    }
    int mins = int(seconds / 60);
    seconds = seconds % 60;
    if (mins < 60) {
        return std::to_string(mins) + "分" + std::to_string(seconds) + "秒";
    }
    int hours = mins / 60;
    mins = mins % 60;
    if (hours < 24) {
        return std::to_string(hours) + "小时" + std::to_string(mins) + "分" + std::to_string(seconds) + "秒";
    }
    int days = hours / 24;
    hours = hours % 24;
    return std::to_string(days) + "天" + std::to_string(hours) + "小时" + std::to_string(mins) + "分" + std::to_string(seconds) + "秒";
}
#pragma warning(disable:4996)
inline const char* _built_time() {
	static char tloc[20]("0000-00-00 " __TIME__);
	strncpy(tloc, __DATE__ + 7, 4);
	switch (__DATE__[0]) {
	case'J':
		if (__DATE__[1] == 'a')tloc[6] = '1';
		if (__DATE__[1] == 'u')tloc[6] = '6';
		break;
	case'F':
		if (__DATE__[1] == 'e')tloc[6] = '2';
		break;
	case'M':
		if (__DATE__[2] == 'r')tloc[6] = '3';
		else if (__DATE__[2] == 'y')tloc[6] = '5';
		break;
	case'A':
		if (__DATE__[1] == 'p')tloc[6] = '4';
		if (__DATE__[1] == 'u')tloc[6] = '8';
		break;
	case'S':
		tloc[6] = '9';
		break;
	case'O':
		strncpy(tloc + 5, "10", 2);
		break;
	case'N':
		strncpy(tloc + 5, "11", 2);
		break;
	case'D':
		strncpy(tloc + 5, "12", 2);
		break;
	}
	if (__DATE__[4] != ' ')tloc[8] = __DATE__[4];
	tloc[9] = __DATE__[5];
	return tloc;
}