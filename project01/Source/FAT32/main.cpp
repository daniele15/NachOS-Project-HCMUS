#include "FAT32.h"

int main() {
	BYTE sector[512];
	BYTE FAT01[512];
    FAT32_BOOTSECTOR boot_sector;

	// Input disk
	wchar_t disk_path[] = L"\\\\.\\?:";
	wcout << L"Disk letter: ";
	wcin >> disk_path[4];

	// Start reading disk
    if (readSector(disk_path, 0, sector)) {
        wcout << L"Cant Read Disk" << endl;
        return 1;
    }

	// Show File Allocation Table
	showFAT(sector);

	//Put bytes in sector buffer into FAT32_BOOTSECTOR struct
	memcpy(&boot_sector, sector, sizeof(FAT32_BOOTSECTOR));
	printBootSectorInfo(boot_sector);

	uint32_t sb = bytesToInt(boot_sector.BPB_RsvdSecCnt, 2), 
		sc = uint32_t(boot_sector.BPB_SecPerClus),
		nf = uint32_t(boot_sector.BPB_NumFATs), 
		sf = bytesToInt(boot_sector.BPB_FATSz32, 4);
	uint32_t first_sector_of_data = sb + nf * sf;

	// read FAT1
	readSector(disk_path, sb * 512, FAT01);

	
	// Read RDET as well as item property at once
	wcout << "Disk items' simple properies:\n\n";
	read_RDET(disk_path, first_sector_of_data, 0, FAT01, sc, first_sector_of_data);
	
	// Print folder tree
	// *****UPDATED*******
	wcout << "Folder Tree:\n\t|" << disk_path[4] << ":\n";
	printTree();
	
	
	// This is a test to run any type of files
	// replace 8 as the number of the file in the item list
	// For fun only
	//openFile(disk_path, 8);

	system("pause");

	return 0;
}