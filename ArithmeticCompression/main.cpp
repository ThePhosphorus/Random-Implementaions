#include <iostream>
#include <fstream>

#include "cachedRange.h"

static void binHexDisplay(const std::vector<uint8_t>& binary)
{
	std::cout << std::hex;
	for (uint8_t byte : binary)
	{
		std::cout << static_cast<size_t>(byte) << " ";
	}
	std::cout << std::dec << std::endl;
}


int main(int argc, char** argv)
{
	std::string textFilename;
	std::string outFilename;
	
	if (argc != 3)
	{
		std::cout << "Please enter the file containing your text" << std::endl;
		std::cin >> textFilename;
		std::cout << "Please enter the file where the compression will be appended" << std::endl;
		std::cin >> outFilename;
	}

	if (textFilename == "-d")
	{
		std::ifstream inFile(outFilename, std::ios::binary);

		if (!inFile.is_open())
		{
			std::cout << "Could not open " << outFilename << std::endl;
			return EXIT_FAILURE;
		}

		size_t nbChar = 0;
		size_t nbLetter = 0;
		size_t dumpSize = 0;
		std::vector<std::pair<char, size_t>> letterCount;

		inFile.read(reinterpret_cast<char*>(&nbChar), sizeof(nbChar));
		inFile.read(reinterpret_cast<char*>(&dumpSize), sizeof(dumpSize));
		inFile.read(reinterpret_cast<char*>(&nbLetter), sizeof(nbLetter));

		for (size_t i = 0; i < nbLetter; ++i)
		{
			std::pair<char, size_t> newPair;
			inFile.read(reinterpret_cast<char*>(&newPair.first), sizeof(newPair.first));
			inFile.read(reinterpret_cast<char*>(&newPair.second), sizeof(newPair.second));

			letterCount.push_back(newPair);
		}

		ArithmeticRange range;
		range.setRatios(nbChar, letterCount);

		std::vector<uint8_t> byteArray;
		for(size_t i = 0; i < dumpSize; ++i)
		{
			uint8_t letter = 0;
			inFile.read(reinterpret_cast<char*>(&letter), sizeof(letter));

			byteArray.push_back(letter);
		}

		std::string text = range.popString(byteArray, nbChar);

		std::cout << text << std::endl;
			   
	}
	else {
		std::ifstream textFile(textFilename);
		std::ofstream outFile(outFilename, std::ios::binary);

		if (!textFile.is_open())
		{
			std::cout << "Could not open " << textFilename << std::endl;
			return EXIT_FAILURE;
		}

		if (!outFile.is_open())
		{
			std::cout << "Could not open " << outFilename << std::endl;
			return EXIT_FAILURE;
		}

		std::string text;
		while (!textFile.eof())
		{
			const char letter = textFile.get();
			if (letter != EOF)
			{
				text.push_back(letter);
			}
		}

		ArithmeticRange range;
		range.setRatios(text);

		range.pushText(text);

		std::vector<uint8_t> dump = range.dumpBin();

		binHexDisplay(dump); // For Debug


		// Write number of character
		const size_t textSize = text.size();
		outFile.write(reinterpret_cast<const char*>(&textSize), sizeof(textSize));

		// write dump size
		const size_t dumpSize = dump.size();
		outFile.write(reinterpret_cast<const char*>(&dumpSize), sizeof(dumpSize));

		std::vector<std::pair<char, size_t>> letterCount = ArithmeticRange::getLetterCount(text);

		// write number of letters
		const size_t letterCountSize = letterCount.size();
		outFile.write(reinterpret_cast<const char*>(&letterCountSize), sizeof(letterCountSize));

		// Write the letter and the count
		for (const std::pair<char, size_t>& count : letterCount)
		{
			outFile.write(reinterpret_cast<const char*>(&count.first), sizeof(count.first));
			outFile.write(reinterpret_cast<const char*>(&count.second), sizeof(count.second));
		}

		// write bytes to file
		for (uint8_t byte : dump)
		{
			outFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
		}

		return EXIT_SUCCESS;
	}
}