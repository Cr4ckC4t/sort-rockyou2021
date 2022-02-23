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

#define MAX_LINE_LEN 20
#define MIN_LINE_LEN 6

#define CHUNK_SIZE 4096 // >> max_line_len (can be played around with)

int main(int argc, char**argv) {

	if (argc != 2)
		return EXIT_FAILURE;

	char ofName[8];

	/* Open output files */
	FILE* ofs[MAX_LINE_LEN+1];
	for (int i=MIN_LINE_LEN; i<=MAX_LINE_LEN; i++) {
		snprintf(ofName, 8, "%03d.txt", i);
		ofs[i] = fopen(ofName, "a");
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

#ifdef DEBUG
	fprintf(stdout, "[+] Processing %s now ...\n", argv[1]);
#endif

	/* Allocate memory for chunks */
	char *chunk = malloc(sizeof(char) * CHUNK_SIZE);
	if (!chunk){
		fprintf(stderr, "[!] Failed to malloc\n");
		return EXIT_FAILURE;
	}

	size_t bytesRead = 0;

	char outChunks[MAX_LINE_LEN+1][CHUNK_SIZE]; // Array of output chunks (buffer outputs before writing)
	unsigned int outIndex[MAX_LINE_LEN+1] = {0};
	long int len = 0;

	char leftOver[MAX_LINE_LEN] = {0};
	long int leftOverLen = 0;

	char* chunkStart = NULL;

	/* Process file chunk by chunk */
	while ((bytesRead = fread(chunk, 1, CHUNK_SIZE, ifd)) > 0) {
#ifdef DEBUG
		fprintf(stdout, "[+] Processing chunk\n");
#endif
		/* Clear output buffers */
		memset(outChunks, 0, sizeof(outChunks));
		memset(outIndex, 0, sizeof(outIndex));

		chunkStart = chunk;

		/* Deal with remainder from previous chunks */
		if (leftOverLen) {
#ifdef DEBUG
			fprintf(stdout, "[+]\t Dealing with leftovers\n");
#endif

			/* Advance pointer to the position after the first \n */
			while (*chunkStart++ != '\n'){};
			long int rightOverLen = chunkStart-chunk;
			len = leftOverLen+rightOverLen-1; // calculate line length across chunk border
#ifdef DEBUG
			fprintf(stdout, "[+]\t Leftover len=%li\n", leftOverLen);
#endif

			/* Stitch together the line */
			memcpy(outChunks[len], leftOver, leftOverLen);
			memcpy(outChunks[len]+leftOverLen, chunk, rightOverLen);
			outIndex[len]=len+1;

			/* Clear the remainder */
			leftOverLen = 0;
		}

#ifdef DEBUG
		fprintf(stdout, "[+]\t Dealing with chunk\n");
#endif

		/* Process the chunk */
		char* chunkEnd = chunk+bytesRead;
		for (char *s = chunkStart+MIN_LINE_LEN; s<chunkEnd; ++s) {
			if (*s == '\n') {
				len = s-chunkStart; // determine line length
				memcpy(outChunks[len]+outIndex[len], chunkStart, len+1); // copy line to output buffer
				outIndex[len]+=len+1; // save the last index of the output buffer
				chunkStart = s+1; // save the new line start position (after the current \n)
				s+=MIN_LINE_LEN; // we can skip the search for the next \n by the minimum line length
			}
		}

		/* Save any unprocessed characters */
		if (chunkStart != chunkEnd) {
			leftOverLen = chunkEnd-chunkStart;
			memcpy(leftOver, chunkStart, leftOverLen);
#ifdef DEBUG
			fprintf(stdout, "[+]\t Saving %li characters for next chunk\n", leftOverLen);
#endif
		}

#ifdef DEBUG
		fprintf(stdout, "[+] Writing to files\n");
#endif
		/* Write output buffers to files */
		for (int i=MIN_LINE_LEN; i<=MAX_LINE_LEN; i++)
			if (outIndex[i])
				fwrite(outChunks[i], outIndex[i], 1, ofs[i]); // write outchunk to file
	}

#ifdef DEBUG
	fprintf(stdout, "[+] Cleaning up\n");
#endif

	free(chunk);

	fclose(ifd);

	/* Close the output files */
	for (int i=MIN_LINE_LEN; i<=MAX_LINE_LEN; i++) {
		fclose(ofs[i]);
	}

#ifdef DEBUG
	fprintf(stdout, "[+] Done (%s)\n", argv[1]);
#endif

	return EXIT_SUCCESS;
}
