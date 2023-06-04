#pragma once
#include <cstdint>

class Record
{
public:
	uint8_t data[5];
	bool valid = true;
	float getValue();
};

