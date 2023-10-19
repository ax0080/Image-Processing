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

bool flipHorizontal(unsigned char* imgBuf, int width, int height, int biBitCount) {
    int lineByte = (width * biBitCount / 8 + 3) / 4 * 4;
    unsigned char* tempRow = new unsigned char[lineByte];

    for (int i = 0; i < height; i++) {
        int offset1 = i * lineByte;
        int offset2 = (i + 1) * lineByte - biBitCount / 8;

        while (offset1 < offset2) {
            for (int j = 0; j < biBitCount / 8; j++) {
                tempRow[j] = imgBuf[offset1 + j];
                imgBuf[offset1 + j] = imgBuf[offset2 + j];
                imgBuf[offset2 + j] = tempRow[j];
            }
            offset1 += biBitCount / 8;
            offset2 -= biBitCount / 8;
        }
    }

    delete[] tempRow;
    return true;
}

// Bilinear interpolation function
void bilinearInterpolation(const unsigned char* srcImg, int srcWidth, int srcHeight, unsigned char* dstImg, int dstWidth, int dstHeight, int biBitCount) {
    for (int y = 0; y < dstHeight; y++) {
        for (int x = 0; x < dstWidth; x++) {
            float srcX = x * (srcWidth - 1) / (float)(dstWidth - 1);
            float srcY = y * (srcHeight - 1) / (float)(dstHeight - 1);

            int x0 = (int)srcX;
            int y0 = (int)srcY;
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            float alpha = srcX - x0;
            float beta = srcY - y0;

            for (int c = 0; c < biBitCount / 8; c++) {
                int srcIndex00 = (y0 * srcWidth + x0) * (biBitCount / 8) + c;
                int srcIndex01 = (y0 * srcWidth + x1) * (biBitCount / 8) + c;
                int srcIndex10 = (y1 * srcWidth + x0) * (biBitCount / 8) + c;
                int srcIndex11 = (y1 * srcWidth + x1) * (biBitCount / 8) + c;
                int dstIndex = (y * dstWidth + x) * (biBitCount / 8) + c;

                dstImg[dstIndex] = (1 - alpha) * (1 - beta) * srcImg[srcIndex00] +
                                  alpha * (1 - beta) * srcImg[srcIndex01] +
                                  (1 - alpha) * beta * srcImg[srcIndex10] +
                                  alpha * beta * srcImg[srcIndex11];
            }
        }
    }
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
    char readPath[] = "input1.bmp";
    if (!readBmp(readPath)) {
        cout << "Error reading BMP file." << endl;
        return 1;
    }
/*
    // Flip the image horizontally
    flipHorizontal(pBmpBuf, bmpWidth, bmpHeight, biBitCount);

    char writePath[] = "output1_flip.bmp";
    if (!saveBmp(writePath, pBmpBuf, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }
*/

    //UP scaling
    int newWidth = bmpWidth * 1.5; // Adjust the new width as needed
    int newHeight = bmpHeight * 1.5; // Adjust the new height as needed
    unsigned char* resizedBmpBuf = new unsigned char[newWidth * newHeight * (biBitCount / 8)];

    // Perform bilinear interpolation to resize the image
    bilinearInterpolation(pBmpBuf, bmpWidth, bmpHeight, resizedBmpBuf, newWidth, newHeight, biBitCount);

    char writePath_resize_up[] = "output1_up.bmp";
    if (!saveBmp(writePath_resize_up, resizedBmpBuf, newWidth, newHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }
    //down scaling
    newWidth = bmpWidth * (1/1.5); // Adjust the new width as needed
    newHeight = bmpHeight * (1/1.5); // Adjust the new height as needed
    resizedBmpBuf = new unsigned char[newWidth * newHeight * (biBitCount / 8)];

    // Perform bilinear interpolation to resize the image
    bilinearInterpolation(pBmpBuf, bmpWidth, bmpHeight, resizedBmpBuf, newWidth, newHeight, biBitCount);

    char writePath_resize_down[] = "output1_down.bmp";
    if (!saveBmp(writePath_resize_down, resizedBmpBuf, newWidth, newHeight, biBitCount, pColorTable)) {
        cout << "Error saving BMP file." << endl;
        return 1;
    }    

    delete[] pBmpBuf;
    delete[] resizedBmpBuf; // Don't forget to free the memory for the resized image

    if (biBitCount == 8)
        delete[] pColorTable;

    return 0;
}
