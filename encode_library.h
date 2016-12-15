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

struct huffman_node_pointer_comparator {
	bool operator()(huffman_node *h1, huffman_node *h2);
};

#define FILE_BUFFER_SIZE 2048 //number of word_t items stored in file buffer when reading
#define MAX_CODE_LENGTH 12 //max number of bits in a code

std::map<word_t, frequency_t> getCharacterFrequencies(FILE *filePointer);
std::map<word_t, size_t> getCodeLengths(const std::map<word_t, frequency_t> &characterFrequencies, size_t lengthLimit);
std::map<word_t, code_t> getCodes(const std::map<word_t, size_t> &codeLengths);
void constructCodesFromTree(huffman_node *huffmanTree,
		std::map<word_t, code_t> &codes, code_t code = 0 << 8 * sizeof(code_t), size_t depth = 0);
std::vector<unsigned short> getDecodingTable(const std::map<word_t, code_t> &codes, const std::map<word_t, size_t> &codeLengths);

void encodeFile(
	FILE *sourceFile, 
	FILE *destinationFile, 
	const std::vector<unsigned short> &decodingTable, 
	const std::map<word_t,code_t> &codes, 
	const std::map<word_t,size_t> &codeLengths
);

void writeCode(code_t code, size_t bitLength, size_t bitOffset, unsigned char *stream);

//utility functions for huffman tree nodes
huffman_node * mergeTrees(huffman_node *h1, huffman_node *h2); // for Huffman tree construction
void addLeafOccurrenceCounts(huffman_node *h, std::map<word_t, size_t> &counts);
huffman_node * mergePackages(huffman_node *h1, huffman_node *h2); // for package-merge algorithm
void deleteTree(huffman_node *h);

//debug functions

#endif
