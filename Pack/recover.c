#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static const int PAGE_SIZE = 4 * 1024;
static const int HEAD_SIZE = 16;
static const int LINE_OFFSET = 4 * 8;
static const int DATA_LINE_SIZE = 984 * 4;
static const int DATA_SIZE = DATA_LINE_SIZE * 500 - HEAD_SIZE;
static const int BLOCK_SIZE = (DATA_LINE_SIZE + LINE_OFFSET) * 512;

static const char *fileHead = "Upgrade_";
static const int MAX_PACKAGE = 100;

typedef struct PackageHead {
	uint32_t index;
	uint32_t len;
	uint32_t fileId;
	uint32_t crc;
} PackageHead;

typedef struct ReadBuf {
	PackageHead head;
	char buf[DATA_SIZE];
} ReadBuf;

static ReadBuf* readList[MAX_PACKAGE] = {NULL};

int
read_content()
{
	FILE *fpin;
	for (int i = 1; i != MAX_PACKAGE; ++i) {
		char name[32];
		sprintf(name, "%s%04d", fileHead, i);
		printf("name %s\n", name);

		if (NULL == (fpin = fopen(name, "r"))) {
			printf("fopen failed!\n");
			break;
		}

		ReadBuf *readBuf = (ReadBuf*)malloc(sizeof(ReadBuf));
		readList[i-1] = readBuf;

		int readCount = 0;
		char buf_block[BLOCK_SIZE];
		if (0 == (readCount = fread(buf_block, 1, BLOCK_SIZE, fpin))) {
			if (ferror(fpin)) {
				printf("read block failed\n");
				return -1;
			}
			printf("read 0\n");
		}

		// // read package Head
		// if (sizeof(PackageHead) != fread(&(readBuf->head), 1, sizeof(PackageHead), fpin)) {
		// 	printf("fread head failed!\n");
		// 	return -1;
		// }

		memcpy(&(readBuf->head), buf_block, sizeof(PackageHead));

		printf("index %d crc %#010x fileId %#010x len %d\n", readBuf->head.index, 
			readBuf->head.crc, readBuf->head.fileId, readBuf->head.len);

		int hasRead = 0;
		int firstTime = 1;
		int offset = HEAD_SIZE;
		while (hasRead < readBuf->head.len ) {
			int size;
			if (firstTime) {
				firstTime = 0;
				size = DATA_LINE_SIZE - HEAD_SIZE;
			}
			else {
				if (hasRead + DATA_LINE_SIZE > readBuf->head.len) {
					size = readBuf->head.len - hasRead;
				}
				else {
					size = DATA_LINE_SIZE;
				}
			}

			// if (0 == (readCount = fread(readBuf->buf + hasRead, 1, size, fpin))) {
			// 	if (ferror(fpin)) {
			// 		printf("fread error\n");
			// 		return -1;
			// 	}
			// 	printf("fread 0\n");
			// }

			memcpy(readBuf->buf + hasRead, buf_block + offset, size);

			hasRead += size;
			offset += size + LINE_OFFSET;
		}

		printf("hasRead %d\n", hasRead);

		fclose(fpin);
	}

	return 0;
}

int generate_copy_file()
{
	FILE *fpout;
	for (int i = 0; readList[i] != NULL; ++i) {
		// handle package head.		
		char *fileName;
		if (readList[i]->head.fileId & (1 << 16)) {
			fileName = "copy1";
		}
		else if (readList[i]->head.fileId & (1 << 17)) {
			fileName = "copy2";
		}
		else if (readList[i]->head.fileId & (1 << 18)) {
			fileName = "copy3";
		}
		else {
			printf("%s file else\n", __FUNCTION__);
			return -1;
		}

		if (NULL == (fpout = fopen(fileName, "a"))) {
			printf("%s fopen failed!\n", __FUNCTION__);
			return -1;
		}

		if (readList[i]->head.len != fwrite(readList[i]->buf, 1, readList[i]->head.len, fpout)) {
			printf("%s fwrite failed!\n", __FUNCTION__);
			return -1;
		}
		fclose(fpout);
	} 

	return 0;
}

int
main(int argc, char const *argv[])
{
	char *head = "Upgrade_";

	FILE *fpout = NULL, *fpin = NULL;
	char buf[BLOCK_SIZE];

	// read content into list.
	if (0 > read_content()) {
		printf("readContent faield!\n");
	}

	// generate copy file
	generate_copy_file();

	return 0;
}