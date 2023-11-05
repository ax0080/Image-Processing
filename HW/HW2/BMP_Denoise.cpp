#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

typedef unsigned short WORD;
typedef unsigned char BYTE;

#pragma pack(1)
struct BITMAPFILEHEADER {
    WORD bfType;
    int bfSize;
    int bfReserved;
    int bfOffbits;
};

struct BITMAPINFOHEADER {
    int biSize;
    int biWidth;
    int biHeight;
    WORD biPlanes;
    WORD biBitCount;
    int biCompression;
    int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    int biClrUsed;
    int biClrImportant;
};

struct RGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
};

unsigned char* pBmpBuf;
int bmpWidth;
int bmpHeight;
int biBitCount;

RGBQUAD* pColorTable;

// Read BMP
bool readBmp(char* bmpName) {
    FILE* fp = fopen(bmpName, "rb");
    if (fp == 0)
        return false;

    fseek(fp, sizeof(BITMAPFILEHEADER), 0);
    BITMAPINFOHEADER head;
    fread(&head, sizeof(BITMAPINFOHEADER), 1, fp);

    bmpWidth = head.biWidth;
    bmpHeight = head.biHeight;
    biBitCount = head.biBitCount;

    int lineByte = (bmpWidth * biBitCount / 8 + 3) / 4 * 4;

    if (biBitCount == 8) {
        pColorTable = new RGBQUAD[256];
        fread(pColorTable, sizeof(RGBQUAD), 256, fp);
    }

    pBmpBuf = new unsigned char[lineByte * bmpHeight];
    fread(pBmpBuf, 1, lineByte * bmpHeight, fp);
    fclose(fp);
    return true;
}

// Save BMP
bool saveBmp(char* bmpName, unsigned char* imgBuf, int width, int height, int biBitCount, RGBQUAD* pColorTable) {
    if (!imgBuf)
        return false;

    int colorTablesize = 0;

    if (biBitCount == 8)
        colorTablesize = 1024;

    int lineByte = (width * biBitCount / 8 + 3) / 4 * 4;
    FILE* fp = fopen(bmpName, "wb");

    if (fp == 0)
        return false;

    BITMAPFILEHEADER fileHead;
    fileHead.bfType = 0x4D42;
    fileHead.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte * height;
    fileHead.bfReserved = 0;
    fileHead.bfOffbits = 54 + colorTablesize;

    fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);

    BITMAPINFOHEADER head;
    head.biBitCount = biBitCount;
    head.biClrImportant = 0;
    head.biClrUsed = 0;
    head.biCompression = 0;
    head.biHeight = height;
    head.biPlanes = 1;
    head.biSize = 40;
    head.biSizeImage = lineByte * height;
    head.biWidth = width;
    head.biXPelsPerMeter = 0;
    head.biYPelsPerMeter = 0;

    fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp);

    if (biBitCount == 8)
        fwrite(pColorTable, sizeof(RGBQUAD), 256, fp);

    fwrite(imgBuf, height * lineByte, 1, fp);
    fclose(fp);
    return true;
}

// Median Filter
void medianFilter(unsigned char* srcBuf, unsigned char* destBuf, int width, int height, int windowSize) {
    int halfSize = windowSize / 2;
    int lineByte = (width * biBitCount / 8 + 3) / 4 * 4;

    for (int y = halfSize; y < height - halfSize; y++) {
        for (int x = halfSize; x < width - halfSize; x++) {
            int index = y * lineByte + x * (biBitCount / 8);

            unsigned char redValues[25];   // Red channel values
            unsigned char greenValues[25]; // Green channel values
            unsigned char blueValues[25];  // Blue channel values
            int windowIndex = 0;

            for (int j = -halfSize; j <= halfSize; j++) {
                for (int i = -halfSize; i <= halfSize; i++) {
                    int pixelIndex = index + (j * lineByte) + (i * (biBitCount / 8));
                    redValues[windowIndex] = srcBuf[pixelIndex + 2];     // Red channel
                    greenValues[windowIndex] = srcBuf[pixelIndex + 1];   // Green channel
                    blueValues[windowIndex] = srcBuf[pixelIndex];        // Blue channel
                    windowIndex++;
                }
            }

            // Sort the channel values
            sort(redValues, redValues + windowSize);
            sort(greenValues, greenValues + windowSize);
            sort(blueValues, blueValues + windowSize);

            // Set the median values for each channel
            destBuf[index + 2] = redValues[windowSize / 2];      // Red channel
            destBuf[index + 1] = greenValues[windowSize / 2];    // Green channel
            destBuf[index] = blueValues[windowSize / 2];         // Blue channel

            // Handle the alpha channel for 32-bit BMP
            if (biBitCount == 32) {
                destBuf[index + 3] = srcBuf[index + 3];
            }
        }
    }
}


int main() {
    char readPath[] = "input3.bmp";
    if (!readBmp(readPath)) {
        cout << "Error reading BMP file." << endl;
        return 1;
    }

    char writePath1[] = "output3_1.bmp";
    unsigned char* pFilteredBmpBuf1 = new unsigned char[bmpWidth * bmpHeight * (biBitCount / 8)];
    memcpy(pFilteredBmpBuf1, pBmpBuf, bmpWidth * bmpHeight * (biBitCount / 8));
    medianFilter(pBmpBuf, pFilteredBmpBuf1, bmpWidth, bmpHeight, 20); // Adjust window size as needed
    if (!saveBmp(writePath1, pFilteredBmpBuf1, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }
    delete[] pFilteredBmpBuf1;

    char writePath2[] = "output3_2.bmp";
    unsigned char* pFilteredBmpBuf2 = new unsigned char[bmpWidth * bmpHeight * (biBitCount / 8)];
    memcpy(pFilteredBmpBuf2, pBmpBuf, bmpWidth * bmpHeight * (biBitCount / 8));
    medianFilter(pBmpBuf, pFilteredBmpBuf2, bmpWidth, bmpHeight, 5); // Another set of filter parameters
    if (!saveBmp(writePath2, pFilteredBmpBuf2, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }
    delete[] pFilteredBmpBuf2;

    delete[] pBmpBuf;
    if (biBitCount == 8)
        delete[] pColorTable;

    return 0;
}
