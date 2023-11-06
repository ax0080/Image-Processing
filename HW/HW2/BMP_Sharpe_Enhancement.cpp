#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iomanip>

using namespace std;

// BMP Head
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



void sharpenImage(unsigned char* imgBuf, int width, int height, int biBitCount, int filter[3][3]) {
    int lineByte = (width * biBitCount / 8 + 3) / 4 * 4;
    unsigned char* newImgBuf = new unsigned char[lineByte * height];

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumR = 0, sumG = 0, sumB = 0;

            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    int pixelOffset = (y + j) * lineByte + (x + i) * (biBitCount / 8);
                    sumB += imgBuf[pixelOffset] * filter[j + 1][i + 1];
                    sumG += imgBuf[pixelOffset + 1] * filter[j + 1][i + 1];
                    sumR += imgBuf[pixelOffset + 2] * filter[j + 1][i + 1];
                }
            }

            int pixelOffset = y * lineByte + x * (biBitCount / 8);
            newImgBuf[pixelOffset] = max(0, min(255, sumB));
            newImgBuf[pixelOffset + 1] = max(0, min(255, sumG));
            newImgBuf[pixelOffset + 2] = max(0, min(255, sumR));
        }
    }


    memcpy(imgBuf, newImgBuf, lineByte * height);
    delete[] newImgBuf;
}


int main() {
    char readPath[] = "input2.bmp";
    if (!readBmp(readPath)) {
        cout << "Error reading BMP file." << endl;
        return 1;
    }

    //First Filter Coeff
    int sharpenFilter1[3][3] = {{-1, -1, -1},
                                {-1,  9, -1},
                                {-1, -1, -1}};
 
    sharpenImage(pBmpBuf, bmpWidth, bmpHeight, biBitCount, sharpenFilter1);

    char writePath1[] = "output2_1.bmp";
    if (!saveBmp(writePath1, pBmpBuf, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }

    //Second Filter Coeff
    int sharpenFilter2[3][3] = {{0, -1, 0},
                                {-1,  5, -1},
                                {0, -1, 0}};
   
    sharpenImage(pBmpBuf, bmpWidth, bmpHeight, biBitCount, sharpenFilter2);

    char writePath2[] = "output2_2.bmp";
    if (!saveBmp(writePath2, pBmpBuf, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }

    delete[] pBmpBuf;
    if (biBitCount == 8)
        delete[] pColorTable;

    return 0;
}