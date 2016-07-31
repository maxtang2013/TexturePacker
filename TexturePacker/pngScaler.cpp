#include <iostream>
#include <fstream>
#include "TexturePacker.h"
#include "MyPngWriter.h"

struct MyColor {
	unsigned char r, g, b, a;
};

//   0.05 0.1 0.05
//   0.1  0.6 0.1
//   0.05 0.1 0.05
//
inline MyColor Filter(MyPngWriter* writer, float u, float v)
{
	int w = writer->getwidth(), h = writer->getheight();
	int x = (int)(w * u), y = (int) (h * v);

	float c[3][3] = {
		{0.05f, 0.1f, 0.05f,},
		{0.1f,  0.6f, 0.1f,},
		{0.05f, 0.1f, 0.05f}
	};

	float a = 0, r = 0, g = 0, b = 0;

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			unsigned char alpha = 0;
			int x1 = x + (i-1), y1 = y + (j-1);
			if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h)
			{
				a += c[i][j] * writer->getAlpha(x1, y1);
				r += c[i][j] * writer->getRed(x1, y1);
				g += c[i][j] * writer->getGreen(x1, y1);
				b += c[i][j] * writer->getBlue(x1, y1);
			}
		}
	}

	if (r + g + b < 10.0f) a = r = g = b = 0.0f;
	MyColor color = {(unsigned char)r, (unsigned char)g,(unsigned char)b,(unsigned char)a  };
	return color;
}

void ClearAlpha(const char* inFile, const char* outFile)
{
	MyPngWriter* inPngFile = new MyPngWriter(1, 1, 0, "");
	inPngFile->readfromfile(inFile);
	int w = inPngFile->getwidth();
	int h = inPngFile->getheight();

	MyPngWriter outPngFile(w, h, 0, outFile);

	for (int j = 0; j < h; ++j)
	{
		for (int i = 0; i < w; ++i)
		{
			int a = inPngFile->getAlpha(i, j);
			int r = inPngFile->getRed(i, j);
			int g = inPngFile->getGreen(i, j);
			int b = inPngFile->getBlue(i, j);

			//if (r + g + b < 5) a = r = g = b = 0;
			if (r + g + b >= 254 * 3) a = r = g = b = 0;
			
			outPngFile.plot(i, j, r, g, b, a);
		}
	}
	outPngFile.close();
	delete inPngFile;
}


int main1(int argc, char** argv)
{
	std::string filename("");

	if (argc != 5 && argc != 4 && argc != 3)
	{
		std::cout << "Usage:\n"
				  << "    PngScaler file width height\n";
		return -1;
	}

	if (argc == 4) {
		ClearAlpha(argv[1], argv[2]);
		return 0;
	}

	int outWidth, outHeight;
	MyPngWriter* inPngFile = new MyPngWriter(1, 1, 0, "");
	inPngFile->readfromfile(argv[1]);

	if (argc == 5)
	{
		outWidth= atoi(argv[3]);
		outHeight = atoi(argv[4]);
	}
	else
	{
		outWidth = inPngFile->getwidth()/2;
		outHeight = inPngFile->getheight()/2;
	}
	
	MyPngWriter outPngFile(outWidth, outHeight, 0, argv[2]);

	for (int j = 0; j < outHeight; ++j)
	{
		float v = j / (float) outHeight;
		for (int i = 0; i < outWidth; ++i)
		{
			float u = i / (float)outWidth;
			MyColor c = Filter(inPngFile,u,v);
			outPngFile.plot(i, j, c.r, c.g, c.b, c.a);
		}
	}
	outPngFile.close();
	delete inPngFile;
}


