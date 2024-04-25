#include "NTFSModule.h"

void ntfsAnalysis(BYTE* sector, NTFSVolume& vol)
{
	if (!vol.load) {
		cout << "Boot Sector info not loaded. Proceeding to load." << endl;
		loadSectorInfo(sector, vol);
	}
	BPB* p = &vol.sectorInfo;
	cout << "Bytes per Sector: " << p->bytesPerSector << endl;
	cout << "Sectors per Cluster: " << p->sC << endl;
	cout << "Reserved sectors: " << p->reservedSectors << endl;
	cout << "Media Descriptor:" << p->mediaDescriptor << endl;
	cout << "Sectors per Track: " << p->sT << endl;
	cout << "Number of heads: " << p->nHeads << endl;
	cout << "Hidden Sectors: " << p->hiddenSectors << endl;
	cout << "Total Sectors: " << p->totalSectors << endl;
	cout << "Starting cluster to MFT: " << p->mftCNum << endl;
	cout << "Starting cluster to MFT mirror: " << p->mftCNum_2 << endl;
	cout << "Size of File Record Segment: " << p->CPFRS << endl;
	cout << "Index Buffer size in Clusters: " << p->CPIB << endl;
	cout << "Volume Serial Number: "; 
	asciiCodedPrint(p->serialNum);
	cout << endl << "Checksum (unused by NTFS): " << p->checksum << endl;
}

void loadSectorInfo(BYTE* sector, NTFSVolume& vol)
{
	BPB* p = &vol.sectorInfo;
	p->bytesPerSector = bytesToDecimal(readInfo(11, 2, sector));
	p->sC = bytesToDecimal(readInfo(13, 1, sector));
	p->reservedSectors = bytesToDecimal(readInfo(14, 2, sector));
	p->mediaDescriptor = bytesToDecimal(readInfo(21, 1, sector));
	p->sT = bytesToDecimal(readInfo(24, 2, sector));
	p->nHeads = bytesToDecimal(readInfo(26, 2, sector));
	p->hiddenSectors = bytesToDecimal(readInfo(28, 4, sector));
	p->totalSectors = bytesToDecimal(readInfo(40, 8, sector));
	p->mftCNum = bytesToDecimal(readInfo(48, 8, sector));
	p->mftCNum_2 = bytesToDecimal(readInfo(56, 8, sector));

	// Special case: CPFRS hex value -> binary value(two's complement)-> int
	BYTE b = sector[64];
	p->CPFRS = char(b);
	if (p->CPFRS < 0) {
		p->CPFRS *= -1;
		p->CPFRS = 1 << (p->CPFRS);
	}

	p->CPIB = bytesToDecimal(readInfo(68, 1, sector));
	p->serialNum = bytesToDecimal(readInfo(72, 8, sector));
	p->checksum = bytesToDecimal(readInfo(80, 4, sector));

	vol.load = 1;
}

void loadEntries(LPCWSTR drive, BYTE* block, NTFSVolume& vol)
{
	// Check if the block passed in is the $MFT
	uint8_t name[50];
	getFilename(block, name);

	BPB* p = &vol.sectorInfo;

	if (strcmp(reinterpret_cast<char*>(name), "$MFT") == 0) {
		// Scan through the MFT entries.
		int jump = 1024; // size of MFT
		
		int loop = 1;

		uint64_t offset =	vol.sectorInfo.bytesPerSector* 
							vol.sectorInfo.mftCNum*
							vol.sectorInfo.sC;
		uint8_t filename[50];

		uint64_t limit = endOfMFT(drive, vol) * 8;
		uint64_t rp = offset;
		do {

			BYTE* entry;
			entry = ReadBytes(drive, rp , jump);
			getFilename(entry, filename);
			// Check signature

			// Push any entry that is valid

			if (bytesToDecimal(readInfo(0, 4, entry)) == 0x454C4946) {

				MFTEntry obj;
				analyze(entry, obj);
				readData(drive, entry, obj);
				vol.entries.push_back(obj);
			}
			rp += 1024;
			delete[]entry;
		} while (rp / 512<= limit);
	}
}

// Tinh toan diem ket thuc MFT
uint64_t endOfMFT(LPCWSTR drive, NTFSVolume& vol)
{
	BYTE* MFT = new BYTE[1024];
	uint64_t pos = uint64_t(vol.sectorInfo.mftCNum) * vol.sectorInfo.sC * vol.sectorInfo.bytesPerSector;

	MFT = ReadBytes(drive, pos, vol.sectorInfo.CPFRS);

	MFTEntry obj;
	analyze(MFT,obj);

	BYTE* list;
	dataRunList(MFT,list,obj);

	uint8_t head = list[0];
	uint8_t l = (head >> 4) | 0;
	uint8_t r = head - (l << 4);

	uint64_t sz = bytesToDecimal(readInfo(1, r, list));
	return vol.sectorInfo.mftCNum + sz;
}

void outputEntries(NTFSVolume& vol)
{
	for (int i = 0; i < vol.entries.size(); i++) {
		printEntry(vol.entries[i]);
	}
}

void constructTree(Tree& tree, NTFSVolume& vol)
{
	int c_Passed = 0;
	int c_Failed = 0;

	// Tim cac file hien tren nhanh.
	for (int i = 0; i < vol.entries.size(); i++) {
		MFTEntry* p = &vol.entries[i];
		
		if ((p->filePermissionsFlag & Perms::System) == 0 && p->pId == 5) {
			tree.branches.push_back(new TreeNode{ p->pId,p->id,*p });
		}
	}

	
	for (int i = 0; i < tree.branches.size(); i++) {
		// Xet tung nhanh, xem co ton tai file nao thuoc nhanh do

		uint32_t init = tree.branches[i]->Id;

		for (int j = 0; j < vol.entries.size(); j++) {
			MFTEntry* p = &vol.entries[j];

			if ((p->filePermissionsFlag & Perms::System) == 0 && p->pId == tree.branches[i]->Id) {
				tree.branches[i]->nodes.push_back(new TreeNode{ p->pId,p->id,*p });
			}
		}
	}
}
