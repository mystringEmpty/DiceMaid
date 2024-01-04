#pragma once
#include <string>
struct hash_ci {
	[[nodiscard]] size_t operator()(const std::string& str)const {
		size_t seed = 131;
		size_t hash = 0;
		for (char ch : str) {
			hash = hash * seed + tolower(static_cast<unsigned char>(ch));
		}
		return (hash & 0x7FFFFFFF);
	}
};
struct equal_ci
{
	bool operator()(const char& ch1, const char& ch2) const {
		return ((ch1 & ~(unsigned char)0xff) | tolower(static_cast<unsigned char>(ch1)))
			== ((ch2 & ~(unsigned char)0xff) | tolower(static_cast<unsigned char>(ch2)));
	}
	template<typename _Char>
	bool operator()(const _Char& ch1, const _Char& ch2) const {
		return tolower(static_cast<unsigned char>(ch1 & 0xff)) == tolower(static_cast<unsigned char>(ch2 & 0xff));
	}
	bool operator()(const std::string& str1, const std::string& str2) const {
		std::string::const_iterator it1 = str1.cbegin(), it2 = str2.cbegin();
		while (it1 != str1.cend() && it2 != str2.cend()) {
			if (tolower(static_cast<unsigned char>(*it2)) != tolower(static_cast<unsigned char>(*it1)))return false;
			++it1;
			++it2;
		}
		return str1.length() == str2.length();
	}
};