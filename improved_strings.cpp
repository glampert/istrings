
// ================================================================================================
// -*- C++ -*-
// File: improved_strings.cpp
// Author: Guilherme R. Lampert
// Created on: 07/12/15
//
// Brief: Simple tool that replaces the standard 'strings' utility.
//
// This source code is released under the MIT license.
// See the accompanying LICENSE file for details.
//
// ================================================================================================

//
// c++ -std=c++11 -O2 -Wall -Wextra -pedantic improved_strings.cpp -o istrings
//
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>

static inline bool isASCII(int chr)
{
    return chr >= 0 && chr <= 127;
}

static inline bool isPrint(int chr)
{
    return chr >= 32 && chr <= 126;
}

static inline bool isLetter(int chr)
{
    return (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z');
}

static std::vector<char> loadFileContents(const char * filename)
{
    FILE * fileIn = nullptr;
    std::vector<char> fileContents;

    #ifdef _MSC_VER
    fopen_s(&fileIn, filename, "rb");
    #else // _MSC_VER
    fileIn = std::fopen(filename, "rb");
    #endif // _MSC_VER

    if (fileIn == nullptr)
    {
        std::cerr << "Failed to open \"" << filename << "\": " << std::strerror(errno) << std::endl;
        return fileContents;
    }

    std::fseek(fileIn, 0, SEEK_END);
    const auto fileLength = std::ftell(fileIn);
    std::fseek(fileIn, 0, SEEK_SET);

    if (fileLength <= 0 || std::ferror(fileIn))
    {
        std::cerr << "Error getting length or empty file!" << std::endl;
        std::fclose(fileIn);
        return fileContents;
    }

    fileContents.resize(fileLength);
    if (std::fread(fileContents.data(), 1, fileLength, fileIn) != std::size_t(fileLength))
    {
        std::cerr << "WARNING! Failed to read whole file \"" << filename << "\"." << std::endl;
    }

    std::fclose(fileIn);
    return fileContents;
}

static int countLargestLetterSequence(const std::string & str)
{
    int current = 0;
    std::vector<int> sequences;

    for (const char chr : str)
    {
        if (!isLetter(chr) && chr != '_')
        {
            if (current > 0)
            {
                sequences.push_back(current);
            }
            current = 0;
            continue;
        }
        ++current;
    }

    if (current > 0)
    {
        sequences.push_back(current);
    }

    auto maxIter = std::max_element(std::begin(sequences), std::end(sequences));
    return (maxIter != std::end(sequences)) ? *maxIter : 0;
}

static bool acceptString(const std::string & str, int minSequence)
{
    if (countLargestLetterSequence(str) >= minSequence)
    {
        return true;
    }
    return false;
}

static void printHelpText(const char * progName)
{
    std::cout << "\n"
        << "Usage:\n"
        << " $ " << progName << " <input-file> [output-file] [options]\n"
        << " Tries to find printable strings inside a binary file.\n"
        << " If no output file is provided output is printed to stdout.\n"
        << " Options are:\n"
        << "  -h, --help  Prints this message and exits.\n"
        << "  --min=<N>   Minimum sequence of letters (aA-zZ) for a string to be considered. Defaults to 4.\n"
        << "\n"
        << "Created by Guilherme R. Lampert, " << __DATE__ << ".\n";
}

int main(int argc, const char * argv[])
{
    if (argc <= 1)
    {
        printHelpText(argv[0]);
        return EXIT_FAILURE;
    }

    if (argv[1][0] == '-')
    {
        if (std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0)
        {
            printHelpText(argv[0]);
            return EXIT_SUCCESS;
        }
    }

    const std::string inFileName{ argv[1] };

    // Check for a flag in the wrong place/empty string...
    if (inFileName.empty() || inFileName[0] == '-')
    {
        std::cerr << "Invalid filename \"" << inFileName << "\"!" << std::endl;
        return EXIT_FAILURE;
    }

    const auto fileContents = loadFileContents(inFileName.c_str());
    if (fileContents.empty())
    {
        return EXIT_FAILURE;
    }

    std::ofstream outFile;
    std::ostream * pOut = &outFile;

    if (argc >= 3 && argv[2][0] != '-') // Output name provided?
    {
        outFile.open(argv[2]);
        if (!outFile.is_open() || !outFile.good())
        {
            std::cerr << "Problems opening output file!" << std::endl;
            return EXIT_FAILURE;
        }
    }
    else // Use stdout:
    {
        pOut = &std::cout;
    }

    // Additional flags:
    int minSequence = 4;
    if (argc >= 3)
    {
        int tmp;
        if (argc == 3 && argv[2][0] == '-')
        {
            if (std::sscanf(argv[2], "--min=%d", &tmp) == 1)
            {
                minSequence = tmp;
            }
        }
        else if (argc == 4 && argv[3][0] == '-')
        {
            if (std::sscanf(argv[3], "--min=%d", &tmp) == 1)
            {
                minSequence = tmp;
            }
        }
    }

    //
    // First pass, look for potential matches:
    //
    std::string potentialMatch;
    std::vector<std::string> matches;
    for (const char chr : fileContents)
    {
        if (chr == '\0' || chr == '\n' || chr == '\r' || !isASCII(chr) || !isPrint(chr))
        {
            if (!potentialMatch.empty())
            {
                matches.emplace_back(std::move(potentialMatch));
                potentialMatch.clear();
            }
            continue;
        }
        potentialMatch.push_back(chr);
    }

    //
    // Gather a list of unique records:
    //
    std::map<std::string, bool> uniqueMatches;
    for (const auto & match : matches)
    {
        if (uniqueMatches.find(match) == std::end(uniqueMatches))
        {
            uniqueMatches.emplace(match, false);
        }
    }

    //
    // Now we print the good matches, but using the vector instead to preserve the
    // order found in the file. The map tells us if an entry was already printed or not.
    //
    for (const auto & match : matches)
    {
        if (acceptString(match, minSequence))
        {
            auto m = uniqueMatches.find(match);
            if (m != std::end(uniqueMatches) && !m->second)
            {
                (*pOut) << m->first << "\n";
                m->second = true;
            }
        }
    }

    return EXIT_SUCCESS;
}
