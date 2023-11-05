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

// Log Transformation function for Y channel in YCbCr color space
void logTransformYCbCr(unsigned char* inputBuf, unsigned char* outputBuf, int width, int height, int c) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            double R = inputBuf[index * 3];
            double G = inputBuf[index * 3 + 1];
            double B = inputBuf[index * 3 + 2];
            double Y = 0.299 * R + 0.587 * G + 0.114 * B;
            double Cb = 0.564 * (B - Y);
            double Cr = 0.713 * (R - Y);

            // Apply log transformation to Y channel
            double transformedY = c * log(1 + Y);

            // Reconstruct the color with transformed Y and unchanged Cb and Cr
            double R_out = transformedY + 1.13983 * Cr;
            double G_out = transformedY - 0.39465 * Cb - 0.58060 * Cr;
            double B_out = transformedY + 2.03211 * Cb;

            // Clip the result values to [0, 255] range
            R_out = max(0.0, min(255.0, R_out));
            G_out = max(0.0, min(255.0, G_out));
            B_out = max(0.0, min(255.0, B_out));

            // Store the result in the output image
            outputBuf[index * 3] = static_cast<unsigned char>(R_out);
            outputBuf[index * 3 + 1] = static_cast<unsigned char>(G_out);
            outputBuf[index * 3 + 2] = static_cast<unsigned char>(B_out);
        }
    }
}

int main() {
    char readPath[] = "input1.bmp";
    if (!readBmp(readPath)) {
        cout << "Error reading BMP file." << endl;
        return 1;
    }

    // Apply log transformation with c = 30
    int c1 = 30;
    // Apply log transformation with c = 50
    int c2 = 50;

    // Create two new output image buffers
    unsigned char* outputBuf1 = new unsigned char[bmpWidth * bmpHeight * 3];
    unsigned char* outputBuf2 = new unsigned char[bmpWidth * bmpHeight * 3];

    // Apply the log transformation in YCbCr color space with c = 10
    logTransformYCbCr(pBmpBuf, outputBuf1, bmpWidth, bmpHeight, c1);

    // Apply the log transformation in YCbCr color space with c = 30
    logTransformYCbCr(pBmpBuf, outputBuf2, bmpWidth, bmpHeight, c2);

    char writePath1[] = "output1_1.bmp";
    char writePath2[] = "output1_2.bmp";

    if (!saveBmp(writePath1, outputBuf1, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file (output1_1.bmp)." << endl;
        return 1;
    }

    if (!saveBmp(writePath2, outputBuf2, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file (output1_2.bmp)." << endl;
        return 1;
    }

    delete[] pBmpBuf;
    delete[] outputBuf1;
    delete[] outputBuf2;
    if (biBitCount == 8)
        delete[] pColorTable;

    return 0;
}
