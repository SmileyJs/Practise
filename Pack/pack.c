#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static const int PAGE_SIZE = 4 * 1024;
static const int HEAD_SIZE = 16;
static const int LINE_OFFSET = 4 * 8;
static const int LINE_SIZE = 992 * 4;
static const int DATA_LINE_SIZE = 984 * 4;
static const int DATA_SIZE = DATA_LINE_SIZE * 500 - HEAD_SIZE;
static const int BLOCK_SIZE = (DATA_LINE_SIZE + LINE_OFFSET) * 512;

static const char *fileHead = "Upgrade_"; 

typedef struct PackageHead {
	uint32_t index;
	uint32_t len;
	uint32_t fileId;
	uint32_t crc;
} PackageHead;

uint32_t
generate_head(int packageNum, int packageIndex, int isFirstFile, int isLastFile, int fileType)
{
	uint32_t fileId = 0;
	if (packageIndex == 1) {
		if (packageNum == 1) {
			fileId |= 8;
		}
		else {
			fileId |= 1;
		}

		if (isFirstFile) {
			fileId |= 1 << 24;
		}
	}
	else {
		if (packageIndex == packageNum) {
			fileId |= 4;
			if (isLastFile) {
				fileId |= 1 << 25;
			}
		}
		else {
			fileId |= 2;		
		}
	}

	fileId |= 1 << (fileType + 16);

	return fileId;
}

int
generate_package(const char *fileName, int packageNum, int startIndex, int isFirstFile, int isLastFile, int fileType)
{
	FILE *fp;
	if (NULL == (fp = fopen(fileName, "r"))) {
		printf("%s fopen failed!\n", __FUNCTION__);
		return -1;
	}

	char buf_data[DATA_SIZE];
	char buf_block[BLOCK_SIZE];
	memset(buf_data, 0, DATA_SIZE);
	memset(buf_block, 0xFF, BLOCK_SIZE);

	for (int i = 0; i != packageNum; ++i) {
		char name[32];
		sprintf(name, "%s%04d", fileHead, startIndex + i);
		printf("%s\n", name);

		int readCount;
		if (0 == (readCount = fread(buf_data, 1, DATA_SIZE, fp))) {
			if (ferror(fp)) {
				printf("%s fread error\n", __FUNCTION__);
				return -1;
			}
			printf("%s fread 0\n", __FUNCTION__);
			break;
		}

		printf("read count %d\n", readCount);

		FILE *fpout;
		if (NULL == (fpout = fopen(name, "w"))) {
			printf("%s write fopen failed!\n", __FUNCTION__);
			return -1;
		}

		PackageHead head;
		head.index = startIndex + i;
		head.crc = 0xFFFFFFFF;
		head.len = readCount;
		head.fileId = generate_head(packageNum, i+1, isFirstFile, isLastFile, fileType);
		printf("%#010x\n", head.fileId);

		// copy package head into buf_block
		memcpy(buf_block, &head, sizeof(PackageHead));

		// copy data into buf_block;
		int hasWrite = 0;
		int offset = HEAD_SIZE;
		int firstLine = 1;
		while (hasWrite < head.len) {
			int size;
			if (firstLine) {
				size = DATA_LINE_SIZE - HEAD_SIZE;
				firstLine = 0;
			}
			else {
				if (hasWrite + DATA_LINE_SIZE > head.len) {
					size = head.len - hasWrite;
				}
				else {
					size = DATA_LINE_SIZE;
				}
			}
			memcpy(buf_block + offset, buf_data + hasWrite, size);
			hasWrite += size;
			offset += size + LINE_OFFSET;
		}
		printf("has write %d\n", hasWrite);

		if (BLOCK_SIZE != fwrite(buf_block, 1, BLOCK_SIZE, fpout)) {
			printf("%s fwrite failed!", __FUNCTION__);
			return -1;
		}

		fclose(fpout);
		memset(buf_block, 0xFF, BLOCK_SIZE);
	}

	fclose(fp);
	return 0;
}

int
append_file_crc(const char *fileName, const char *bakName)
{
	FILE *fpin,*fpout;
	if (NULL == (fpin = fopen(fileName, "r"))) {
		printf("%s fopen failed!\n", __FUNCTION__);
		return -1;
	}

	if (NULL == (fpout = fopen(bakName, "w"))) {
		printf("%s fopen out failed!\n", __FUNCTION__);
		return -1;
	} 

	char buf[PAGE_SIZE];

	int n;
	while (0 != (n = fread(buf, 1, PAGE_SIZE, fpin))) {
		if (n != fwrite(buf, 1, n, fpout)) {
			printf("%s fwrite failed!\n", __FUNCTION__);
			return -1;
		}
	}

	if (ferror(fpin)) {
		printf("%s fread error!\n", __FUNCTION__);
		return -1;
	}

	fclose(fpin);
	fclose(fpout);
	return 0;
}

int
main(int argc, char const *argv[])
{
	if (argc < 2) {
		printf("argument error!\n");
		return -1;
	}

	int totalNum = 0;
	for (int i = 1; i != argc; ++i) {
		const char *fileName = argv[i];
		struct stat sb;
		stat(fileName, &sb);

		int packageNum = sb.st_size / DATA_SIZE + 1;
		printf("%s(size:%lu) need %d package\n", fileName, sb.st_size, packageNum);

		char bakName[32];
		sprintf(bakName, "%s_bak", argv[i]);

		// Todo, just generate bak file.
		append_file_crc(fileName, bakName);

		int isFirstFile = (i==1 ? 1:0);
		int isLastFile = (i==argc-1 ? 1:0);

		generate_package(bakName, packageNum, totalNum + 1, isFirstFile, isLastFile, i-1);

		totalNum += packageNum;

		// delete bakFile at last.
		unlink(bakName);
	}

	printf("total pachage num is %d\n", totalNum);

	return 0;
}