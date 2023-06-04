#include "Tape.h"
#include "Record.h"
#include <iostream>

Tape::Tape(int bufferSize, std::string filename)
{
	this->filename = filename;
	file.open(filename, std::ios::in | std::ios::binary);

	buffer.data = new char[bufferSize ];
	buffer.size = bufferSize;
}

Tape::~Tape()
{
	file.close();
	delete buffer.data;
	//std::cout << "Destruktor \n";
}

void Tape::clearBuffer()
{
	buffer.position = 0;
	buffer.dataSize = 0;
}

bool Tape::isBufferEmpty()
{
	if (buffer.dataSize == 0) {
		return true;
	}

	return false;
}

int Tape::getReadDiskCount()
{
	return readCounter;
}

int Tape::getWriteDiskCount()
{
	return writeCounter;
}

bool Tape::saveBuffer()
{
	if (bufferMode == tapeMode::write) {
		saveBufferOnDisk();
		file.close();
		return true;
	}
	
	return false;
}

void Tape::changeMode(tapeMode::Mode mode) {
	if (bufferMode == mode) {
		return;
	}

	// change to write
	if (mode == tapeMode::write) {
		file.close();
		file.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	}
	// change to read
	else {
		saveBufferOnDisk();
		file.close();
		file.open(filename, std::ios::in | std::ios::binary);
	}

	buffer.dataSize = 0;
	buffer.position = 0;
	bufferMode = mode;
}

bool Tape::fillBuffer() {
	int readBlockSize = buffer.size - buffer.dataSize;

	if (file.read((buffer.data + buffer.dataSize), readBlockSize)) {
		buffer.position = 0;
		buffer.dataSize = buffer.size;
		readCounter++;
		return true;
	}
	else if (file.gcount() > 0) {
		buffer.position = 0;
		buffer.dataSize += file.gcount();
		readCounter++;
		return true;
	}

	return false;
}

void Tape::displayTape()
{
	int i;
	for (i = 0; i < buffer.dataSize; i++) {
		std::cout << unsigned((uint8_t)buffer.data[i]) << " ";	
	}
	std::cout << "elements: " << i << '\n';
	std::cout << std::endl;
}

void Tape::displayTapeRecords()
{
	Record record;

	for (int i = 0; i < buffer.dataSize; i += recordSize) {
		memcpy(record.data, buffer.data + i, recordSize);
		std::cout << record.getValue() << ' ';
	}

	std::cout << std::endl;
}

Record Tape::read()
{
	changeMode(tapeMode::read);

	//std::cout << "Read." << std::endl;
	Record record;

	if (buffer.position == 0) {
		if (fillBuffer()) {
			memcpy(record.data, buffer.data, recordSize);
			buffer.dataSize -= recordSize;
			buffer.position += recordSize;

			//std::cout << "Rekord jest pusty, uzupelnianie i pobieranie." << std::endl;
		}
		else {
			record.valid = false;
			return record;
		}
	}
	else {
		if (buffer.dataSize < recordSize) {
			memcpy(buffer.data, buffer.data + buffer.position, buffer.dataSize);

			if (!fillBuffer()) {
				record.valid = false;
				return record;
			}

			//std::cout << "Rekord uszkodzony, dopelnianie." << std::endl;
		}

		memcpy(record.data, buffer.data + buffer.position, recordSize);

		buffer.dataSize -= recordSize;
		buffer.position += recordSize;

		//std::cout << "Jest rekord, pobieranie." << std::endl;
	}
	
	return record;
}

bool Tape::saveBufferOnDisk()
{
	if (file.write(buffer.data, buffer.dataSize)) {
		buffer.dataSize = 0;
		buffer.position = 0;
		writeCounter++;
		return true;
	}
	return false;
}

bool Tape::write(Record record)
{
	changeMode(tapeMode::write);

	if ((buffer.dataSize + recordSize) > buffer.size) {
		int freeBytesCount = buffer.size - buffer.dataSize;

		for (int i = 0; i < freeBytesCount; i++) {
			buffer.data[buffer.position++] = record.data[i];
			buffer.dataSize++;
		}
		int recordPosition = freeBytesCount;

		if (!saveBufferOnDisk()) {
			return false;
		}

		for (int i = 0; i < recordSize - recordPosition; i++) {
			buffer.data[i] = record.data[i + recordPosition];
			buffer.dataSize++;
			buffer.position++;
		}

		return true;
	}
	else {
		for (int i = 0; i < recordSize; i++) {
			buffer.data[buffer.position] = record.data[i];
			buffer.dataSize++;
			buffer.position++;
		}

		return true;
	}

	return false;
}
