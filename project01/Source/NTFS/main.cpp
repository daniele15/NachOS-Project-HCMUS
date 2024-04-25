#include "NTFSModule.h"


int main(int argc, char** argv)
{
	LPCWSTR drive = L"\\\\.\\E:";
	BYTE sector[512];
	ReadSector(drive, 0, sector); // Reading VBR
	printBytes(sector, 512);

	NTFSVolume vol;
	ntfsAnalysis(sector,vol);

	BPB boot = vol.sectorInfo;

	uint64_t readPoint = uint64_t(boot.mftCNum) * uint64_t(boot.sC) * uint64_t(boot.bytesPerSector);

	vector<BYTE*> entries;

	BYTE* MFT = nullptr;
	MFT = ReadBytes(drive, readPoint, boot.CPFRS);

	loadEntries(drive, MFT, vol);

	outputEntries(vol);
	
	cout << "Chon file can doc: ";
	string buf;
	getline(cin, buf);
	uint8_t* data;
	for (int i = 0; i < vol.entries.size(); i++) {
		if (strcmp(buf.c_str(), reinterpret_cast<char*>(vol.entries[i].name)) == 0) {
			printData(vol.entries[i]);
			break;
		}
	}
	
	return 0;
}