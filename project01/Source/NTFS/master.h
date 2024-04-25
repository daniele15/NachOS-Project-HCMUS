#pragma once
#ifndef MASTER_H
#define MASTER_H
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <vector>
#include <iomanip>
#include <string>

using namespace std;

int ReadSector(LPCWSTR  drive, int readPoint, BYTE sector[512]);
void printBytes(BYTE* bytes, int n);
BYTE* ReadBytes(LPCWSTR, uint64_t, int);
void asciiCodedPrint(uint64_t);
unsigned bytesToDecimal(vector<BYTE>vec);
vector<BYTE> readInfo(int offset, int length, BYTE* sector);

#endif