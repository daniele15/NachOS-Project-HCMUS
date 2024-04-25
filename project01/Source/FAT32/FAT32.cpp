
#include "FAT32.h"

wchar_t _HEX_CHAR[16] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };
vector<ITEM> list_item;

// Converse 8-bit Integer into Hex String
wstring bytesToHex(BYTE byte) {
	wstring s = L"";
	int sum = (int)byte;

	s = _HEX_CHAR[sum / 16];
	s = s + _HEX_CHAR[sum % 16];

	return s;
}

//// Show File Allocation Table
//void showFAT(BYTE* bs) {
//	wcout << "\nFile Allocation Table:\n";
//	//wcout << L"|------------------------------------------------|\n";
//	for (int i = 0; i < 16; i++) {
//		wcout << setw(2) << i + 1 << ' ';
//	}
//	wcout << endl;
//
//	for (int i = 0; i < 512; i++) {
//		wcout << bytesToHex(bs[i]) << L' ';
//		if ((i + 1) % 16 == 0) {
//			wcout << endl;
//		}
//	}
//
//	wcout << "\n";
//}

void showFAT(BYTE* bytes)
{
	int n = 512;
	int col = 16;
	int rows = n / col;
	// Output boot sector details.

	printf("Offset\t\t");
	for (int i = 0; i < 16; i++) {
		printf("%02X ", i);
	}
	printf("\n\n");

	int i = 0, j = 0;

	for (i = 0; i < rows; i++) {
		printf("%08X\t", i);
		for (j = 0; j < col; j++) {
			printf("%02X ", bytes[i * 16 + j]);
		}

		printf("\t");
		for (j = 0; j < col; j++) {
			BYTE b = bytes[i * 16 + j];
			if (isprint(b)) {
				printf("%c", b);
			}
			else {
				printf(".");
			}

		}
		printf("\n");
	}
}

// Read sector and save into buffer
int readSector(LPCWSTR drive, int readPoint, BYTE sector[512])
{
	DWORD bytesRead;
	HANDLE device = NULL;

	device = CreateFile(drive,    // Drive to open
		GENERIC_READ,           // Access mode
		FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
		NULL,                   // Security Descriptor
		OPEN_EXISTING,          // How to create
		0,                      // File attributes
		NULL);                  // Handle to template

	if (device == INVALID_HANDLE_VALUE) // Open Error
	{
		return 1;
	}

	SetFilePointer(device, readPoint, NULL, FILE_BEGIN); //Set a Point to Read

	// Read here
	if (!ReadFile(device, sector, 512, &bytesRead, NULL)) // Read Error
	{
		CloseHandle(device);
		return 2;
	}
	else
	{
		CloseHandle(device);
		return 0;
	}
}

// Print Boot Sector Info
void printBootSectorInfo(const FAT32_BOOTSECTOR& boot_sector) {

	char fileSystemType[9];
	memcpy(fileSystemType, boot_sector.BS_FilSysType, 8);
	fileSystemType[8] = '\0'; 

	wcout << L"FAT32 Boot Sector Basic Info:\n";
	wcout << L"File system type                      :" << setw(13) << fileSystemType << endl;
	wcout << L"Bytes per sector                      :" << setw(10) << bytesToInt(boot_sector.BPB_BytsPerSec,2) << endl;
	wcout << L"Sector per cluster (Sc)               :" << setw(10) << (int)boot_sector.BPB_SecPerClus << " sector(s)" << endl;
	wcout << L"Sector in Boot Sector (Sb)            :" << setw(10) << bytesToInt(boot_sector.BPB_RsvdSecCnt, 2) << " sector(s)" << endl;
	wcout << L"Number of FATs (nF)                   :" << setw(10) << (int)boot_sector.BPB_NumFATs << " FAT(s)" << endl;
	wcout << L"Number of root directory entries (Sr) :" << setw(10) << bytesToInt(boot_sector.BPB_RootEntCnt,2) << " sector(s)" << endl;
	wcout << L"Size of volume (Sv)                   :" << setw(10) << bytesToInt(boot_sector.BPB_TotSec32, 4) << " sector(s)" << endl;
	wcout << L"Size of FAT (Sf)                      :" << setw(10) << bytesToInt(boot_sector.BPB_FATSz32, 4) << " sector(s)" << endl;
	wcout << L"Start cluster RDET                    :" << setw(10) << bytesToInt(boot_sector.BPB_RootClus, 4) << endl;
	wcout << L"Additional Information Sector         :" << setw(10) << bytesToInt(boot_sector.BPB_FSInfo, 2) << " sector(s)" << endl;
	wcout << L"Boot Sector's Backup Sectors          :" << setw(10) << bytesToInt(boot_sector.BPB_BkBootSec, 2) << " sector(s)" << endl << endl << endl;
}

// Converse 8-bit Integers into one 32-bit Integer
uint32_t bytesToInt(const uint8_t bytes[], int n) {
	uint32_t res = 0;
	for (int i = n - 1; i >= 0; i--) {
		res = (res << 8) | bytes[i];
	}
	return res;
}

// Converse 8-bit Integers into one string
wstring bytesToWString(const uint8_t bytes[], int n) {
	wstring result = L"";

	for (int i = 0; i < n - 1; i += 2) {
		if ((bytes[i] == 0 && bytes[i + 1] == 0) || (bytes[i] == 0xff && bytes[i + 1] == 0xff)) {
			break;
		}

		result += wchar_t(bytes[i]);
	}

	return result;
}

ITEM getNewItem(wstring name, uint32_t size, uint32_t firstCluster, int type, int level) {
	ITEM result;
	result.itemName = name;
	result.itemType = type;
	result.itemSize = size;
	result.firstCluster = firstCluster;
	result.level = level;
	return result;
}

void read_RDET(LPCWSTR disk, uint32_t sector_index, int level, BYTE FAT[512], uint32_t sc, uint32_t first_sector_of_data) {
	// Load RDET
	BYTE rdet[512];
	readSector(disk, sector_index * 512, rdet);

	wstring fileName = L"";

	// Set pointer to go through sector
	uint32_t pointer = 0;

	// Skip two first entries -> System
	pointer = 2 * 32; 

	do {
		// Read another sector
		if (pointer == 512) {
			pointer = 0;
			sector_index += 1;
			readSector(disk, sector_index * 512, rdet);
		}

		// Deleted sector
		if (rdet[pointer] == 0xE5) {
			pointer += 32;
			continue;
		}

		// Empty entry
		if (rdet[pointer + 11] == 0x00) {
			break;
		}
		// Sub Entry
		else if (rdet[pointer + 11] == 0x0F) {
			// Get name from Sub Entry
			fileName = bytesToWString(rdet + pointer + 1, 10) + bytesToWString(rdet + pointer + 14, 12) + bytesToWString(rdet + pointer + 28, 4) + fileName;
		}
		// Not File nor Folder
		else if (rdet[pointer + 11] != 0x10 && rdet[pointer + 11] != 0x20) {
			fileName = L"";
		}
		else {
			processFolderOrFile(disk, rdet, pointer, fileName, FAT, sc, first_sector_of_data, level);

		}

		pointer += 32;
	} while (true);
}
void processFolderOrFile(LPCWSTR disk, BYTE rdet[512], uint32_t pointer, wstring &fileName, BYTE FAT[512], uint32_t sc, uint32_t first_sector_of_data, int level) {
	FAT32_MAINENTRY mainEnt;
	memcpy(&mainEnt, rdet + pointer, 32);

	uint32_t first_cluster = bytesToInt(mainEnt.startCluster, 2);
	uint32_t last_cluster = first_cluster;

	while (true) {
		uint32_t FATmember = bytesToInt(FAT + last_cluster * 4, 4);
		if (FATmember == 0x0FFFFFFF || FATmember == 0x0FFFFFF8 || last_cluster == 0) {
			break;
		}
		else if (FATmember == 0x0FFFFFF7 || FATmember == 0) {
			//wcout << L"    FAT Table deleted or empty.\n";
			break;
		}
		else if (FATmember == 0xCCCCCCCC) {
			wcout << L"Unable to read FAT Table -> Uathority\nPlease run as administator.";
			break;
		}
		else last_cluster = FATmember;
	}

	// Folder
	if (mainEnt.fileAttribute == 0x10) {
		// Main Entry name
		if (fileName == L"") fileName = bytesToWString(mainEnt.fileName, 8);

		// Get Item to list -> Easier to mange
		ITEM item = getNewItem(fileName, 0, (first_cluster - 2) * sc + first_sector_of_data, 1, level);
		list_item.push_back(item);

		// Print Item's properties
		wcout << L" *Item: " << fileName << endl;
		wcout << L"    Type: Folder" << endl;
		wcout << L"    First Cluster: " << first_cluster << endl;
		wcout << L"    Cluster list: ";
		for (int i = first_cluster; i <= last_cluster; i++) {
			wcout << i;
			if (i != last_cluster) wcout << L", ";
		}
		wcout << endl << L"    Sector list: ";
		for (int i = item.firstCluster; i <= (last_cluster - 1) * sc + first_sector_of_data; i++) {
			wcout << i;
			if (i != (last_cluster - 1) * sc + first_sector_of_data) cout << ", ";
		}
		wcout << endl << endl;
		fileName = L"";

		// Read Sub Folder
		if (bytesToInt(rdet + pointer + 28, 4) == 0) {
			read_RDET(disk, item.firstCluster, level + 1, FAT, sc, first_sector_of_data);
		}
	}
	// File
	else if (mainEnt.fileAttribute == 0x20) {
		// Main Entry Name
		if (fileName == L"") fileName = bytesToWString(mainEnt.fileName, 8);

		// Get Item to list -> Easier to mange
		ITEM item = getNewItem(fileName, 0, (first_cluster - 2) * sc + first_sector_of_data, 0, level);
		list_item.push_back(item);

		// Print Item's properties
		wcout << L" *Item: " << fileName << endl;
		wcout << L"    Type: File" << endl;
		wcout << L"    Size: " << bytesToInt(mainEnt.fileSize, 4) << L" B" << endl;
		wcout << L"    First Cluster: " << first_cluster << endl;
		wcout << L"    Cluster list: ";
		for (int i = first_cluster; i <= last_cluster; i++) {
			wcout << i;
			if (i != last_cluster) wcout << L", ";
		}
		wcout << endl << L"    Sector list: ";
		for (int i = item.firstCluster; i <= (last_cluster - 1) * sc + first_sector_of_data; i++) {
			wcout << i;
			if (i != (last_cluster - 1) * sc + first_sector_of_data) cout << ", ";
		}

		// If file is .txt file
		if (fileName.substr(fileName.size() - 3, 3) == L"txt") {
			wcout << endl;
			printTXTContent(disk, item.firstCluster);
		}
		else {
			//wcout << endl << L"    ID: " << list_item.size() - 1;
			wcout << endl << L"    Cannot open file content!";
		}

		wcout << endl << endl;
		fileName = L"";
	}
}
// Print Tree
void printTree() {
	int i = 0;
	while (i < list_item.size()) {
		ITEM tmp = list_item[i];

		wcout << L"\t";
		for (int i = -2; i < tmp.level - 1; i++) {
			wcout << L"|   ";
		}
		wcout << L"|";
		wcout << tmp.itemName << endl;

		i++;
	}
}

// Open File With Program Associated with its extension
void openFile(LPCWSTR disk_path, int idx) {
	wstring result = L"";
	result += disk_path[4];
	result += L":\\";

	int ptr = 0;
	int count = MIN_LEVEL;
	for (int i = idx - 1; i >= ptr; i++) {
		if (count == list_item[idx].level) {
			result += L"\\" + list_item[idx].itemName;
			break;
		}

		if (list_item[i].level == count && list_item[i].itemType == 1) {
			result += L"\\" + list_item[i].itemName ;
			count++;
			ptr = i;
			i = idx - 1;
		}
	}

	ShellExecute(NULL, 0, result.c_str(), 0, 0, SW_SHOW);


	//return result;
}

// Print .txt file context
void printTXTContent(LPCWSTR disk, uint32_t cluster) {
	BYTE data[512];
	readSector(disk, cluster * 512, data);
	wstring result = L"";

	for (int i = 0; i < 512; i++) {
		if ((data[i] == 0 && data[i + 1] == 0) || (data[i] == 0xff && data[i + 1] == 0xff)) {
			break;
		}

		result += wchar_t(data[i]);
	}
	
	wcout << L"    Content: " << result << endl;
}
