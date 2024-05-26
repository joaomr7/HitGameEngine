#pragma once

#include "Core/Types.h"

#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <vector>

namespace hit
{
	static std::string str_trim(std::string_view str)
	{
		auto start = str.find_first_not_of(" \t\n\r");
		if(start == std::string::npos)
		{
			return "";
		}

		auto end = str.find_last_not_of(" \t\n\r");
		return std::string(str.substr(start, end - start + 1));
	}

	static bool str_insensitive_compare(std::string_view a, std::string_view b)
	{
		return std::ranges::equal(a, b, [](char a, char b) { return std::tolower(a) == std::tolower(b); });
	}

	static bool str_match(std::string_view str, const std::vector<std::string_view>& matches)
	{
		for(auto match : matches)
		{
			if(str == match)
			{
				return true;
			}
		}

		return false;
	}

	static bool str_insensitive_match(std::string_view str, const std::vector<std::string_view>& matches)
	{
		for(auto match : matches)
		{
			if(str_insensitive_compare(str, match))
			{
				return true;
			}
		}

		return false;
	}

	inline i64 str_to_i64(const std::string& str)
	{
		return std::stoll(str);
	}

	inline ui64 str_to_ui64(const std::string& str)
	{
		return std::stoull(str);
	}

	inline f32 str_to_f32(const std::string& str)
	{
		return std::stof(str);
	}

	inline f64 str_to_f64(const std::string& str)
	{
		return std::stod(str);
	}
}