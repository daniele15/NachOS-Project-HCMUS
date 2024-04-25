#include "MFTEntry.h" 


void analyze(BYTE* block, MFTEntry& entry)
{
	for (int i = 0; i < 4; i++) {
		entry.sign = (entry.sign << 8) | block[i];
	}
	entry.id = bytesToDecimal(readInfo(0x2C, 4, block));
	entry.startAttribute = bytesToDecimal(readInfo(0x14, 2, block));
	entry.flag = bytesToDecimal(readInfo(0x16, 2, block));
	entry.entrySize = bytesToDecimal(readInfo(0x18, 4, block));
	entry.stdInfoSize = bytesToDecimal(readInfo(entry.startAttribute + 4, 4, block));

	entry.filePermissionsFlag = bytesToDecimal(readInfo(0x70, 4, block));


	int nameO = entry.stdInfoSize + entry.startAttribute;
	int fnSize = bytesToDecimal(readInfo(nameO + 4, 4, block));

	entry.nameOffset = bytesToDecimal(readInfo(nameO + 20, 2, block));

	nameO += entry.nameOffset;

	entry.pId = bytesToDecimal(readInfo(nameO, 6, block));
	entry.nameLen = bytesToDecimal(readInfo(nameO + 64, 1, block));

	getFilename(block, entry.name);

	// Kiem tra attribute sau co typeid = 128?
	int nextAttributeOffset = entry.startAttribute + entry.stdInfoSize + fnSize;
	
	entry.size = 0;

	bool B1 = (entry.flag & 2) != 2 && (entry.flag & 4) != 4 && (entry.flag & 8) != 8;

	if (B1) // Phat hien co bao thu muc
	{
		int id = -1;
		do
		{
			id = bytesToDecimal(readInfo(nextAttributeOffset, 4, block));
			if (id == 128) break;

			nextAttributeOffset += bytesToDecimal(readInfo(nextAttributeOffset + 4, 4, block));

		} while (id != 128); // Dung khi tim thay id = 128

		entry.dataOffset = nextAttributeOffset;

		uint8_t nonRes = bytesToDecimal(readInfo(entry.dataOffset + 8, 1, block));

		if (nonRes == 0)
			entry.size = bytesToDecimal(readInfo(entry.dataOffset + 16, 4, block));
		else
		{
			uint16_t offset = entry.dataOffset + 64;
			uint8_t head = bytesToDecimal(readInfo(offset, 1, block)); // 0x21 = 0010 0001
			uint8_t l = (head >> 4) | 0;							   //    l = 0000 0010
			uint8_t r = head - (l << 4);							   //    r = 0000 0001

			entry.size = bytesToDecimal(readInfo(offset + 1, r, block)) * 4096;
		}
	}
	else entry.dataOffset = -1; // Bo qua data.
}

void printEntry(MFTEntry& entry)
{
	cout << "------------------------------------------------------" << endl;
	cout << "Signature: ";
	asciiCodedPrint(entry.sign);
	cout << endl;
	cout << "Is directory: " << ((entry.flag & 2)==2) << endl;
	cout << "Size: " << entry.size << endl;
	cout << "ID: " << entry.id << endl;
	cout << "Parent ID: " << entry.pId << endl;
	printf("Name: %s\n", entry.name);
}

void dataRunList(BYTE* block, BYTE*& list, MFTEntry& entry)
{
	if (entry.dataOffset != -1) {
		uint16_t offset = entry.dataOffset + 64;
		if (bytesToDecimal(readInfo(entry.dataOffset, 4, block)) != 128)
			return;
		uint8_t head = bytesToDecimal(readInfo(offset, 1, block)); // 0x21 = 0010 0001
		uint8_t l = (head >> 4) | 0;							   //    l = 0000 0010
		uint8_t r = head - (l << 4);							   //    r = 0000 0001

		list = new BYTE[1 + l + r];
		for (int i = 0; i < 1 + l + r; i++) {
			list[i] = block[offset + i];
		}
	}
}

void readData(LPCWSTR drive, BYTE* block, MFTEntry& entry)
{
	if (strstr(reinterpret_cast<char*>(entry.name), ".txt") == NULL) {
		cout << "Data not readable." << endl;
	}
	if (entry.dataOffset != -1) {
		uint8_t nonRes = bytesToDecimal(readInfo(entry.dataOffset + 8, 1, block));

		uint16_t f = entry.flag;
		bool bits[4] = { f & 1,f & 2, f & 4,f & 8 };

		// Kiem tra entry co phai la cua thu muc?
		if (bits[1]) {
			return;
		}

		if (nonRes == 0) {
			// Doc data thuoc loai resident
			uint16_t offset = entry.dataOffset + bytesToDecimal(readInfo(entry.dataOffset + 10, 2, block));
			uint32_t size = bytesToDecimal(readInfo(entry.dataOffset + 16, 4, block));

			entry.data = new uint8_t[size + 1];
			for (int i = 0; i < size; i++) {
				entry.data[i] = block[offset + i];
			}
			entry.data[size] = '\0';

			entry.size = size;
		}
		else
		{
			uint16_t offset = entry.dataOffset + 64;
			uint8_t head = bytesToDecimal(readInfo(offset, 1, block)); // 0x21 = 0010 0001
			uint8_t l = (head >> 4) | 0;							   //    l = 0000 0010
			uint8_t r = head - (l << 4);							   //    r = 0000 0001

			uint32_t size = bytesToDecimal(readInfo(offset + 1, r, block)) * 4096;
			uint32_t firstClus = bytesToDecimal(readInfo(offset + r + 1, l, block));

			entry.data = new uint8_t[size + 1];
			entry.data = ReadBytes(drive, uint64_t(firstClus) * 4096, size);
			entry.data[size] = '\0';
			entry.size = size;
		}
	}
	else {
		cout << "Entry data not readable." << endl;
	}
}

void printData(MFTEntry& entry)
{
	if (entry.data != NULL) {
		printf("%s", entry.data);
	}
}

