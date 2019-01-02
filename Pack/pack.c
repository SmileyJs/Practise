#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

static const uint32_t crc32_table[256] =
{
    0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
    0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
    0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
    0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
    0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
    0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
    0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
    0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
    0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
    0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
    0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
    0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
    0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
    0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
    0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
    0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
    0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
    0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
    0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
    0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
    0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
    0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
    0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
    0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
    0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
    0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
    0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
    0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
    0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
    0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
    0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
    0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
    0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
    0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
    0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
    0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
    0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
    0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
    0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
    0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
    0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
    0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
    0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
    0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
    0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
    0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
    0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
    0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
    0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
    0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
    0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
    0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
    0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
    0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
    0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
    0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
    0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
    0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
    0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
    0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
    0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
    0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
    0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
    0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};

uint32_t crc_calculate(const unsigned char *data, uint32_t bytes, uint32_t crc)
{
    while (bytes--) {
    	crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *data++];
   	}

    return crc;
}

int byte_swap(uint8_t *data, size_t len)
{
	if (0 != len % 4) {
		printf("byte swap length error!\n");
		return -1;
	}

	for (int i = 0; i < len; i+=4) {
		*(data + i) ^= *(data + 3 + i);
		*(data + 3 + i) ^= *(data + i);
		*(data + i) ^= *(data + 3 + i);
	}
}

static const uint32_t DATA_LINE_SIZE = 984 * 4;
static const uint32_t BLOCK_SIZE = 512 * 992 * 4;
static const uint32_t HEAD_SIZE = 16;
static const uint32_t DATA_SIZE = 500 * DATA_LINE_SIZE - HEAD_SIZE;
static const uint32_t PAGE_SIZE = 4 * 1024;
static const uint32_t LINE_OFFSET = 4 * 8;

static const char *fileNameHead = "X37_ICtoMP5_Upgrade_";
static const uint32_t MAX_FILENUM = 3;

static const char *FILENAME_GIP = "ifs-mx6q-sabrelite_etfs.raw";
static const char *FILENAME_VIP = "PRODUCT_APP.L1";
static const char *FILENAME_EEPROM = "PRODUCT.N07";

typedef struct {
	uint32_t crc;
	uint32_t index;
	uint32_t len;
	uint32_t fileID;
} PackageHead;

typedef enum {
	FILE_GIP = 0,
	FILE_VIP,
	FILE_EEPROM
} FileType;

typedef struct {
	FileType type;
	uint32_t packageNum;
} FileInfo;

typedef struct {
	uint32_t fileNum;
	uint32_t crtFileIndex;
	uint32_t crtPackageIndex;
	FileInfo fileInfo[MAX_FILENUM];
} PackInfo;

int append_file_crc(const char* fileName, const char* bakName)
{
	FILE *fpin, *fpout;
	if (NULL == (fpin = fopen(fileName, "r"))) {
		printf("%s fopen input failed!\n", __FUNCTION__);
		return -1;
	}

	if (NULL == (fpout = fopen(bakName, "w+"))) {
		printf("%s fopen output failed!\n", __FUNCTION__);
		return -1;
	}

	unsigned char buf[PAGE_SIZE] = {0};
	int n = 0;
	uint32_t crc = 0xFFFFFFFF;

	// calculate crc result for every page size data
	while (0 != (n = fread(buf, 1, sizeof(buf), fpin))) {
		if (n != fwrite(buf, 1, n, fpout)) {
			printf("%s fwrite failed!\n", __FUNCTION__);
		}
		crc = crc_calculate(buf, n, crc);
	}

	printf("crc result = %#010x\n", crc);

	// append crc check sum into backup file
	if (1 != fwrite(&crc, sizeof(crc), 1, fpout)) {
		printf("%s fwrite crc failed!\n", __FUNCTION__);
		return -1;
	}

	fclose(fpin);
	fclose(fpout);

	return 0;
}

uint32_t
generate_fileID(PackInfo *info, int bFirstPackage, int bLastPackage)
{
	uint32_t fileID = 0;

	if (bFirstPackage && bLastPackage) {
		fileID |= 1 << 3;
	}
	else if (bFirstPackage) {
		fileID |= 1;
	}
	else if (bLastPackage) {
		fileID |= 1 << 2;
	}
	else {
		fileID |= 1 << 1;
	}

	if (0 == info->crtFileIndex && 1 == info->fileNum) {
		fileID |= 1 << 26;
	}
	else if (0 == info->crtFileIndex) {
		if (bFirstPackage)
			fileID |= 1 << 24;
	}
	else if (info->crtFileIndex == info->fileNum - 1) {
		if (bLastPackage)
			fileID |= 1 << 25;
	}

	if (info->fileInfo[info->crtFileIndex].type == FILE_GIP) {
		fileID |= 1 << 16;
	}
	else if (info->fileInfo[info->crtFileIndex].type == FILE_VIP) {
		fileID |= 1 << 17;
	}
	else if (info->fileInfo[info->crtFileIndex].type == FILE_EEPROM) {
		fileID |= 1 << 18;
	}
	else {
		printf("type error\n");
	}
	
	printf("fileID %#010x\n", fileID);

	return fileID;
}

int
generate_package(FILE *fpin, PackInfo *info, int bFirstPackage, int bLastPackage)
{
	unsigned char buf_block[BLOCK_SIZE];
	memset(buf_block, 0, BLOCK_SIZE);

	unsigned char buf_data[DATA_SIZE+HEAD_SIZE];
	memset(buf_data, 0, DATA_SIZE+HEAD_SIZE);

	int readN;
	if (0 == (readN = fread(buf_data+HEAD_SIZE, 1, DATA_SIZE, fpin))) {
		if (ferror(fpin)) {
			printf("%s fread error\n", __FUNCTION__);
			return -1;
		}
		printf("%s fread count = 0\n", __FUNCTION__);
		return -1;
	}

	char packageName[32];
	sprintf(packageName, "%s%04d", fileNameHead, info->crtPackageIndex + 1);
	printf("%s\n", packageName);

	FILE *fpout;
	if (NULL == (fpout = fopen(packageName, "w"))) {
		printf("%s fopen fpout failed!\n", __FUNCTION__);
		return -1;
	}

	int firstLine = 1;
	unsigned char *dest = buf_block;
	unsigned char *src = buf_data + HEAD_SIZE; // skip HEAD
	int hasCopy = 0; 
	int size = 0;
	while (hasCopy < readN) {
		if (firstLine) {
			size = DATA_LINE_SIZE - HEAD_SIZE;
			dest += HEAD_SIZE;
			firstLine = 0;
		}
		else {
			size = DATA_LINE_SIZE;
		}

		if (hasCopy + size > readN)
			size = readN - hasCopy;

		memcpy(dest, src + hasCopy, size);

		hasCopy += size;
		dest += (size + LINE_OFFSET);
	}

	printf("length %d %#010x hasCopy %d\n", readN, readN, hasCopy);

	PackageHead head;
	head.len = readN;
	head.index = info->crtPackageIndex + 1; // index start with 0, but index start with 1
	head.fileID = generate_fileID(info, bFirstPackage, bLastPackage);
	head.crc = 0xFFFFFFFF;

	memcpy(buf_data, &head, sizeof(PackageHead));

	uint32_t crc = 0xFFFFFFFF;
	crc = crc_calculate(buf_data + sizeof(uint32_t), readN + 3*sizeof(uint32_t), crc);
	head.crc = crc;

	memcpy(buf_block, &head, sizeof(PackageHead));

	printf("index %d len %d fileId %#010x crc %#010x\n", head.index, head.len, head.fileID, head.crc);

	byte_swap((uint8_t*)buf_block, BLOCK_SIZE);

	if (BLOCK_SIZE != fwrite(buf_block, 1, BLOCK_SIZE, fpout)) {
		printf("%s fwrite failed!\n", __FUNCTION__);
		return -1;
	}

	fclose(fpout);

	info->crtPackageIndex++;

	return 0;
}

int
file_split(const char *fileName, PackInfo *info)
{
	FILE *fpin;

	if (NULL == (fpin = fopen(fileName, "r"))) {
		printf("file open %s failed!\n", fileName);
		return -1;
	}

	int packageNum = info->fileInfo[info->crtFileIndex].packageNum;

	for (int i = 0; i != packageNum; ++i) {
		int isFirst = (0 == i) ? 1 : 0;
		int isLast = (i == (packageNum-1)) ? 1 : 0;

		if (0 > generate_package(fpin, info, isFirst, isLast)) {
			printf("generate_package failed!\n");
			return -1;
		}
	}

	fclose(fpin);
	return 0;
}

int
main(int argc, char const *argv[])
{
	if (argc < 2) {
		printf("please input file name, try again\n");
		return -1;
	}
	else if (argc > MAX_FILENUM + 1) {
		printf("two many file, try again.\n");
		return -1;
	}

	PackInfo info;
	info.fileNum = argc - 1;
	info.crtPackageIndex = 0; // package num start with 1
	info.crtFileIndex = 0;

	for (int i = 1; i != argc; ++i) {
		info.crtFileIndex = i - 1;
		// identify file by file name.
		if (0 == strcmp(argv[i], FILENAME_GIP)) {
			info.fileInfo[info.crtFileIndex].type = FILE_GIP;
		}
		else if (0 == strcmp(argv[i], FILENAME_VIP)) {
			info.fileInfo[info.crtFileIndex].type = FILE_VIP;
		}
		else if (0 == strcmp(argv[i], FILENAME_EEPROM)) {
			info.fileInfo[info.crtFileIndex].type = FILE_EEPROM;
		}
		else {
			printf("type else\n");
			continue;
		}

		struct stat sb;
		stat(argv[i], &sb);
		info.fileInfo[info.crtFileIndex].packageNum =  (sb.st_size + 4) / DATA_SIZE +1;	// add 4 for file crc code
		printf("%s(size:%lu) need %d package\n", argv[i], sb.st_size, info.fileInfo[info.crtFileIndex].packageNum);

		// generate a temporary file. name as fileName_bak.
		char bakName[64];
		sprintf(bakName, "%s_bak", argv[i]);

		if (0 != append_file_crc(argv[i], bakName)) {
			printf("file %s append_file_crc failed\n", argv[i]);
		}

		// read data from the bak file.
		if (0 != file_split(bakName, &info)) {
			printf("file %s split failed!\n", argv[i]);
		}

		unlink(bakName); // delete bak file after generate package.
	}

	printf("Total %d package was created!\n", info.crtPackageIndex);

	return 0;
}
