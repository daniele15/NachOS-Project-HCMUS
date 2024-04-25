#pragma once
#include "master.h"
#include "MFTutil.h"

enum Perms {
	ReadOnly = 0x0001,
	Hidden = 0x0002,
	System = 0x0004,
	Archive = 0x0020,
	Device = 0x0040,
	Normal = 0x0080,
	Temporary = 0x0100,
	Sparse = 0x0200,
	Reparse = 0x0400,
	Compressed = 0x0800,
	Offline = 0x1000,
	NotIndexed = 0x2000,
	Encrypted = 0x4000
};

enum Format {
	POSIX = 0,
	Win32 = 1,
	DOS = 2
};

// TODO: Loc nhung file co co bao system qua phan $STANDARD_INFORMATION
struct MFTEntry
{
	// Thuoc tinh co ban:
	uint16_t flag; // Trang thai cua Entry
	uint8_t name[50]; // Ten file
	uint64_t size; // in bytes
	uint32_t id;
	uint32_t pId;

	// Co bao system
	uint32_t filePermissionsFlag; // offset 0x70, 4

	// Thuoc tinh nang cao
	uint32_t sign; // Dau hieu nhan biet
	uint16_t startAttribute; // Offset bat dau cua phan attribute
	uint32_t entrySize; // kich thuoc entry (offset 18->1B) - dung de danh dau ket thuc entry
	uint32_t stdInfoSize; // Kich thuoc STANDARD_INFORMATION
	uint16_t nameOffset; // Offset bat dau cua $FILE_NAME
	uint8_t nameLen; // Chieu dai ten file
	uint16_t dataOffset;
	uint16_t nonresFlag;

	uint8_t* data = NULL;
};

void analyze(BYTE* block, MFTEntry& entry);
void printEntry(MFTEntry& entry);
void dataRunList(BYTE* block, BYTE*& list, MFTEntry& entry);
void readData(LPCWSTR drive, BYTE* block, MFTEntry& entry);
void printData(MFTEntry& entry);