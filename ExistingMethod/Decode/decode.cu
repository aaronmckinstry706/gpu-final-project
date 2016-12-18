#include <stdio.h>

#define gpuErrorCheck(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPU assert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

inline void assertSuccessfulAllocation(void *ptr, const char * str) {
	if (ptr == 0) {
		fprintf(stderr, "Assert: memory could not be allocated, %s\n", str);
		exit(1);
	}
}

#define MAX_NUM_THREADS ((unsigned long long)1 << 20)

void decode(const char *sourceFilename, const char *destinationFilename, unsigned long long codeLengthLimit, unsigned long long subBlockSize, unsigned long long originalFileSize, unsigned long long sizeOfEncodedData);

__global__ void decodeKernel(unsigned long long *subBlockInputBitOffsets, unsigned char *encodedData,
		unsigned char *decodedData, unsigned short *decodingTable,
		unsigned long long blockInputByteOffset, unsigned long long numSymbolsPerSubBlock,
		unsigned long long encodedDataBitSize, unsigned long long codeBitLengthLimit,
		unsigned long long blockSize)
{
//	__shared__ unsigned short sharedDecodingTable[1 << 12];
//	unsigned short decodingTableSize = 1 << codeBitLengthLimit;
//	for (unsigned i = threadIdx.x; i < decodingTableSize; i += blockDim.x) {
//		sharedDecodingTable[i] = decodingTable[i];
//	}
	
//	__syncthreads();
	
	unsigned long long subBlock = blockIdx.x*blockDim.x + threadIdx.x;
	if (subBlock >= blockSize)
		return;
	unsigned long long subBlockInputBitOffset = subBlockInputBitOffsets[subBlock] - 8*blockInputByteOffset;
	unsigned long long inputBitCurOffset = subBlockInputBitOffset;
	
	for (size_t codeWordIndex = 0;
		codeWordIndex < numSymbolsPerSubBlock
			& inputBitCurOffset < encodedDataBitSize - 8*blockInputByteOffset;
		codeWordIndex++)
	{
		unsigned long long byte = inputBitCurOffset/8;
		unsigned long long bit = inputBitCurOffset - 8*byte;
		unsigned long long code = 0 << 8*sizeof(unsigned long long);
		code |= (unsigned long long)encodedData[byte];
		code |= (unsigned long long)encodedData[byte+1] << 8;
		code |= (unsigned long long)encodedData[byte+2] << 16;
		code >>= bit;
		code &= 0xFFFFFFFFFFFFFFFFull >> (64 - codeBitLengthLimit);
		unsigned short decoderEntry = decodingTable[code];
		unsigned char output = (unsigned char)decoderEntry;
		unsigned long long length = decoderEntry >> 8;
		inputBitCurOffset += length;
		decodedData[subBlock * numSymbolsPerSubBlock + codeWordIndex] = output;
	}
}

bool debugMode = false;

int main(int argc, char** argv) {
	if (argc != 3 && argc != 4) {
		printf("Usage: ./decode <encodedFile> <destinationFile> <debug(optional)>\n");
		exit(0);
	}
	
	FILE *source = fopen(argv[1], "r");
	
	unsigned long long codeLengthLimit, subBlockSize, originalFileSize, sizeOfEncodedData;
	fread((void *)&codeLengthLimit, sizeof(unsigned long long), 1, source);
	printf("got here\n");
	fread((void *)&subBlockSize, sizeof(unsigned long long), 1, source);
	fread((void *)&originalFileSize, sizeof(unsigned long long), 1, source);
	fread((void *)&sizeOfEncodedData, sizeof(unsigned long long), 1, source);
	
	if (argc == 4) {
		debugMode = true;
		printf("Code Length Limit: %llu\n"
			"Sub-Block Size: %llu\n"
			"Original File Size: %llu\n"
			"Size of Encoded Data: %llu\n",
			codeLengthLimit,
			subBlockSize,
			originalFileSize,
			sizeOfEncodedData);
	}
	
	fclose(source);
	
	decode(argv[1], argv[2], codeLengthLimit, subBlockSize, originalFileSize, sizeOfEncodedData);
	
	return 0;
	
}

void printBits(unsigned long long n) {
	for (size_t i = 0; i < 8*sizeof(unsigned long long); ++i) {
		printf("%c", (n & ((unsigned long long)1 << i)) != (0 << 8*sizeof(unsigned long long)) ? '1' : '0');
	}
}

void printBits(unsigned char n) {
	for (size_t i = 0; i < 8*sizeof(unsigned char); ++i) {
		printf("%c", (n & ((unsigned char)1 << i)) != (0 << 8*sizeof(unsigned char)) ? '1' : '0');
	}
}

void printBits(unsigned short n) {
	for (size_t i = 0; i < 8*sizeof(unsigned short); ++i) {
		printf("%c", (n & ((unsigned short)1 << i)) != (0 << 8*sizeof(unsigned short)) ? '1' : '0');
	}
}

void decode(const char *sourceFilename, const char *destinationFilename, unsigned long long codeLengthLimit, unsigned long long subBlockSize, unsigned long long originalFileSize, unsigned long long sizeOfEncodedData) {
	
	cudaError_t result;//for debugging
	
	unsigned long long subBlockOffsetTableSize = originalFileSize/subBlockSize + (originalFileSize%subBlockSize > 0);
	if (debugMode)
		printf("Offset Table Size: %llu\n", subBlockOffsetTableSize);
	unsigned long long decodeTableSize = (unsigned long long)1 << (codeLengthLimit + 1);
	if (debugMode)
		printf("Decoding Table Size: %llu\n", decodeTableSize);
	
	if (debugMode) printf("Reading decoding table...\n");
	FILE *decodingTableReader = fopen(sourceFilename, "r");
	fseek(decodingTableReader, subBlockOffsetTableSize*sizeof(unsigned long long) + 4*sizeof(unsigned long long), SEEK_SET);
	unsigned short *decodingTable = new unsigned short[decodeTableSize/2];
	assertSuccessfulAllocation(decodingTable, "decoding table");
	size_t numRead = fread((void *)decodingTable, sizeof(unsigned short), decodeTableSize/2, decodingTableReader);
	fclose(decodingTableReader);
	if (debugMode) printf("Finished reading decoding table.\n");
	
	unsigned long long offsetTableLocation = 8*4;
	if (debugMode) printf("Offset Table Location: %llu\n", offsetTableLocation);
	unsigned long long encodedDataLocation = 8*4 + 8*subBlockOffsetTableSize + decodeTableSize;
	if (debugMode) printf("Encoded Data Location: %llu\n", encodedDataLocation);
	
	FILE *offsetReader = fopen(sourceFilename, "r");
	fseek(offsetReader, offsetTableLocation, SEEK_SET);
	
	FILE *encodedDataReader = fopen(sourceFilename, "r");
	fseek(encodedDataReader, encodedDataLocation, SEEK_SET);
	if (debugMode) {
	//	unsigned char byte;
	//	while (fread((void *)&byte, sizeof(unsigned char), 1, encodedDataReader))
	//		printBits(byte);
	//	printf("\n");
	//	fseek(encodedDataReader, encodedDataLocation, SEEK_SET);
	}
	
	unsigned long long codeLengthLimitInBytes = codeLengthLimit/8 + (codeLengthLimit % 8 > 0);
	
	unsigned char *inputBuffer = new unsigned char[codeLengthLimitInBytes * MAX_NUM_THREADS * subBlockSize];
	assertSuccessfulAllocation(decodingTable, "decoding table");
	unsigned char *outputBuffer = new unsigned char[MAX_NUM_THREADS * subBlockSize];
	assertSuccessfulAllocation(outputBuffer, "output buffer");
	unsigned long long *offsetsBuffer = new unsigned long long[MAX_NUM_THREADS];
	assertSuccessfulAllocation(offsetsBuffer, "offsets buffer");
	
	if (debugMode) printf("Allocating CUDA memory...\n");
	unsigned char *cudaInput;
	result = cudaMalloc(&cudaInput, codeLengthLimitInBytes * MAX_NUM_THREADS * subBlockSize);
	gpuErrorCheck(result);
	unsigned char *cudaOutput;
	result = cudaMalloc(&cudaOutput, MAX_NUM_THREADS * subBlockSize);
	gpuErrorCheck(result);
	
	unsigned long long *cudaOffsets;
	result = cudaMalloc(&cudaOffsets, MAX_NUM_THREADS * sizeof(unsigned long long));
	gpuErrorCheck(result);
	unsigned short *cudaDecodingTable;
	result = cudaMalloc(&cudaDecodingTable, decodeTableSize);
	gpuErrorCheck(result);
	result = cudaMemcpy(cudaDecodingTable, decodingTable, decodeTableSize, cudaMemcpyHostToDevice);
	gpuErrorCheck(result);
	if (debugMode) printf("Finished allocating CUDA memory.\n");
	
	FILE *outputWriter = fopen(destinationFilename, "w");
	
	unsigned long long numSubBlocksRead = 0;
	unsigned long long numInputBytesRead = 0;
	while (numSubBlocksRead < subBlockOffsetTableSize) {
		unsigned long long numThreads = min((unsigned long long)MAX_NUM_THREADS, subBlockOffsetTableSize - numSubBlocksRead);
		if (debugMode) printf("Num Threads: %llu\n", numThreads);
		fread((void *)offsetsBuffer, sizeof(unsigned long long), numThreads, offsetReader);
		
		numInputBytesRead = offsetsBuffer[0]/8;
		fseek(encodedDataReader, encodedDataLocation + numInputBytesRead, SEEK_SET);
		unsigned long long numInputBytes = min(MAX_NUM_THREADS * subBlockSize * codeLengthLimitInBytes, (sizeOfEncodedData/8 + (sizeOfEncodedData%8 > 0)) - numInputBytesRead);
		if (debugMode) printf("Num Input Bytes Intended to be Read: %llu\n", numInputBytes);
		numRead = fread((void *)inputBuffer, sizeof(unsigned char), numInputBytes + 2, encodedDataReader);
		if (debugMode) {
			printf("Num Input Bytes Actually Read: %llu\n", numRead);
//			for (size_t i = 0; i < numInputBytes + 2; ++i)
//				printBits(inputBuffer[i]);
//			printf("\n");
		}
		
		result = cudaMemcpy(cudaInput, inputBuffer, numInputBytes, cudaMemcpyHostToDevice);
		gpuErrorCheck(result);
		result = cudaMemcpy(cudaOffsets, offsetsBuffer, numThreads*sizeof(unsigned long long),
				cudaMemcpyHostToDevice);
		gpuErrorCheck(result);
		
		dim3 threadsPerBlock(min((unsigned long long)512, subBlockOffsetTableSize));
		dim3 numBlocks(numThreads/threadsPerBlock.x + (numThreads%threadsPerBlock.x>0));
		if (debugMode) printf("Num Thread-Blocks in current kernel: %d\n", numBlocks.x);
		decodeKernel<<<numBlocks,threadsPerBlock>>>(
			cudaOffsets, cudaInput, cudaOutput, cudaDecodingTable,
			numInputBytesRead, subBlockSize, sizeOfEncodedData, codeLengthLimit, numThreads
		);
		
		result = cudaMemcpy(outputBuffer, cudaOutput, numThreads*subBlockSize,
				cudaMemcpyDeviceToHost);
		gpuErrorCheck(result);
		
		if (debugMode) {
			printf("Output bits for last 4 characters: ");
			printBits(outputBuffer[476*subBlockSize]);
			printBits(outputBuffer[476*subBlockSize + 1]);
			printBits(outputBuffer[476*subBlockSize + 2]);
			printBits(outputBuffer[476*subBlockSize + 3]);
			printf("\n");
			
			printf("Input offset for last block: ");
			printBits(offsetsBuffer[476]);
			printf("\n");
			
			printf("Input offset in decimal for last block: %llu\n", offsetsBuffer[476]);
			
			printf("Encoded data in input buffer: ");
			size_t offset = offsetsBuffer[476];
			printBits(inputBuffer[offset/8]);
			printBits(inputBuffer[offset/8 + 1]);
			printf("\n");
			printf("(First %llu bits above should be ignored.)\n", offset - 8*(offset/8));
			
			//printf("Decoding table result of encoded data:\n");
			unsigned long long index = inputBuffer[offset/8 + 1];
			index <<= 8;
			index |= inputBuffer[offset/8];
			index &= 0x0000000000000FFFull;
			printBits(index);
			printf("\n");
			printf("%c %d", (unsigned char)(decodingTable[index] & (unsigned short)0xFF), (int)(decodingTable[index] >> 8));
			printf("\n");
		}
		
		fwrite((void *)outputBuffer, sizeof(unsigned char),
			min(numThreads*subBlockSize, originalFileSize - numSubBlocksRead*subBlockSize),
			outputWriter);
		
		numSubBlocksRead += numThreads;
		if (debugMode) {
			static int x = 0;
			printf("Finished iteration %d\n", ++x);
		}
	}
	
	cudaFree(cudaDecodingTable);
	cudaFree(cudaInput);
	cudaFree(cudaOutput);
	cudaFree(cudaOffsets);
	
	delete [] offsetsBuffer;
	delete [] outputBuffer;
	delete [] inputBuffer;
	
}


