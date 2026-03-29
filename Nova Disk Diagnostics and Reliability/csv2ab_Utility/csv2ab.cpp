// csv2ab.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <cctype>
#include <stdexcept>

struct WordEntry
{
    uint16_t address;
    uint16_t data;
};

static std::string Trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;

    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;

    return s.substr(start, end - start);
}

static void RemoveUtf8Bom(std::string& s)
{
    if (s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF)
    {
        s.erase(0, 3);
    }
}

static bool ParseOctalWord(const std::string& text, uint16_t& value)
{
    std::string s = Trim(text);
    if (s.empty())
        return false;

    for (char c : s)
    {
        if (c < '0' || c > '7')
            return false;
    }

    try
    {
        unsigned long n = std::stoul(s, nullptr, 8);
        if (n > 0xFFFF)
            return false;

        value = static_cast<uint16_t>(n);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

static bool LooksLikeHeader(const std::string& col1, const std::string& col2)
{
    uint16_t dummy = 0;
    return !ParseOctalWord(col1, dummy) || !ParseOctalWord(col2, dummy);
}

static bool ReadCsvObjectFile(
    const std::string& filename,
    std::vector<WordEntry>& entries,
    bool& haveStartAddress,
    uint16_t& startAddress)
{
    std::ifstream file(filename);
    if (!file)
        return false;

    entries.clear();
    haveStartAddress = false;
    startAddress = 0;

    std::string line;
    size_t lineNumber = 0;
    bool firstNonEmptyLine = true;

    while (std::getline(file, line))
    {
        ++lineNumber;

        if (lineNumber == 1)
            RemoveUtf8Bom(line);

        line = Trim(line);

        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string col1, col2;

        if (!std::getline(ss, col1, ','))
        {
            std::cerr << "Invalid CSV at line " << lineNumber << "\n";
            return false;
        }

        if (!std::getline(ss, col2))
        {
            std::cerr << "Missing second column at line " << lineNumber << "\n";
            return false;
        }

        col1 = Trim(col1);
        col2 = Trim(col2);

        if (firstNonEmptyLine && LooksLikeHeader(col1, col2))
        {
            firstNonEmptyLine = false;
            continue;
        }

        firstNonEmptyLine = false;

        uint16_t address = 0;
        uint16_t data = 0;

        if (!ParseOctalWord(col1, address))
        {
            std::cerr << "Invalid octal address at line " << lineNumber << ": " << col1 << "\n";
            return false;
        }

        if (!ParseOctalWord(col2, data))
        {
            std::cerr << "Invalid octal data at line " << lineNumber << ": " << col2 << "\n";
            return false;
        }

        if (address == 0177777)
        {
            haveStartAddress = true;
            startAddress = data;
        }
        else
        {
            entries.push_back({ address, data });
        }
    }

    return true;
}

static void AppendWordLE(std::vector<uint8_t>& out, uint16_t word)
{
    out.push_back(static_cast<uint8_t>(word & 0x00FF));
    out.push_back(static_cast<uint8_t>((word >> 8) & 0x00FF));
}

static uint16_t ComputeChecksum(const std::vector<uint16_t>& wordsWithoutChecksum)
{
    uint32_t sum = 0;
    for (uint16_t w : wordsWithoutChecksum)
        sum += w;

    uint16_t sum16 = static_cast<uint16_t>(sum & 0xFFFF);
    return static_cast<uint16_t>(0 - sum16);
}

static void AppendBlockSeparator(std::vector<uint8_t>& out)
{
    out.push_back(0);
    out.push_back(0);
    out.push_back(0);
    out.push_back(0);
}

static void AppendDataBlock(
    std::vector<uint8_t>& out,
    uint16_t loadAddress,
    const std::vector<uint16_t>& dataWords)
{
    if (dataWords.size() > 16)
        throw std::runtime_error("Internal error: data block exceeds 16 words.");

    uint16_t countWord = static_cast<uint16_t>(
        0 - static_cast<uint16_t>(dataWords.size()));

    std::vector<uint16_t> blockWords;
    blockWords.push_back(countWord);
    blockWords.push_back(loadAddress);
    blockWords.push_back(0);
    blockWords.insert(blockWords.end(), dataWords.begin(), dataWords.end());

    blockWords[2] = ComputeChecksum(blockWords);

    AppendBlockSeparator(out);

    for (uint16_t w : blockWords)
        AppendWordLE(out, w);
}

static void AppendStartBlock(std::vector<uint8_t>& out, uint16_t startAddress)
{
    std::vector<uint16_t> blockWords;
    blockWords.push_back(1);
    blockWords.push_back(startAddress);
    blockWords.push_back(0);

    blockWords[2] = ComputeChecksum(blockWords);

    AppendBlockSeparator(out);

    for (uint16_t w : blockWords)
        AppendWordLE(out, w);
}

static std::vector<uint8_t> EncodeAbsoluteBinary(
    std::vector<WordEntry> entries,
    bool haveStartAddress,
    uint16_t startAddress)
{
    std::vector<uint8_t> out;

    if (entries.empty() && !haveStartAddress)
        return out;

    std::sort(entries.begin(), entries.end(),
        [](const WordEntry& a, const WordEntry& b)
        {
            return a.address < b.address;
        });

    for (size_t i = 1; i < entries.size(); ++i)
    {
        if (entries[i].address == entries[i - 1].address)
            throw std::runtime_error("Duplicate address found in CSV input.");
    }

    size_t i = 0;
    while (i < entries.size())
    {
        uint16_t runStartAddress = entries[i].address;
        std::vector<uint16_t> runData;
        runData.push_back(entries[i].data);

        size_t j = i + 1;
        while (j < entries.size() &&
            entries[j].address == static_cast<uint16_t>(entries[j - 1].address + 1))
        {
            runData.push_back(entries[j].data);
            ++j;
        }

        size_t offset = 0;
        while (offset < runData.size())
        {
            size_t chunkSize = std::min<size_t>(16, runData.size() - offset);

            std::vector<uint16_t> blockData(
                runData.begin() + static_cast<std::ptrdiff_t>(offset),
                runData.begin() + static_cast<std::ptrdiff_t>(offset + chunkSize));

            uint16_t blockAddress = static_cast<uint16_t>(runStartAddress + offset);
            AppendDataBlock(out, blockAddress, blockData);

            offset += chunkSize;
        }

        i = j;
    }

    if (haveStartAddress)
        AppendStartBlock(out, startAddress);

    return out;
}

static bool WriteBinaryFile(const std::string& filename, const std::vector<uint8_t>& bytes)
{
    std::ofstream out(filename, std::ios::binary);
    if (!out)
        return false;

    if (!bytes.empty())
        out.write(reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));

    return out.good();
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: nova_csv_to_abs <input.csv> <output.bin>\n";
        return 1;
    }

    const std::string inputFile = argv[1];
    const std::string outputFile = argv[2];

    std::vector<WordEntry> entries;
    bool haveStartAddress = false;
    uint16_t startAddress = 0;

    if (!ReadCsvObjectFile(inputFile, entries, haveStartAddress, startAddress))
    {
        std::cerr << "Failed to read CSV file: " << inputFile << "\n";
        return 1;
    }

    if (entries.empty() && !haveStartAddress)
    {
        std::cerr << "Input CSV contains no load data and no start address.\n";
        return 1;
    }

    try
    {
        std::vector<uint8_t> absBinary =
            EncodeAbsoluteBinary(entries, haveStartAddress, startAddress);

        if (!WriteBinaryFile(outputFile, absBinary))
        {
            std::cerr << "Failed to write output file: " << outputFile << "\n";
            return 1;
        }

        std::cout << "Read " << entries.size() << " data words from " << inputFile << "\n";
        if (haveStartAddress)
        {
            std::cout << "Start address: "
                << std::oct << startAddress << std::dec << "\n";
        }
        else
        {
            std::cout << "No start address record present.\n";
        }

        std::cout << "Wrote " << absBinary.size() << " bytes to " << outputFile << "\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
