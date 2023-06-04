#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include "Buffer.h"
#include "Record.h"
#include "Mode.h"

#pragma once
class Tape
{
	public:
		Tape(int bufferSize, std::string filename);
		~Tape();
		Record read();
		bool write(Record record);
		void displayTape();
		void displayTapeRecords();
		void clearBuffer();
		bool isBufferEmpty();
		int getReadDiskCount();
		int getWriteDiskCount();
		bool saveBuffer();
		void changeMode(tapeMode::Mode mode);
	private:
		bool fillBuffer(); // odczyt z dysku
		bool saveBufferOnDisk(); // zapis na dysk

		std::string filename;
		std::fstream file;
		Buffer buffer;
		int recordSize = 5;
		tapeMode::Mode bufferMode = tapeMode::read;

		int readCounter = 0;
		int writeCounter = 0;
		int emptyRecords = 0;
};

