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

std::map<word_t, size_t> getCodeLengths(const std::map<word_t, frequency_t>& characterFrequencies, size_t lengthLimit) {
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

std::map<word_t, code_t> getCodes(const std::map<word_t, size_t>& codeLengths) {
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
	}

	//now we have the huffman tree; traverse it to get the codes
	std::map<word_t, code_t> codes;
	constructCodesFromTree(forest[0], codes);
	deleteTree(forest[0]);
	return codes;
}

void constructCodesFromTree(huffman_node *root, std::map<word_t, code_t>& codes, code_t code, size_t depth) {
	if (root->left == 0 && root->right == 0)
		codes[root->word] = code;
	else {
		constructCodesFromTree(root->left, codes, code, depth + 1);
		constructCodesFromTree(root->right, codes, code | (1 << depth), depth + 1);
	}
}

// -------------------------- UTILITY FUNCTIONS FOR TREES OF HUFFMAN_NODE ----------------------

huffman_node * mergeTrees(huffman_node *h1, huffman_node *h2) {
	return new huffman_node(h1->width + 1, 0, h1, h2);
}

void addLeafOccurrenceCounts(huffman_node *h, std::map<word_t, size_t>& counts) {
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

