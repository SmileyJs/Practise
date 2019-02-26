#include <iostream>
#include <vector>

// c
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

static const uint32_t DATA_LINE_SIZE = 984 * 4;
static const uint32_t BLOCK_SIZE = 512 * 992 * 4;
static const uint32_t HEAD_SIZE = 16;
static const uint32_t PAGE_SIZE = 4 * 1024;
static const uint32_t LINE_OFFSET = 4 * 8;

static const uint32_t FORMAT_DATA_SIZE = 500 * DATA_LINE_SIZE;
static const uint32_t FORMAT_HEAD_SIZE = 21;

static const uint32_t DATA_SIZE = 500 * DATA_LINE_SIZE * 3/4 - HEAD_SIZE;

static const char *fileNameHead = "X37_ICtoMP5_Upgrade_";

static const string FILENAME_GIP = "ifs-mx6q-sabrelite_etfs.raw";
static const string FILENAME_VIP = "PRODUCT_APP.L1";
static const string FILENAME_EEPROM = "PRODUCT.N07";

#ifndef DEBUG
#define DEBUG 1
#endif

#define LOG(format, ...) if (DEBUG) { \
	printf(format, ##__VA_ARGS__); \
}

// more info about crc, please visit http://www.ross.net/crc/download/crc_v3.txt

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

	// swap each byte0 and byte3
	for (int i = 0; i < len; i+=4) {
		*(data + i) ^= *(data + 2 + i);
		*(data + 2 + i) ^= *(data + i);
		*(data + i) ^= *(data + 2 + i);
	}
}

static uint32_t totalFileNum = 0;
static uint32_t crtFileNum = 0;

class FileInfo {
public:
	FileInfo(const string name, uint32_t startIndex, uint32_t fileNum) {
		mName = name;
		mStartIndex = startIndex;
		mFileIndex = fileNum;
		setFileType();
		setFileSize();
		setPackNum();
	}

	enum FILETYPE {
		FILETYPE_GIP = 0,
		FILETYPE_VIP,
		FILETYPE_EEPROM
	};

	string getName() {
		return mName;
	}

	uint32_t getPackNum() {
		return mPackNum;
	}

	uint32_t getFileSize() {
		return mSize;
	}

	uint32_t getStartIndex() {
		return mStartIndex;
	}

	FILETYPE getFileType() {
		return mType;
	}

	void splitFile();

private:
	void setFileSize() {
		struct stat sb;
		stat(mName.c_str(), &sb);
		mSize = sb.st_size;

		LOG("file %s size %d\n", mName.c_str(), mSize);
	}

	void setPackNum() {
		mPackNum = (mSize + 4) / DATA_SIZE + ((mSize+4) % DATA_SIZE ? 1 : 0); // add 4 for file crc code
	
		LOG("file %s packNum %d\n", mName.c_str(), mPackNum);
	}

	string& getBakFileName() {
		return mBakFileName;
	}

	void generateBakFile();

	void setFileType() {
		if (mName == FILENAME_GIP) {
			mType = FILETYPE_GIP;
		}
		else if (mName == FILENAME_VIP) {
			mType = FILETYPE_VIP;
		}
		else if (mName == FILENAME_EEPROM) {
			mType = FILETYPE_EEPROM;
		}
		else {
			std::cout << "file type else" << std::endl;
		}

		LOG("file %s filetype %d\n", mName.c_str(), mType);
	}

	string mName;
	string mBakFileName;
	uint32_t mSize;
	uint32_t mPackNum;
	uint32_t mStartIndex;
	uint32_t mFileIndex;
	FILETYPE mType;
};

class PackageHead {
public:
	PackageHead() {
		crc = 0xffffffff;
		index = 0;
		len = 0;
		fileID = 0;
	}

	void generateFileID(int, int, FileInfo::FILETYPE);

	uint32_t crc;
	uint32_t index;
	uint32_t len;
	uint32_t fileID;
};

void
PackageHead::generateFileID(int bFirstPackage, int bLastPackage, FileInfo::FILETYPE type) {
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

	if (1 == totalFileNum && 1 == crtFileNum) {
		if (bFirstPackage && bLastPackage)
			fileID |= 1 << 26;
		else if (bFirstPackage)
			fileID |= 1 << 24;
		else if (bLastPackage)
			fileID |= 1 << 25;
	}
	else {
		if (1 == crtFileNum) {
			if (bFirstPackage)
				fileID |= 1 << 24;
		}
		else if (crtFileNum == totalFileNum) {
			if (bLastPackage)
				fileID |= 1 << 25;
		}
	}

	if (type == FileInfo::FILETYPE_GIP) {
		fileID |= 1 << 16;
	}
	else if (type == FileInfo::FILETYPE_VIP) {
		fileID |= 1 << 17;
	}
	else if (type == FileInfo::FILETYPE_EEPROM) {
		fileID |= 1 << 18;
	}
	else {
		LOG("type error\n");
	}
}

class Package {
public:
	Package(uint32_t index, uint32_t startIndex, unsigned char *p, uint32_t len, int bFirst, int bLast, FileInfo::FILETYPE type) {
		setName(startIndex + index - 1);
		mData = p;
		mHead.index = startIndex + index - 1;
		mHead.len = len;
		mHead.generateFileID(bFirst, bLast, type);
	}

	string getName() {
		return mPackName;
	}

	void generatePackage();

private:
	void setName(uint32_t index) {
		char tmpName[32];
		sprintf(tmpName, "%s%04d", fileNameHead, index);
		LOG("%s\n", tmpName);
		mPackName = tmpName;
	}

	void formatData(unsigned char *);
	int insertInvalidByte(unsigned char *, unsigned char*, int);


	string mPackName;
	unsigned char *mData;
	PackageHead mHead;
};

int 
Package::insertInvalidByte(unsigned char *dest, unsigned char *src, int readCount)
{
	int len = 0;

	while (readCount > 0) {
		int size;
		if (readCount >= 3)
			size = 3;
		else
			size = readCount;

		memcpy(dest, src, size);
		dest += 4;
		len += 4;
		src += size;
		readCount -= size;
	}

	return len;
}

void
Package::formatData(unsigned char *buf)
{
	unsigned char tmp_buf[DATA_SIZE + HEAD_SIZE] = {0};
	memcpy(tmp_buf + HEAD_SIZE, mData, mHead.len);
	memcpy(tmp_buf, &mHead, sizeof(PackageHead));

	// calculate crc result
	uint32_t crc = 0xFFFFFFFF;
	mHead.crc = crc_calculate(tmp_buf + sizeof(uint32_t), mHead.len + 3*sizeof(uint32_t), crc);
	LOG("crc %#010x length %d index %d fileId %#010x\n", mHead.crc, mHead.len, mHead.index, mHead.fileID);

	memcpy(tmp_buf, &mHead.crc, sizeof(uint32_t)); // append crc into head.

	unsigned char fmt_buf[FORMAT_DATA_SIZE] = {0};
	int len = insertInvalidByte(fmt_buf, tmp_buf, mHead.len + HEAD_SIZE); // insert invalid byte

	unsigned char *dest = buf; 
	unsigned char *src = fmt_buf;
	int hasCopy = 0; 
	int size = 0;

	while (hasCopy < len) {
		size = DATA_LINE_SIZE;
		
		if (hasCopy + size > len)
			size = len - hasCopy;

		memcpy(dest, src + hasCopy, size);

		hasCopy += size;
		dest += (DATA_LINE_SIZE + LINE_OFFSET);
	}
	LOG("length %d hasCopy %d crc %#010x\n", len, hasCopy, mHead.crc);

	byte_swap((uint8_t*)buf, BLOCK_SIZE);
}

void
Package::generatePackage()
{
	if (NULL == mData) {
		LOG("%s mData is null\n", __FUNCTION__);
		return;
	}

	unsigned char buf[BLOCK_SIZE] = {0};
	formatData(buf);

	FILE *fpPack;
	if (NULL == (fpPack = fopen(this->getName().c_str(), "w"))) {
		LOG("fopen write failed! %s", this->getName().c_str());
		return;
	}

	if (BLOCK_SIZE != fwrite(buf, 1, BLOCK_SIZE, fpPack)) {
		LOG("fwrite failed!\n");
		return;
	}
}

void
FileInfo::generateBakFile()
{
	FILE *fp;
	if (NULL == (fp = fopen(this->getName().c_str(), "r"))) {
		LOG("fopen read failed! %s", this->getName().c_str());
		return;
	}

	FILE *fpBak;
	char bakName[32];
	sprintf(bakName, "%s_bak", this->getName().c_str());
	this->mBakFileName = bakName;
	if (NULL == (fpBak = fopen(mBakFileName.c_str(), "w"))) {
		LOG("fopen write failed! %s", mBakFileName.c_str());
		return;
	}

	unsigned char buf[PAGE_SIZE] = {0};
	int n;
	uint32_t crc = 0xffffffff;
	while (0 != (n = fread(buf, 1, PAGE_SIZE, fp))) {
		if (n != fwrite(buf, 1, n, fpBak)) {
			LOG("fwrite %s failed", mBakFileName.c_str());
			return;
		}
		crc = crc_calculate(buf, n, crc);
	}

	if (sizeof(uint32_t) != fwrite(&crc, 1, sizeof(uint32_t), fpBak)) {
		LOG("append failed!");
		return;
	}

	fclose(fp);
	fclose(fpBak);
}

void
FileInfo::splitFile()
{
	LOG("%s %s\n", __FUNCTION__, this->getName().c_str());

	// generate backup file and append crc checksum
	generateBakFile();

	FILE *fpBak;
	if (NULL == (fpBak = fopen(this->getBakFileName().c_str(), "r"))) {
		LOG("fopen fpBak failed! %s\n", this->getBakFileName().c_str());
		return;
	}

	unsigned char buf[DATA_SIZE] = {0};
	for (int i = 1; i <= this->getPackNum(); ++i) {
		int readCount;
		if (0 == (readCount = fread(buf, 1, DATA_SIZE, fpBak))) {
			if (ferror(fpBak)) {
				LOG("fread faild!");
			}
		}

		int bFirst = i == 1 ? 1 : 0;
		int bLast = (i == this->getPackNum()) ? 1 : 0;

		Package pack(i, this->mStartIndex, buf, readCount, bFirst, bLast, this->mType);
		pack.generatePackage();
	}

	unlink(this->getBakFileName().c_str());
	fclose(fpBak);
}

int main(int argc, char const *argv[])
{
	if (1 == argc) {
		std::cout << "please input file as parameter" << std::endl;
		return -1;
	}

	FileInfo* file[argc -1];

	int startIndex = 1;
	for (int i = 1; i != argc; ++i) {
		file[i-1] = new FileInfo(argv[i], startIndex, i);
		startIndex += file[i-1]->getPackNum();
		LOG("%s will generate %d package\n", file[i-1]->getName().c_str(), file[i-1]->getPackNum());
	}

	totalFileNum = argc - 1;

	for (int i = 0; i != argc - 1; ++i) {
		int isFirst = (i == 0) ? 1 : 0;
		int isLast = (i == argc - 2) ? 1 : 0;

		crtFileNum = i + 1;

		file[i]->splitFile();
	}

	return 0;
}
