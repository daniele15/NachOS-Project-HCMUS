#pragma once

#include <windows.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <fstream>
#include <queue>

#define MIN_LEVEL 0

using namespace std;

typedef struct
{
    uint8_t BS_jmpBoot[3];              // 0x00: Jump instruction to start executing code
    uint8_t BS_OEMName[8];              // 0x03: OEM manufacturer name
    uint8_t BPB_BytsPerSec[2];          // 0x0B: Bytes per sector
    uint8_t BPB_SecPerClus;             // 0x0D: Sectors per cluster
    uint8_t BPB_RsvdSecCnt[2];          // 0x0E: Reserved sector count
    uint8_t BPB_NumFATs;                // 0x10: Number of File Allocation Tables (FATs)
    uint8_t BPB_RootEntCnt[2];          // 0x11: Number of root directory entries
    uint8_t BPB_TotSec16[2];           // 0x13: Total sectors (16-bit) - for FAT16
    uint8_t BPB_Media;                  // 0x15: Media type (e.g., hard disk)
    uint8_t BPB_FATSz16[2];             // 0x16: FAT size (16-bit) - for FAT16
    uint8_t BPB_SecPerTrk[2];           // 0x18: Sectors per track
    uint8_t BPB_NumHeads[2];            // 0x1A: Number of heads
    uint8_t BPB_HiddenSec[4];           // 0x1C: Hidden sectors before the boot sector
    uint8_t BPB_TotSec32[4];            // 0x20: Total sectors (32-bit) - for FAT32
    uint8_t BPB_FATSz32[4];             // 0x24: FAT size (32-bit) - for FAT32
    uint8_t BPB_ExtFlags[2];            // 0x28: Extended flags
    uint8_t BPB_FSVer[2];               // 0x2A: File system version
    uint8_t BPB_RootClus[4];            // 0x2C: First cluster of root directory
    uint8_t BPB_FSInfo[2];              // 0x30: FSInfo sector
    uint8_t BPB_BkBootSec[2];           // 0x32: Backup boot sector
    uint8_t BPB_Reserved[12];           // 0x34: Reserved bytes
    uint8_t BS_DrvNum;                  // 0x40: Drive number
    uint8_t BS_Reserved1;               // 0x41: Reserved byte
    uint8_t BS_BootSig;                 // 0x42: Boot signature
    uint8_t BS_VolID[4];                // 0x43: Volume ID
    uint8_t BS_VolLab[11];              // 0x47: Volume label
    uint8_t BS_FilSysType[8];           // 0x52: File system type (usually "FAT32")
    uint8_t reservedCode[420];          // 0x5A: Reserved bytes
    uint8_t endSignal[2];               // 0x1FE: End of the boot sector
} FAT32_BOOTSECTOR, * PFAT32_BOOTSECTOR;


typedef struct
{
	uint8_t fileName[8];
	uint8_t fileExtention[3];
	uint8_t fileAttribute;
	uint8_t resevered[10];
	uint8_t lastModifiedTime[2];
	uint8_t lastModifiedDate[2];
	uint8_t startCluster[2];
	uint8_t fileSize[4];
}FAT32_MAINENTRY, * PFAT32_MAINENTRY;

typedef struct
{
	uint8_t seqNum;
	uint8_t fileName1[10];
	uint8_t fileAttribute;
	uint8_t fileType;
	uint8_t fileCheckSum;
	uint8_t fileName2[12];
	uint8_t startCluster[2];
	uint8_t fileName3[4];
}FAT32_SUBENTRY, * PFAT32_SUBENTRY;

class ITEM {
public:
	wstring itemName;
	int itemType;					// 1. Folder, 0. File
	uint32_t itemSize;
	uint32_t firstCluster;
    uint32_t lastCluster;
	int level;
};

wstring bytesToHex(BYTE byte);
void showFAT(BYTE* bs);
int readSector(LPCWSTR drive, int readPoint, BYTE sector[512]);
uint32_t bytesToInt(const uint8_t bytes[], int n);
wstring bytesToWString(const uint8_t bytes[], int n);
void printBootSectorInfo(const FAT32_BOOTSECTOR& boot_sector);
ITEM getNewItem(wstring name, uint32_t size, uint32_t firstCluster,  int type, int level);
void read_RDET(LPCWSTR disk, uint32_t sector_index, int level, BYTE FAT[512], uint32_t sc, uint32_t first_sector_of_data);
void printTree();
void openFile(LPCWSTR disk_path, int idx);
void printTXTContent(LPCWSTR disk, uint32_t cluster);
void processFolderOrFile(LPCWSTR disk, BYTE rdet[512], uint32_t pointer, wstring& fileName, BYTE FAT[512], uint32_t sc, uint32_t first_sector_of_data, int level);