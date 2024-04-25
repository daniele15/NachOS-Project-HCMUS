#pragma once
#ifndef NTFSMODULE_H
#define NTFSMODULE_H

#include "master.h"
#include "MFTEntry.h"

struct BPB {
	unsigned int bytesPerSector;
	unsigned int sC;
	unsigned int reservedSectors;
	unsigned int mediaDescriptor;
	unsigned int sT;
	unsigned int nHeads;
	unsigned int hiddenSectors;
	unsigned int totalSectors;
	unsigned int mftCNum;
	unsigned int mftCNum_2;
	int CPFRS;
	unsigned int CPIB;
	unsigned int serialNum;
	unsigned int checksum;
};

struct NTFSVolume {
	BPB sectorInfo = {};
	int load = 0;
	vector<MFTEntry> entries;
};

struct TreeNode {
	uint32_t parentId;
	uint32_t Id;
	MFTEntry entry;
	vector<TreeNode*> nodes;
};

struct Tree { // Xem tree da co root node = 5
	vector<TreeNode*> branches;
};

void ntfsAnalysis(BYTE* sector, NTFSVolume& vol);
void loadSectorInfo(BYTE*, NTFSVolume& vol);
void loadEntries(LPCWSTR drive, BYTE* block, NTFSVolume& vol); // 
uint64_t endOfMFT(LPCWSTR drive, NTFSVolume& vol);
void outputEntries(NTFSVolume& vol);
void constructTree(Tree& tree, NTFSVolume& vol); // Cay thu muc chi hien thi nhung file khong thuoc he thong.


#endif // !NTFSMODULE_H
