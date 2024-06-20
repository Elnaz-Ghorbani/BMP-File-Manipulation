#include <algorithm>
#include <fstream>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

// declaring BMP file
struct BMP
{
    int width;
    int height;
    unsigned char header[54];
    unsigned char *pixels;
    int size;
    int row_padded;
    long long int size_padded;
};

// writing
void writeBMP(string filename, BMP image)
{
    string fileName = filename;
    FILE *out = fopen(fileName.c_str(), "wb");
    fwrite(image.header, sizeof(unsigned char), 54, out);

    unsigned char tmp;
    for (int i = 0; i < image.height; i++)
    {
        for (int j = 0; j < image.width * 3; j += 3)
        {
            // Convert BGR to RGB (the order of colors is BGR we need RGB)
            tmp = image.pixels[j];
            image.pixels[j] = image.pixels[j + 2];
            image.pixels[j + 2] = tmp;
        }
    }
    fwrite(image.pixels, sizeof(unsigned char), image.size_padded, out);
    fclose(out);
}

// read BMP file
BMP readBMP(string filename)
{
    BMP image;
    string fileName = filename;
    FILE *in = fopen(fileName.c_str(), "rb");

    if (in == NULL)
        throw "Argument Exception";

    fread(image.header, sizeof(unsigned char), 54, in); // read the 54-byte header

    // extract image height and width from header
    image.width = *(int *)&image.header[18];
    image.height = *(int *)&image.header[22];

    image.row_padded = (image.width * 3 + 3) & (~3);     // size of a single row must be rounded up to multiple of 4
    image.size_padded = image.row_padded * image.height; // padded full size
    image.pixels = new unsigned char[image.size_padded];

    if (fread(image.pixels, sizeof(unsigned char), image.size_padded, in) == image.size_padded)
    {
        unsigned char tmp;
        for (int i = 0; i < image.height; i++)
        {
            for (int j = 0; j < image.width * 3; j += 3)
            {
                // Convert BGR to RGB
                tmp = image.pixels[j];
                image.pixels[j] = image.pixels[j + 2];
                image.pixels[j + 2] = tmp;
            }
        }
    }
    else
    {
        cout << "Error: all bytes couldn't be read" << endl;
    }

    fclose(in);
    return image;
}

// 180 degrees rotation
BMP rotate180(BMP image)
{
    BMP newImage = image;
    unsigned char *pixels = new unsigned char[image.size_padded];

    int H = image.height, W = image.width;
    for (int x = 0; x < H; x++)
    {
        for (int y = 0; y < W; y++)
        {
            pixels[(x * W + y) * 3 + 0] = image.pixels[(H * W - x * W - y) * 3 + 0];
            pixels[(x * W + y) * 3 + 1] = image.pixels[(H * W - x * W - y) * 3 + 1];
            pixels[(x * W + y) * 3 + 2] = image.pixels[(H * W - x * W - y) * 3 + 2];
        }
    }

    newImage.pixels = pixels;
    return newImage;
}

// 90 degrees rotation (clockwise / left)
BMP rotate90_l(BMP image)
{
    BMP newImage = image;
    unsigned char *pixels = new unsigned char[image.size_padded];

    int H = image.height, W = image.width;

    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            pixels[(y * W + x) * 3 + 0] = image.pixels[(x * W + y) * 3 + 0];
            pixels[(y * W + x) * 3 + 1] = image.pixels[(x * W + y) * 3 + 1];
            pixels[(y * W + x) * 3 + 2] = image.pixels[(x * W + y) * 3 + 2];
        }
    }

    newImage.pixels = pixels;
    return newImage;
}

// rotate 90 degrees (counterclockwise / right)
BMP rotate90_r(BMP image)
{
    BMP newImage = image;
    unsigned char *pixels = new unsigned char[image.size_padded];

    int H = image.height, W = image.width;

    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
        {
            pixels[(y * W + x) * 3 + 0] = image.pixels[(H * W - x * H + y - W) * 3 + 0];
            pixels[(y * W + x) * 3 + 1] = image.pixels[(H * W - x * H + y - W) * 3 + 1];
            pixels[(y * W + x) * 3 + 2] = image.pixels[(H * W - x * H + y - W) * 3 + 2];
        }
    }

    newImage.pixels = pixels;
    return newImage;
}

// lighting
BMP lighting_down(BMP image)
{
    BMP newImage = image;
    unsigned char *pixels = new unsigned char[image.size_padded];

    int H = image.height, W = image.width;

    for (int x = 0; x < H; x++)
    {
        for (int y = 0; y < W; y++)
        {
            pixels[(x * W + y) * 3 + 0] = image.pixels[(x * W + y) * 3 + 0] / 3;
            pixels[(x * W + y) * 3 + 1] = image.pixels[(x * W + y) * 3 + 1] / 3;
            pixels[(x * W + y) * 3 + 2] = image.pixels[(x * W + y) * 3 + 2] / 3;
        }
    }

    newImage.pixels = pixels;
    return newImage;
}

// contrast : this code corrects the contrast by 127
BMP contrast(BMP image)
{
    BMP newImage = image;
    unsigned char *pixels = new unsigned char[image.size_padded];

    int H = image.height, W = image.width;

    int factor0 = (259 * (image.pixels[0] + 255)) / (255 * (259 - image.pixels[0]));
    int factor1 = (259 * (image.pixels[1] + 255)) / (255 * (259 - image.pixels[1]));
    int factor2 = (259 * (image.pixels[2] + 255)) / (255 * (259 - image.pixels[2]));

    for (int x = 0; x < H; x++)
    {
        for (int y = 0; y < W; y++)
        {
            pixels[(x * W + y) * 3 + 0] = factor0 * (image.pixels[(x * W + y) * 3 + 0] - 127) + 127;
            pixels[(x * W + y) * 3 + 1] = factor1 * (image.pixels[(x * W + y) * 3 + 1] - 127) + 127;
            pixels[(x * W + y) * 3 + 2] = factor2 * (image.pixels[(x * W + y) * 3 + 2] - 127) + 127;
        }
    }

    newImage.pixels = pixels;
    return newImage;
}

// Edge dettection  (does not work properly with colored pictures)
BMP hollow(BMP image)
{
    BMP newImage = image;
    unsigned char *pixels = new unsigned char[image.size_padded];

    int H = image.height, W = image.width;

    for (int x = 1; x < H - 1; x++)
    {
        for (int y = 1; y < W - 1; y++)
        {
            if ((image.pixels[(x * W + y) * 3 + 0] - image.pixels[(x * W + y) * 3 + 3]) < 5 &&
                (image.pixels[(x * W + y) * 3 + 1] - image.pixels[(x * W + y) * 3 + 6]) < 5 &&
                (image.pixels[(x * W + y) * 3 + 2] - image.pixels[(x * W + y) * 3 + 9]) < 5)
            {

                pixels[(x * W + y) * 3 + 0] = image.pixels[(x * W + y) * 3 + 0];
                pixels[(x * W + y) * 3 + 1] = image.pixels[(x * W + y) * 3 + 1];
                pixels[(x * W + y) * 3 + 2] = image.pixels[(x * W + y) * 3 + 2];
            }
        }
    }

    newImage.pixels = pixels;
    return newImage;
}

int main()
{
    BMP image = readBMP("test.bmp");
    int input;

    cout << "************ Editing BMP Images ************\n"
         << endl;
    cout << "Choose from the list below : \n"
         << endl;
    cout << "   1) 180 degrees rotation" << endl;
    cout << "   2) 90 degrees rotation (right)" << endl;
    cout << "   3) 90 degrees rotation (left)" << endl;
    cout << "   4) Light decrease" << endl;
    cout << "   5) Changeing contrast" << endl;
    cout << "   6) Finding edges" << endl;

    cin >> input;

    switch (input)
    {
    case 1:
    {
        image = rotate180(image);
        writeBMP("rotate180.bmp", image);
        break;
    }
    case 2:
    {
        image = rotate90_r(image);
        writeBMP("rotate90-right.bmp", image);
        break;
    }
    case 3:
    {
        image = rotate90_l(image);
        writeBMP("rotate90-left.bmp", image);
        break;
    }
    case 4:
    {
        image = lighting_down(image);
        writeBMP("lighting-down.bmp", image);
        break;
    }
    case 5:
    {
        image = contrast(image);
        writeBMP("contrast.bmp", image);
        break;
    }
    case 6:
    {
        image = hollow(image);
        writeBMP("hollow.bmp", image);
        break;
    }
    }
    cout << "Your picture is ready.";

    return 0;
}
