#include <iostream>
#include <fstream>
#include "TexturePacker.h"
#include "MyPngWriter.h"
#include "BoundingGenerator.h"
#include "GeoUtil.h"

bool IsPointInside(const SpriteInfo& sprite, int x, int y);

void PrintUsage()
{
	std::cout << "Usage:\n"
			  << "    WeTexturePacker ListFile OutFileWidth OutFileHeight {DrawDebugLines}\n"
			  << "    List file should contain lines of paths to PNG files.\n";
}

void WriteOutPackedPng(int width, int height, const std::vector<SpriteInfo>& spriteInfos, bool drawDebugLines)
{
	std::ofstream logFile("log.txt");
	MyPngWriter outputFile(width, height, 0, "output.png");

	for (int i = 0; i < spriteInfos.size(); ++i)
	{
		const SpriteInfo& info = spriteInfos[i];

		std::string filename = *(std::string*)(info.userData);

		MyPngWriter inPngFile(1, 1, 0, "");
		inPngFile.readfromfile(filename.c_str());
		int w = inPngFile.getwidth();
		int h = inPngFile.getheight();

		if (info.fitted)
		{
			for (int y = 0; y < h; ++y)
			{
				for (int x = 0; x < w; x++)
				{
					if (IsPointInside(info, x + info.x, info.y + y))
					{
						int r = inPngFile.getRed(x, y);
						int g = inPngFile.getGreen(x, y);
						int b = inPngFile.getBlue(x, y);
						int a = inPngFile.getAlpha(x, y);

						outputFile.plot(info.x + x, info.y + y, r, g, b, a);
					}
				}
			}

			if (drawDebugLines)
			{
				// Draw the debugging lines.
				if (info.vertex.size() > 4)
				{
					int n = info.vertex.size();
					for (int j = 0; j < n; ++j)
					{
						CPoint pt0 = info.vertex[j];
						CPoint pt1 = info.vertex[(j+1)%n];
						outputFile.line(info.x + pt0.x, info.y + pt0.y, info.x + pt1.x, info.y + pt1.y, 255, 0, 0, 255);
					}
				}
			}
		}
		else
		{
			logFile << "File " << filename << "with size(" << w << ", " << h << ") not packed!\n";
		}
	}

	outputFile.close();
}

int main(int argc, char** argv)
{
	if (argc != 4 && argc != 5)
	{
		PrintUsage();
		return -1;
	}

	int width = atoi(argv[2]),
		height = atoi(argv[3]);

	bool drawDebugLines = argc == 5;

	if (width < 128) width = 128;
	if (width > 4096) width = 4096;
	if (height < 128) height = 128;
	if (height > 4096) height = 4096;
	
	TexturePacker packer(width, height);
    std::vector<SpriteInfo> spriteInfos;
	std::string listFilePath = argv[1];
    std::vector<std::string> fileList;
    std::ifstream inFile(listFilePath.c_str());

    while (!inFile.eof())
    {
        std::string line;
        inFile >> line;

        if (line.length() == 0) continue;
        if (inFile.eof()) break;

		if (line.length() > 4)
		{
			std::string suffix = line.substr(line.length() - 4);
			if (suffix[0] == '.'
				&& tolower(suffix[1]) == 'p'
				&& tolower(suffix[2]) == 'n' 
				&& tolower(suffix[3]) == 'g')
			{
				fileList.push_back(line);
			}
		}
    }

    for (int i = 0; i < fileList.size(); ++i)
    {
        std::string fileName = fileList[i];
        MyPngWriter inPngFile(1, 1, 0, fileName.c_str());
        inPngFile.readfromfile(fileName.c_str());

        SpriteInfo info;
        BoundingGenerator boundGen;
        info = boundGen.GenerateMoreCompactBounding(fileName);

        info.fitted = false;
        info.userData = &fileList[i];

        info.x = info.y = -1;

        spriteInfos.push_back(info);
    }
	
	printf("Generate compact bounding done, start packing...\n");
    packer.Pack(spriteInfos);
	printf("Pack done, writing out the packed file, it could take a while ...\n");

	WriteOutPackedPng(width, height, spriteInfos, drawDebugLines);

    return 0;
}