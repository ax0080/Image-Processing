#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
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

//Resize
unsigned char* resize(unsigned char* imgBuf, int width, int height, int biBitCount, float factor, int &newWidth, int &newHeight) {
    newWidth = (int)(width * factor);
    newHeight = (int)(height * factor);
    int bytesPerPixel = biBitCount / 8;
    int lineByte = (newWidth * biBitCount / 8 + 3) / 4 * 4;
    unsigned char* newImgBuf = new unsigned char[lineByte * newHeight];

    for(int i = 0; i < newHeight; i++) {
        for(int j = 0; j < newWidth; j++) {
            int oldX = (int)(j / factor);
            int oldY = (int)(i / factor);
            for(int b = 0; b < bytesPerPixel; b++) {
                newImgBuf[(i * lineByte) + (j * bytesPerPixel) + b] = imgBuf[(oldY * (width * bytesPerPixel)) + (oldX * bytesPerPixel) + b];
            }
        }
    }

    return newImgBuf;
}

int main() {
    char readPath[] = "input2.bmp";
    if (!readBmp(readPath)) {
        cout << "Error reading BMP file." << endl;
        return 1;
    }

    // Rescale up by 1.5
    int newWidth, newHeight;
    unsigned char* upScaledImg = resize(pBmpBuf, bmpWidth, bmpHeight, biBitCount, 1.5, newWidth, newHeight);
    saveBmp("output2_up.bmp", upScaledImg, newWidth, newHeight, biBitCount, pColorTable);
    delete[] upScaledImg;

    // Rescale down by 1.5
    unsigned char* downScaledImg = resize(pBmpBuf, bmpWidth, bmpHeight, biBitCount, 1/1.5, newWidth, newHeight);
    saveBmp("output2_down.bmp", downScaledImg, newWidth, newHeight, biBitCount, pColorTable);
    delete[] downScaledImg;

    delete[] pBmpBuf;
    if (biBitCount == 8) {
        delete[] pColorTable;
    }

    return 0;
}
