#pragma once

struct Buffer {
	char *data;
	int size;
	int dataSize = 0;
	int position = 0;
};