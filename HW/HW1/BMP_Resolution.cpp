#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <iomanip>

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
    BYTE rgbRed;
    BYTE rgbGreen;
    BYTE rgbBlue;
    BYTE rgbReserved;
};

unsigned char* pBmpBuf;
int bmpWidth;
int bmpHeight;
int biBitCount;

RGBQUAD* pColorTable;

//Read BMP
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
    //cout << "Bit Count: " << biBitCount << endl;
    fclose(fp);
    return true;
}

//Output BMP
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

void quantizeChannel(BYTE& channel, int bits) {
    BYTE mask = 0xFF << (8 - bits);
    channel = channel & mask;
}

void quantizeImage(unsigned char* imgBuf, int width, int height, int biBitCount, int bits) {
    int bytesPerPixel = biBitCount / 8;
    int lineByte = (width * bytesPerPixel + 3) / 4 * 4;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            for (int k = 0; k < 3; ++k) {  // R, G, and B channels
                quantizeChannel(imgBuf[i * lineByte + j * bytesPerPixel + k], bits);
            }
        }
    }
}

int main() {
    char readPath[] = "input2.bmp";
    if (!readBmp(readPath)) {
        cout << "Error reading BMP file." << endl;
        return 1;
    }

    if (biBitCount != 24 && biBitCount != 32) {
        cout << "Only 24-bit or 32-bit BMP files are supported for this operation." << endl;
        delete[] pBmpBuf;
        return 1;
    }

    unsigned char* copyBuffer1 = new unsigned char[bmpWidth * bmpHeight * (biBitCount / 8)];
    memcpy(copyBuffer1, pBmpBuf, bmpWidth * bmpHeight * (biBitCount / 8));
    quantizeImage(copyBuffer1, bmpWidth, bmpHeight, biBitCount, 6);
    saveBmp("output2_1.bmp", copyBuffer1, bmpWidth, bmpHeight, biBitCount, nullptr);
    delete[] copyBuffer1;

    unsigned char* copyBuffer2 = new unsigned char[bmpWidth * bmpHeight * (biBitCount / 8)];
    memcpy(copyBuffer2, pBmpBuf, bmpWidth * bmpHeight * (biBitCount / 8));
    quantizeImage(copyBuffer2, bmpWidth, bmpHeight, biBitCount, 4);
    saveBmp("output2_2.bmp", copyBuffer2, bmpWidth, bmpHeight, biBitCount, nullptr);
    delete[] copyBuffer2;

    unsigned char* copyBuffer3 = new unsigned char[bmpWidth * bmpHeight * (biBitCount / 8)];
    memcpy(copyBuffer3, pBmpBuf, bmpWidth * bmpHeight * (biBitCount / 8));
    quantizeImage(copyBuffer3, bmpWidth, bmpHeight, biBitCount, 2);
    saveBmp("output2_3.bmp", copyBuffer3, bmpWidth, bmpHeight, biBitCount, nullptr);
    delete[] copyBuffer3;

    delete[] pBmpBuf;

    return 0;
}