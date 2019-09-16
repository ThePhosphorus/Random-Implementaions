#pragma once

#include <vector>
#include <map>
#include <string>

class CachedRange
{
public:
	// Private Methodes
	/// Try to cache the range, returns if a chaching was done
	bool pushCache();

protected:
	// Private members
	/// Minimal Bound
	double _currentMin{ 0. };
	/// Maximum Bound
	double _currentMax{ 1. };

	/// Cache
	std::vector<bool> _cache;
};

class ArithmeticRange : public CachedRange
{
public:
	/// Set Letter Ratios ( input number of characters in the text, and the number of occurence of each letter)
	void setRatios(const size_t numCharacters, std::vector<std::pair<char, size_t>> letterCount);
	// Set letter ratios from a string
	void setRatios(const std::string& text);
	// Use Algoritmic compression for the range
	void pushLetter(const char letter);

	void pushText(const std::string& text);

	char popLetter(const double number);

	std::string popString(const std::vector<uint8_t>& byteArray, const size_t numChar);

	std::vector<uint8_t> dumpBin() const;

	static std::vector<std::pair<char, size_t>> getLetterCount(const std::string& text);

private:
	// Private members
	std::map<char, std::pair<double, double>> _letterRange;
};
