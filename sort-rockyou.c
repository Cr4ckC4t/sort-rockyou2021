/*
*	Description: Process a big wordlist and split it into seperate files sorted by line-lengths 
*	Author: crackcat
*
*	Compile with: gcc -O3 sort-rockyou.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define DEBUG

#define MAX_LINE_LEN 20 // max len line of the wordlist to be processed (if not set correctly this will cause segfaults)

#define CHUNK_SIZE 4096 // should be >> max_line_len !

int main(int argc, char**argv) {

	if (argc != 2)
		return EXIT_FAILURE;

	char ofName[8];

	FILE* ofs[MAX_LINE_LEN+1];
	for (int i=0; i<MAX_LINE_LEN+1; i++) {
		snprintf(ofName, 8, "%03d.txt", i);
		ofs[i] = fopen(ofName, "w");
		if (!ofs[i]) {
			fprintf(stderr, "[!] Failed to create %s\n", ofName);
			return EXIT_FAILURE;
		}
	}


	FILE* ifd = fopen(argv[1], "r");
	if (!ifd) {
		fprintf(stderr, "[!] Failed to open %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	fprintf(stdout, "[+] Processing %s now ...\n", argv[1]);

	char *chunk = malloc(sizeof(char) * CHUNK_SIZE); // Allocate space for chunk
	if (!chunk){
		fprintf(stderr, "[!] Failed to malloc\n");
		return EXIT_FAILURE;
	}

	size_t bytesRead = 0;

	char outChunks[MAX_LINE_LEN+1][CHUNK_SIZE];
	unsigned int outIndex[MAX_LINE_LEN+1] = {0};
	long int len = 0;

	char leftOver[MAX_LINE_LEN] = {0};
	long int leftOverLen = 0;

	char* chunkStart = NULL;

	while ((bytesRead = fread(chunk, 1, CHUNK_SIZE, ifd)) > 0) {
#ifdef DEBUG
		fprintf(stdout, "[+] Processing chunk\n");
#endif
		memset(outChunks, 0, sizeof(outChunks));
		memset(outIndex, 0, sizeof(outIndex));

		chunkStart = chunk;

		// deal with left over from previous chunk
		if (leftOverLen) {
#ifdef DEBUG
			fprintf(stdout, "[+]\t Dealing with leftovers\n");
#endif

			while (*chunkStart++ != '\n'){};
			long int rightOverLen = chunkStart-chunk;
			len = leftOverLen+rightOverLen-1;
#ifdef DEBUG
			fprintf(stdout, "[+]\t Leftover len=%li\n", leftOverLen);
#endif

			memcpy(outChunks[len], leftOver, leftOverLen); // stitch together line
			memcpy(outChunks[len]+leftOverLen, chunk, rightOverLen);
			outIndex[len]=len+1;
			// reset remainder
			memset(leftOver, 0, sizeof(leftOver));
			leftOverLen = 0;
			// adapt bytesRead
		}

#ifdef DEBUG
		fprintf(stdout, "[+]\t Dealing with chunk\n");
#endif

		// process chunk
		char* s = NULL;
		for (s = chunkStart; s<chunk+bytesRead; ++s) { // iterate over chunk
			if (*s == '\n') {
				len = s-chunkStart; // determine line length
				memcpy(outChunks[len]+outIndex[len], chunkStart, len+1); // copy line to outChunk
				outIndex[len]+=len+1; // save the index of outChunk
				chunkStart = s+1; // save new line start position
			}
		}

		if (chunkStart != s) { // save remaining buffer
			leftOverLen = s-chunkStart;
			memcpy(leftOver, chunkStart, leftOverLen);
#ifdef DEBUG
			fprintf(stdout, "[+]\t Saving %li characters for next chunk\n", leftOverLen);
#endif
		}
#ifdef DEBUG

		fprintf(stdout, "[+] Writing to files\n");
#endif
		for (int i=0; i<MAX_LINE_LEN+1; i++)
			if (outIndex[i])
				fwrite(outChunks[i], outIndex[i], 1, ofs[i]); // write outchunk to file
	}

	fprintf(stdout, "[+] Done (%s)\n", argv[1]);

	fclose(ifd);

	for (int i=0; i<MAX_LINE_LEN+1; i++) {
		fclose(ofs[i]);
	}

	return EXIT_SUCCESS;
}
