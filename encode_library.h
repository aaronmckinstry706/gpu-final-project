#ifndef ENCODE_LIBRARY_H
#define ENCODE_LIBRARY_H

#include <cstdio>
#include <map>

typedef unsigned char word_t;
typedef unsigned char code_t;
typedef unsigned long long frequency_t;

struct huffman_node {
	word_t word;
	frequency_t frequency;
	double width;
	huffman_node *left;
	huffman_node *right;

	huffman_node(double width, frequency_t freq, huffman_node *L, huffman_node *R);
	huffman_node(double width, word_t word, frequency_t freq);
};

#define FILE_BUFFER_SIZE 2048 //number of word_t items stored in file buffer when reading

std::map<word_t, frequency_t> getCharacterFrequencies(FILE *filePointer);
std::map<word_t, size_t> getCodeLengths(const std::map<word_t, frequency_t>& characterFrequencies, size_t lengthLimit);

//utility functions for huffman trees

void addLeafOccurrenceCounts(huffman_node *h, std::map<word_t, size_t>& counts);
huffman_node * mergeTrees(huffman_node *h1, huffman_node *h2);
void deleteTree(huffman_node *h);

//debug functions

#endif
