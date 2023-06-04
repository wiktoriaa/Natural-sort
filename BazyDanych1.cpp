// BazyDanych1.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include "Record.h"
#include "Tape.h"
#include <random>

using namespace std;

void generateFile(string filename, int bytes) {
    fstream file(filename, ios::out | ios::trunc | ios::binary);
    srand(time(NULL));
    char byte[1];
    uint8_t num;

    for (int i = 0; i < bytes; i++) {
        num = rand() % 255;
        byte[0] = (char) num;
        file.write(byte, 1);
    }

    file.close();
}

void createFile(string filename) {
    fstream file(filename, ios::out | ios::trunc | ios::binary);

    int recordSize = 5;
    int recordsCount;
    uint8_t number;
    string byte;
    Record record;

    string inputRecord;

    cout << "Wpisz liczbe rekordow: ";
    cin >> recordsCount;
    cin.ignore();

    for (int i = 0; i < recordsCount; i++) {
        record.valid = true;

        cout << "RECORD " << i << ":\n";
        cout << "Wpisz " << recordSize <<  " liczb z zakresu 0-255 oddzielonych spacjami lub enterami: ";

        for (int i = 0; i < 5; i++)
        {
            cin >> byte;
            number = (uint8_t)atoi(byte.c_str());

            if (number > 255 || number < 0) {
                cout << "Zly zakres, nie udalo sie wczytac rekordu\n";
                record.valid = false;
                break;
            }

            record.data[i] = number;
        }

        
        if (record.valid) {
            file.write((const char*) record.data, 5);
        }
    }

    file.close();
}

void displayFile(string filename) {
    fstream file(filename, ios::in | ios::binary);
    char byte[1];

    while (file.read(byte, 1)) {
        cout << unsigned((uint8_t)byte[0]) << " ";
    }
}

void displayFileRecords(string filename) {
    fstream file(filename, ios::in | ios::binary);
    Record record;
    char bytes[5];

    cout << "\n\n*** REKORDY W PLIKU: ***\n";
    int counter = 0;

    while (file.read(bytes, 5)) {
        memcpy(record.data, bytes, 5);
        cout << record.getValue() << " ";
        counter++;
    }

    cout << "\nLICZBA REKORDOW: " << counter << "\n";
}

void displayFileRecords(string filename, int recordsCount) {
    fstream file(filename, ios::in | ios::binary);
    Record record;
    char bytes[5];

    if (file.peek() == std::ifstream::traits_type::eof()) {
        cout << "\n\n*** PLIK JEST PUSTY ***\n";
        return;
    }

    cout << "\n\n*** REKORDY W PLIKU: ***\n";

    int i = 0;

    while (file.read(bytes, 5)) {
        if (i >= recordsCount) {
            break;
        }
        memcpy(record.data, bytes, 5);
        cout << record.getValue() << " ";
        i++;
    }

    cout << "LICZBA REKORDÓW: " << i << "\n";
}

struct SortResult {
    int readsCount = 0;
    int writeCount = 0;
    int seriesCount = 0;
    string outputFilename;
};

SortResult sortFile(string inputFile, string outputFile, bool debug = false) {
    string filename = inputFile;
    Record record;
    int recordSize = 5;
    int blockSize = 53;

    SortResult result;

    Tape tapeRead(blockSize, filename);
    //tapeRead.fillBuffer();

    Tape tape1(blockSize, "tape1.txt");
    Tape tape2(blockSize, "tape2.txt");

    tape1.changeMode(tapeMode::write);
    tape2.changeMode(tapeMode::write);

    Tape tapeWrite(blockSize, outputFile);

    int prevRecordValue;
    int currentTape = 1;

    bool readFileEnd = false;

    // distribution

    record = tapeRead.read();
    int i = 0;

    while (record.valid) {
        
        if (i > 0 && prevRecordValue > record.getValue()) {
            result.seriesCount++;

            if (currentTape == 1) {
                currentTape = 2;
            }
            else {
                currentTape = 1;
            }
        }

        prevRecordValue = record.getValue();

        switch (currentTape) {
        case 1:
            tape1.write(record);
            break;
        case 2:
            tape2.write(record);
            break;
        }

        record = tapeRead.read();
        ++i;
    }

    if (tape1.isBufferEmpty()) {
        //cout << "\n****** PLIK POSORTOWANY *******\n";

        tape2.saveBuffer();

        result.readsCount = tapeRead.getReadDiskCount();
        result.writeCount = tapeWrite.getWriteDiskCount();
        result.outputFilename = "tape2.txt";
        return result;
    }
    else if (tape2.isBufferEmpty()) {
        //cout << "\n****** PLIK POSORTOWANY *******\n";

        tape1.saveBuffer();

        result.readsCount = tapeRead.getReadDiskCount();
        result.writeCount = tapeWrite.getWriteDiskCount();
        result.outputFilename = "tape1.txt";
        return result;
    }

    tape1.saveBuffer();
    tape2.saveBuffer();

    // natural merge

    Record recordT1;
    Record recordT2;

    uint8_t prevRecordT1 = 0;
    uint8_t prevRecordT2 = 0;

    recordT1 = tape1.read();
    recordT2 = tape2.read();

    while (recordT1.valid || recordT2.valid) {
        if (!recordT1.valid) {
            while (recordT2.valid) {
                tapeWrite.write(recordT2);
                recordT2 = tape2.read();
            }
            break;
        }
        else if (!recordT2.valid) {
            while (recordT1.valid) {
                tapeWrite.write(recordT1);
                recordT1 = tape1.read();
            }
            break;
        }

        if (recordT1.getValue() < recordT2.getValue()) {
            tapeWrite.write(recordT1);
            prevRecordT1 = recordT1.getValue();
            recordT1 = tape1.read();

            if (recordT1.getValue() < prevRecordT1) {
                // dopóki w buforze są większe rekordy 
                while (recordT2.getValue() >= prevRecordT2 && recordT2.valid) {
                    tapeWrite.write(recordT2);
                    prevRecordT2 = recordT2.getValue();
                    recordT2 = tape2.read();
                }
            }
        }
        else {
            tapeWrite.write(recordT2);
            prevRecordT2 = recordT2.getValue();
            recordT2 = tape2.read();

            if (recordT2.getValue() < prevRecordT2) {
                // dopóki w buforze są większe rekordy 
                while (recordT1.getValue() >= prevRecordT1 && recordT1.valid) {
                    tapeWrite.write(recordT1);
                    prevRecordT1 = recordT1.getValue();
                    recordT1 = tape1.read();
                }
            }
        }
    }

    // zapis ostatniego (być może niepełnego) bloku na dysk
    tapeWrite.saveBuffer();

    result.readsCount = tapeRead.getReadDiskCount();
    result.writeCount = tapeWrite.getWriteDiskCount();
    result.outputFilename = outputFile;

    if (debug) {
        cout << "\n\nZAKONCZONO FAZE SORTOWANIA: \n";
        displayFileRecords(outputFile);
    }

    return result;
}

int main()
{
    int recordSize = 5;
    string input = "input.txt";
    string output = "output.txt";

    int option;

    cout << "****************************************************************\n";
    cout << "***                  WYBIERZ OPCJE:                          ***\n";
    cout << "****************************************************************\n";
    cout << "***                                                          ***\n";
    cout << "***            1. WYGENERUJ PLIK                             ***\n";
    cout << "***            2. WCZYTAJ DANE Z KLAWIATURY                  ***\n";
    cout << "***            3. WCZYTAJ DANE Z PLIKU                       ***\n";
    cout << "***                                                          ***\n";
    cout << "****************************************************************\n";

    cout << "Twoj wybor: >> ";
    cin >> option;

    int recordsNumber;

    switch (option) {
        case 1: 
            cout << "Podaj liczbe rekordow: >> ";
            cin >> recordsNumber;
            generateFile(input, recordsNumber * recordSize);
            break;
        case 2:
            createFile(input);
            break;
        case 3:
            cout << "Podaj nazwe pliku: >> ";
            cin >> input;
            break;
        default:
            cout << "\n\n*** ERROR: nie ma takiej opcji ***\n\n";
            return -1;
    }

    char debugMode;
    cout << "Czy wyswietlic zawartosc pliku po kazdej fazie sortowania? t/N: >> ";
    cin >> debugMode;
    
    cout << "*** PLIK WEJSCIOWY ***\n\n";
    displayFile(input);
    displayFileRecords(input);

    SortResult result;
    string tmp;

    int phaseCounter = 0;
    int reads = 0;
    int writes = 0;
    int series = 0;

    while (true) {

        if (debugMode == 't' || debugMode == 'T') {
            result = sortFile(input, output, true);
        }
        else {
            result = sortFile(input, output);
        }

        if (phaseCounter == 0) {
            series = result.seriesCount;
        }

        if (result.writeCount == 0) {
            break;
        }
        else {
            phaseCounter++;
        }

        tmp = output;
        output = input;
        input = tmp;

        reads += result.readsCount;
        writes += result.writeCount;
    }

    cout << "\n****** PLIK POSORTOWANY *******\n";
    displayFileRecords(result.outputFilename);
    cout << "\n\nLiczba serii: " << series  << "\n";
    cout << "Liczba faz: " << phaseCounter << "\n";
    cout << "Liczba odczytow z dysku: " << reads << "\n";
    cout << "Liczba zapisow na dysk: " << writes << "\n";
   

    return 0;
}


