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

bool quantizeImage(unsigned char* imgBuf, int width, int height, int biBitCount, int newBitCount) {
    if (biBitCount == newBitCount) {
        // No need to quantize if the bit counts are the same
        return true;
    }

    int lineByte = (width * biBitCount / 8 + 3) / 4 * 4;
    int newLineByte = (width * newBitCount / 8 + 3) / 4 * 4;
    int shiftBits = biBitCount - newBitCount;
    int maxValue = (1 << newBitCount) - 1;

    for (int i = 0; i < height; i++) {
        int offset = i * lineByte;

        for (int j = 0; j < width; j++) {
            int pixelOffset = offset + j * (biBitCount / 8);
            unsigned int pixelValue = 0;

            // Extract the pixel value
            for (int k = 0; k < biBitCount / 8; k++) {
                pixelValue |= (imgBuf[pixelOffset + k] << (k * 8));
            }

            // Quantize the pixel value
            pixelValue = (pixelValue >> shiftBits) & maxValue;

            // Update the pixel value
            for (int k = 0; k < newBitCount; k++) {
                imgBuf[pixelOffset + k] = static_cast<unsigned char>((pixelValue >> (k * 8)) & 0xFF);
            }
        }
    }

    return true;
}

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

int main() {
    char readPath[] = "input2.bmp";
    if (!readBmp(readPath)) {
        cout << "Error reading BMP file." << endl;
        return 1;
    }

    // Quantize the image to 6 bits
    quantizeImage(pBmpBuf, bmpWidth, bmpHeight, biBitCount, 6);

    char writePath[] = "output2_quantized_6bit.bmp";
    if (!saveBmp(writePath, pBmpBuf, bmpWidth, bmpHeight, 6, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }

    // Quantize the image to 4 bits
    quantizeImage(pBmpBuf, bmpWidth, bmpHeight, 6, 4);

    char writePath2[] = "output2_quantized_4bit.bmp";
    if (!saveBmp(writePath2, pBmpBuf, bmpWidth, bmpHeight, 4, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }

    // Quantize the image to 2 bits
    quantizeImage(pBmpBuf, bmpWidth, bmpHeight, 4, 2);

    char writePath3[] = "output2_quantized_2bit.bmp";
    if (!saveBmp(writePath3, pBmpBuf, bmpWidth, bmpHeight, 2, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }

    delete[] pBmpBuf;
    if (biBitCount == 8)
        delete[] pColorTable;

    return 0;
}
