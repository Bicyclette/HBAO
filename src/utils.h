#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstdint>
#include <array>
#include <random>

inline std::string file2String(std::string const & iFile)
{
	std::ifstream fileStream(iFile.c_str());
	if (fileStream.is_open())
	{
		std::stringstream buffer;
		buffer << fileStream.rdbuf();
		return buffer.str();
	}
	else
	{
		std::cerr << "Error: failed opening file \"" << iFile << "\"" << std::endl;
		std::exit(-1);
	}
}

inline float gen_random_float(float const & iFrom, float const & iTo)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> distribution(iFrom, iTo);
	return distribution(mt);
}

inline int gen_random_int(int const& iFrom, int const& iTo)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> distribution(iFrom, iTo);
	return distribution(mt);
}

inline float lerp(float const& iA, float const& iB, float const & iRatio)
{
	return (1.0f - iRatio) * iA + iRatio * iB;
}