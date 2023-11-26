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

// 讀取BMP
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

// 儲存BMP
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

// 白平衡校正 (灰度世界法)
void performWhiteBalance(unsigned char* inputBuf, unsigned char* outputBuf, int width, int height) {
    double sumR = 0, sumG = 0, sumB = 0;

    // 計算每個通道的平均值
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            sumR += inputBuf[index * 3];
            sumG += inputBuf[index * 3 + 1];
            sumB += inputBuf[index * 3 + 2];
        }
    }

    double avgR = sumR / (width * height);
    double avgG = sumG / (width * height);
    double avgB = sumB / (width * height);

    // 計算色溫調整係數
    double scaleR = avgG / avgR;
    double scaleB = avgG / avgB;

    // 執行白平衡校正
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;
            outputBuf[index * 3] = static_cast<unsigned char>(inputBuf[index * 3] * scaleR);
            outputBuf[index * 3 + 1] = static_cast<unsigned char>(inputBuf[index * 3 + 1]);
            outputBuf[index * 3 + 2] = static_cast<unsigned char>(inputBuf[index * 3 + 2] * scaleB);
        }
    }
}

int main() {
    // 修改輸入和輸出檔案名稱
    char readPath[] = "input1.bmp";
    if (!readBmp(readPath)) {
        cout << "讀取BMP文件時發生錯誤。" << endl;
        return 1;
    }

    // 創建一個新的輸出影像緩衝區
    unsigned char* outputBuf = new unsigned char[bmpWidth * bmpHeight * 3];

    // 白平衡校正
    performWhiteBalance(pBmpBuf, outputBuf, bmpWidth, bmpHeight);

    // 修改輸出檔案名稱
    char writePath[] = "output1_1.bmp";

    if (!saveBmp(writePath, outputBuf, bmpWidth, bmpHeight, biBitCount, pColorTable)) {
        cout << "保存BMP文件時發生錯誤 (output4_1.bmp)。" << endl;
        return 1;
    }

    // 釋放資源
    delete[] pBmpBuf;
    delete[] outputBuf;
    if (biBitCount == 8)
        delete[] pColorTable;

    return 0;
}