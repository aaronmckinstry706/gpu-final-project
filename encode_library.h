#ifndef ENCODE_LIBRARY_H
#define ENCODE_LIBRARY_H

#include <cstdio>
#include <map>

typedef unsigned char word_t;
typedef unsigned short code_t;
typedef unsigned long long frequency_t;

#define FILE_BUFFER_SIZE 2048 //number of word_t items stored in file buffer when reading

size_t alphabetSize();

std::map<word_t, frequency_t> getCharacterFrequencies(FILE *filePointer);
std::map<word_t, size_t> getWordCodeLengths(const std::map<word_t, frequency_t>& frequencies);
std::map<word_t, code_t> getWordCodeTable(const std::map<word_t, size_t>& wordCodeLengths);


//debug functions
void printCharacterFrequencies(const std::map<word_t, frequency_t>& frequencies);

#endif
