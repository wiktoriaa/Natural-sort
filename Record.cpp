#include "Record.h"

float Record::getValue()
{
    uint8_t sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += data[i];
    }

    float avg = sum / 5.0;
    return avg;
}
