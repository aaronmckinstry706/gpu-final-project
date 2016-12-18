#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>

#include "encode_library.h"

// ---------------------------- MAIN FUNCTIONS ----------------------------------

size_t alphabetSize() {
	static size_t alphaSize = 1 << 8*sizeof(word_t);
	return alphaSize;
}

std::map<word_t, frequency_t> getCharacterFrequencies(FILE *filePointer) {
	std::map<word_t, frequency_t> frequencies;
	
	static word_t fileBuffer[FILE_BUFFER_SIZE];
	
	size_t numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	while (numRead > 0) {
		for (size_t i = 0; i < numRead; i++) {
			frequencies[fileBuffer[i]]++;
		}
		numRead = fread((void*)fileBuffer, sizeof(word_t), FILE_BUFFER_SIZE, filePointer);
	}
	return frequencies;
}

std::map<word_t, size_t> getCodeLengths(const std::map<word_t, frequency_t> &characterFrequencies, size_t lengthLimit) {
	std::vector<huffman_node*> forest;
	for (std::map<word_t, frequency_t>::const_iterator it = characterFrequencies.begin();
		it != characterFrequencies.end();
		it++)
	{
		for (double width = .5, i = 0; i < lengthLimit; i++, width /= 2) {
			forest.push_back(new huffman_node(width, it->first, it->second));
		}
	}
	
	while (forest.back()->width < 1) {
		std::sort(forest.begin(), forest.end(), huffman_node_pointer_comparator());

		if ((*(forest.end() - 1))->width != (*(forest.end() - 2))->width) {
			deleteTree(forest.back());
			forest.pop_back();
		}
		else {
			huffman_node *newNode = mergePackages(*(forest.end() - 1), *(forest.end() - 2));
			forest.pop_back();
			forest.pop_back();
			forest.push_back(newNode);
		}
	}

	std::map<word_t, size_t> codeLengths;
	for (size_t i = 0; i < 2*characterFrequencies.size() - 2; ++i) {
		addLeafOccurrenceCounts(forest[i], codeLengths);
	}

	for (size_t i = 0; i < forest.size(); ++i) {
		deleteTree(forest[i]);
	}

	for (std::map<word_t, size_t>::iterator it = codeLengths.begin(); it != codeLengths.end(); it++) {
		codeLengths[it->first]--;
	}

	return codeLengths;
}

void printTree(huffman_node *root) {
	printf("{ ");
	if (root->right == 0 && root->left == 0) {
		char mystr[3];
		if (root->word == '\n') {
			mystr[0] = '\\';
			mystr[1] = 'n';
			mystr[2] = '\0';
			printf("%s", mystr);
		}
		else {
			printf("%c", root->word);
		}
	}
	else {
		printTree(root->left);
		printTree(root->right);
	}
	printf(" }");
}

std::map<word_t, code_t> getCodes(const std::map<word_t, size_t> &codeLengths) {
	std::vector<huffman_node*> forest;
	for (std::map<word_t, size_t>::const_iterator it = codeLengths.begin(); it != codeLengths.end(); it++) {
		forest.push_back(new huffman_node(-(double)it->second, it->first, 0));//so hufman_node.width = code length for sorting
	}
	std::sort(forest.begin(), forest.end(), huffman_node_pointer_comparator());

	while (forest.size() > 1) {
		huffman_node *h = mergeTrees(forest.end()[-1], forest.end()[-2]);
		forest.pop_back();
		forest.pop_back();
		forest.push_back(h);
		std::sort(forest.begin(), forest.end(), huffman_node_pointer_comparator());
	}
	
	printTree(forest[0]);

	//now we have the huffman tree; traverse it to get the codes
	std::map<word_t, code_t> codes;
	constructCodesFromTree(forest[0], codes);
	deleteTree(forest[0]);
	return codes;
}

void constructCodesFromTree(huffman_node *root, std::map<word_t, code_t> &codes, code_t code, size_t depth) {
	if (root->left == 0 && root->right == 0)
		codes[root->word] = code;
	else {
		constructCodesFromTree(root->left, codes, code, depth + 1);
		constructCodesFromTree(root->right, codes, code | (1 << depth), depth + 1);
	}
}

std::vector<unsigned short> getDecodingTable(const std::map<word_t, code_t> &codes, const std::map<word_t, size_t> &codeLengths) {
	std::vector<unsigned short> decodingTable(1 << MAX_CODE_LENGTH, 0);

	std::map<word_t, code_t>::const_iterator codeIt = codes.begin();
	std::map<word_t, size_t>::const_iterator lengthIt = codeLengths.begin();
	for ( ; codeIt != codes.end(); ++codeIt, ++lengthIt) {
		size_t increment = 1 << lengthIt->second;
		for (size_t code = codeIt->second; code < (1 << MAX_CODE_LENGTH); code += increment) {
			decodingTable[code] = (unsigned short)(lengthIt->second << 8) | (unsigned short)lengthIt->first;
		}
	}

	return decodingTable;
}

void encodeFile(
	FILE *sourceFile,
	FILE *destinationFile, 
	const std::vector<unsigned short> &decodingTable, 
	const std::map<word_t, code_t> &codes, 
	const std::map<word_t, size_t> &codeLengths,
	size_t subBlockSize,
	unsigned long long sourceFileSize,
	unsigned long long encodedDataBitLength
) {
	//first output all the metadata

	size_t codeLengthLimit = MAX_CODE_LENGTH;
	subBlockSize *= 64;

	fwrite((void*)&codeLengthLimit, sizeof(size_t), 1, destinationFile);
	fwrite((void*)&subBlockSize, sizeof(size_t), 1, destinationFile);
	fwrite((void*)&sourceFileSize, sizeof(unsigned long long), 1, destinationFile);
	fwrite((void*)&encodedDataBitLength, sizeof(unsigned long long), 1, destinationFile);

	writeSubBlockOffsets(sourceFile, destinationFile, codeLengths, subBlockSize);
	fseek(sourceFile, 0, SEEK_SET);

	fwrite((void*)&decodingTable[0], sizeof(unsigned short), decodingTable.size(), destinationFile);

	//then encode the data bytes themselves

	unsigned char inBuffer[FILE_BUFFER_SIZE];
	unsigned char outBuffer[FILE_BUFFER_SIZE + 64];
	size_t outBitOffset = 0;
	
	size_t numRead = fread((void*)inBuffer, sizeof(word_t), FILE_BUFFER_SIZE, sourceFile);
	while (numRead > 0) {
		for (size_t i = 0; i < std::min((size_t)FILE_BUFFER_SIZE, numRead); ++i) {
			if (outBitOffset > 8 * FILE_BUFFER_SIZE) {
				fwrite(outBuffer, sizeof(unsigned char), FILE_BUFFER_SIZE, destinationFile);
				size_t bytesToShift = (outBitOffset - 8 * FILE_BUFFER_SIZE) / 8 + (outBitOffset % 8 > 0);
				for (size_t j = 0; j < bytesToShift; ++j)
					outBuffer[j] = outBuffer[FILE_BUFFER_SIZE + j];
				outBitOffset -= 8 * FILE_BUFFER_SIZE;
			}

			std::map<word_t, code_t>::const_iterator codeIter = codes.find(inBuffer[i]);
			code_t code = codeIter->second;

			std::map<word_t, size_t>::const_iterator lengthIter = codeLengths.find(inBuffer[i]);
			size_t length = lengthIter->second;

			writeCode(code, length, outBitOffset, outBuffer);
			outBitOffset += length;
		}
		numRead = fread((void*)inBuffer, sizeof(word_t), FILE_BUFFER_SIZE, sourceFile);
	}

	if (outBitOffset > 0) {
		fwrite(outBuffer, sizeof(unsigned char), outBitOffset / 8 + (outBitOffset % 8 > 0), destinationFile);
	}
	
	//now pad with 0's so reading is slightly easier to handle
	const size_t paddingSize = MAX_CODE_LENGTH / 8 + (MAX_CODE_LENGTH % 8 > 0);
	unsigned char temp[paddingSize];
	for (size_t i = 0; i < paddingSize; ++i)
		temp[i] = 0;
	fwrite(temp, sizeof(unsigned char), paddingSize, destinationFile);
}

void writeCode(code_t code, size_t bitLength, size_t bitOffset, unsigned char *stream) {
	while (bitLength > 0) {
		size_t byteOffset = bitOffset / 8;
		size_t withinByteOffset = bitOffset - 8 * byteOffset;
		stream[byteOffset] &= ((unsigned char)0xFF >> 8 - withinByteOffset);
		stream[byteOffset] |= (unsigned char)(code << withinByteOffset);
		size_t numBitsWritten = std::min(bitLength, 8 - withinByteOffset);
		bitOffset += numBitsWritten;
		code >>= numBitsWritten;
		bitLength -= numBitsWritten;
	}
}

void writeSubBlockOffsets(FILE *sourceFile, FILE *destinationFile, const std::map<word_t, size_t>& codeLengths, size_t subBlockSize) {
	static word_t fileBuffer[FILE_BUFFER_SIZE];

	unsigned long long offset = 0;

	size_t numRead = fread((void*)fileBuffer, sizeof(word_t), subBlockSize, sourceFile);
	while (numRead > 0) {
		fwrite((void*)&offset, sizeof(unsigned long long), 1, destinationFile);
		for (size_t i = 0; i < numRead; i++) {
			offset += codeLengths.find(fileBuffer[i])->second;
		}
		numRead = fread((void*)fileBuffer, sizeof(word_t), subBlockSize, sourceFile);
	}
}

// -------------------------- UTILITY FUNCTIONS FOR TREES OF HUFFMAN_NODE ----------------------

huffman_node * mergeTrees(huffman_node *h1, huffman_node *h2) {
	return new huffman_node(h1->width + 1, 0, h1, h2);
}

void addLeafOccurrenceCounts(huffman_node *h, std::map<word_t, size_t> &counts) {
	if (h->left == 0 && h->right == 0)
		counts[h->word]++;
	else {
		addLeafOccurrenceCounts(h->left, counts);
		addLeafOccurrenceCounts(h->right, counts);
	}
}

huffman_node * mergePackages(huffman_node *h1, huffman_node *h2) {
	return new huffman_node(
		h1->width + h2->width,
		h1->frequency + h2->frequency,
		h1,
		h2
		);
}

void deleteTree(huffman_node *h) {
	if (h == 0) return;

	deleteTree(h->left);
	deleteTree(h->right);

	delete h;
}

// ----------------------------------- HUFFMAN_NODE-RELATED CLASS METHODS ---------------------

huffman_node::huffman_node(double width, frequency_t freq, huffman_node *L, huffman_node *R) {
	this->frequency = freq;
	this->left = L;
	this->right = R;
	this->width = width;
}

huffman_node::huffman_node(double width, word_t word, frequency_t freq) {
	this->word = word;
	this->frequency = freq;
	this->left = 0;
	this->right = 0;
	this->width = width;
}

bool huffman_node_pointer_comparator::operator()(huffman_node *h1, huffman_node *h2) {
	return h1->width > h2->width //because smallest width must be at the largest index after std::sort()
		|| (
			h1->width == h2->width
			&& (
				h1->frequency > h2->frequency //because smallest freq must be at the largest indexes after std::sort()
				|| (
					h1->frequency == h2->frequency
					&& h1->word < h2->word
					)
				)
			);
}

