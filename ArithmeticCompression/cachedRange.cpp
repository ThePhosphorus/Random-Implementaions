#include "cachedRange.h"

#include <algorithm>
#include <cmath>

constexpr auto BYTE_SIZE = 8;

static void fillByteArrFromBit(std::vector<uint8_t>& ByteArray, const std::vector<bool> bitArray, size_t nbBytes)
{
	for (size_t i = 0; i < nbBytes; ++i)
	{
		uint8_t byte = 0;
		for (size_t j = 0; j < BYTE_SIZE; ++j)
		{
			byte |= bitArray[i * BYTE_SIZE + j] << (BYTE_SIZE - j - 1);
		}
		ByteArray.push_back(byte);
	}
}

bool CachedRange::pushCache()
{
	const double doubledMin = _currentMin * 2.;
	const double doubledMax = _currentMax * 2.;

	// Check if both numbers have the same starting bit
	if (std::floorl(doubledMin) == std::floorl(doubledMax))
	{
		// cache the bit and center the range
		const bool biggerThanHalf = std::floor(doubledMin);
		_cache.push_back(biggerThanHalf);
		_currentMax = doubledMax - static_cast<double>(biggerThanHalf);
		_currentMin = doubledMin - static_cast<double>(biggerThanHalf);

		return true;
	}
	else
	{
		return false;
	}
}

void ArithmeticRange::setRatios(const size_t numCharacters, std::vector<std::pair<char, size_t>> letterCount)
{
	// sort the vector with size
	std::sort(letterCount.begin(), letterCount.end(), [](const std::pair<char, size_t>& lhs, const std::pair<char, size_t>& rhs)
		{
			return lhs.second > rhs.second;
		});

	// generate ratios
	double accumulator = 0.;

	for (const std::pair<char, size_t>& count: letterCount)
	{
		std::pair<double, double> letterRange;
		letterRange.first = accumulator;
		letterRange.second = accumulator += (static_cast<double>(count.second)/numCharacters);

		_letterRange.insert(std::make_pair(count.first, letterRange));
	}
}

void ArithmeticRange::setRatios(const std::string& text)
{
	setRatios(text.size(), getLetterCount(text));
}

void ArithmeticRange::pushLetter(const char letter)
{
	const auto rangeIt = _letterRange.find(letter);
	if (rangeIt == _letterRange.end())
	{
		// Letter not it the text, so we just skip it
		return;
	}

	const std::pair<double, double> letterRange = rangeIt->second;

	// cache what we can
	while (pushCache());

	const double span = _currentMax - _currentMin;
	const double relativeMin = span * letterRange.first;
	const double relativeMax = span * letterRange.second;

	_currentMax = _currentMin + relativeMax;
	_currentMin += relativeMin;

}

char ArithmeticRange::popLetter(const double number)
{
	const double span = (_currentMax - _currentMin);
	const double relativeNumber = (number - _currentMin) / span;

	for (const std::pair<char, std::pair<double, double>>& letterRange : _letterRange)
	{
		if (relativeNumber >= letterRange.second.first && relativeNumber <= letterRange.second.second)
		{
			_currentMax = letterRange.second.second * span + _currentMin;
			_currentMin += letterRange.second.first* span;
			return letterRange.first;
		}
	}
}

std::string ArithmeticRange::popString(const std::vector<uint8_t>& byteArray, const size_t numChar)
{
	// Create the bit Array and fill it
	std::vector<bool> bitArray;

	for (uint8_t byte : byteArray)
	{
		for (int i = 0; i < BYTE_SIZE; ++i)
		{
			bitArray.push_back(byte & 0b10000000);
			byte <<= 1;
		}
	}
	
	constexpr size_t precision = 32;
	std::string output;

	// Build initial floating point value
	double value = 0;
	size_t index = 0;

	for (size_t i = 0; i < precision; ++i)
	{
		value += bitArray[index++] * std::exp2(-static_cast<double>(i + 1));
	}

	for (size_t i = 0; i < numChar; ++i)
	{
		output.push_back(popLetter(value));
		while (pushCache())
		{
			const bool BiggerThanHalf = value > 0.5;
			value -= (BiggerThanHalf) ? 0.5 : 0.;
			value *= 2;
			value += bitArray[index++] * std::exp2(-static_cast<double>(precision));
		}
	}

	return output;

}

void ArithmeticRange::pushText(const std::string& text)
{
	// push each letter of the text
	for (char letter : text)
	{
		pushLetter(letter);
	}
}

std::vector<uint8_t> ArithmeticRange::dumpBin() const
{
	std::vector<uint8_t> byteVec;

	const size_t cacheSize = _cache.size();

	// Start by taken out all possible Bytes
	const size_t initialNumBytes = cacheSize / BYTE_SIZE;
	fillByteArrFromBit(byteVec, _cache, initialNumBytes);

	// Check How many bits we have left
	const size_t cacheLeftOversSize = cacheSize % 8;
	
	std::vector<bool> lastbits;
	// add our last bits in our new array
	for (size_t i = 0; i < cacheLeftOversSize; ++i)
	{
		lastbits.push_back(_cache[initialNumBytes * BYTE_SIZE + i]);
	}

	// Get the middle of our range
	double midRange = (_currentMax + _currentMin) / 2.;

	// Cache all bit from this number
	while (midRange != 0.)
	{
		const bool cachedBit = midRange >= 0.5;
		lastbits.push_back(cachedBit);
		midRange -= (cachedBit) ? 0.5 : 0;
		midRange *= 2;
	}

	const size_t nbZerosToAdd = std::ceil(static_cast<float>(lastbits.size()) / BYTE_SIZE) * BYTE_SIZE - lastbits.size();

	for (size_t i = 0; i < nbZerosToAdd; ++i) {
		lastbits.push_back(false);
	}

	fillByteArrFromBit(byteVec, lastbits, lastbits.size() / BYTE_SIZE);

	return byteVec;
}

std::vector<std::pair<char, size_t>> ArithmeticRange::getLetterCount(const std::string& text)
{
	std::map<char, size_t> countMap;

	for (char letter : text)
	{
		countMap[letter] ++;
	}

	std::vector<std::pair<char, size_t>> letterCount;
	letterCount.reserve(countMap.size());
	for (const std::pair<char, size_t>& count : countMap)
	{
		letterCount.push_back(count);
	}

	return letterCount;
}
