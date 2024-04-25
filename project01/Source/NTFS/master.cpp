#include "master.h"

/// <summary>
/// Sample Read Sector code
/// </summary>
/// <param name="drive"> String format: L"\\\\.\\[Disk (C,D,E,F)]:"</param>
/// <param name="readPoint"> Read offset start </param>
/// <param name="sector"> Array containing sector contents </param>
/// <returns> Error state code </returns>
int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512])
{
	int retCode = 0;
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
		printf("CreateFile: %u\n", GetLastError());
		return 1;
	}

	DWORD newPositionHigh = 0;

	SetFilePointer(device, readPoint, reinterpret_cast<PLONG>(&newPositionHigh), FILE_BEGIN);//Set a Point to Read

	if (!ReadFile(device, sector, 512, &bytesRead, NULL))
	{
		printf("ReadFile: %u\n", GetLastError());
	}
	else
	{
		printf("Success!\n");
	}
}

/// <summary>
/// Sector printer
/// </summary>
/// <param name="sector"> Works for any pointer</param>
void printBytes(BYTE* bytes, int n)
{
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

BYTE* ReadBytes(LPCWSTR drive, uint64_t readPoint, int numberOfBytes) {

	BYTE* bytes = new BYTE[numberOfBytes];
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
		printf("CreateFile: %u\n", GetLastError());
		return nullptr;
	}

	LONG high = readPoint >> 32;
	SetFilePointer(device, readPoint, &high, FILE_BEGIN);//Set a Point to Read

	if (!ReadFile(device, bytes, numberOfBytes, &bytesRead, NULL))
	{
		printf("ReadFile: %u\n", GetLastError());
		return nullptr;
	}
	else
	{
		return bytes;
	}
}

void asciiCodedPrint(uint64_t n)
{
	size_t loops = sizeof(n) / sizeof(uint8_t);
	// 0x 0F A2 98 44 = (00001111 10100010 10011000 01000100)
	uint8_t* arr = new uint8_t[loops];
	for (int i = 0; i < loops; i++) {
		arr[loops - i - 1] = n & 0xFF;
		n >>= 8;
	}

	for (int i = 0; i < loops; i++) {
		if (isprint(arr[i]))
			printf("%c", arr[i]);
	}
	delete[]arr;
}

unsigned bytesToDecimal(vector<BYTE> vec)
{
	// Little endian format
	unsigned result = 0;

	for (int i = vec.size() - 1; i >= 0; i--) {
		result = result * 256 + vec[i];
	}

	return result;
}

vector<BYTE> readInfo(int offset, int length, BYTE* sector)
{
	vector<BYTE> bytes;
	for (int i = 0; i < length; i++) {
		bytes.push_back(sector[offset + i]);
	}

	return bytes;
}
