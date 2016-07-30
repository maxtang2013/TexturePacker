#include <iostream>
#include <fstream>
#include "TextureSpaceArranger.h"
#include "MyPngWriter.h"
#include "BoundingGenerator.h"
#include "GeoUtil.h"

bool IsPointInside(const SpriteInfo& sprite, int x, int y);

int main()
{
    TextureSpaceArranger arrange(4096, 4096);
    std::vector<SpriteInfo> spriteInfos;

    std::string dir("C:\\dev\\Sprites1\\");
    std::string listFilePath = dir + "texList.txt";

    std::vector<std::string> fileList;

    std::ifstream inFile(listFilePath.c_str());

    std::ofstream logFile("log.txt");

    while (!inFile.eof())
    {
        std::string line;
        inFile >> line;

        if (line.length() == 0) continue;
        if (inFile.eof()) break;

        fileList.push_back(line);
    }

    for (int i = 0; i < fileList.size(); ++i)
    {
        std::string fileName = fileList[i];
        MyPngWriter inPngFile(1, 1, 0, (dir + fileName).c_str());
        inPngFile.readfromfile((dir + fileName).c_str());

        SpriteInfo info;
        BoundingGenerator boundGen;
        info = boundGen.GenerateOverlayRects(dir + fileName);

        info.fitted = false;
        info.userData = &fileList[i];

        info.x = info.y = -1;

        spriteInfos.push_back(info);
    }

    arrange.DoArrange(spriteInfos);

    MyPngWriter outputFile(4096, 4096, 0, "output.png");

    for (int i = 0; i < spriteInfos.size(); ++i)
    {
        const SpriteInfo& info = spriteInfos[i];

        std::string filename = *(std::string*)(info.userData);

        MyPngWriter inPngFile(1, 1, 0, "");
        inPngFile.readfromfile((dir + filename).c_str());
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
        else
        {
            logFile << "File " << filename << "with size(" << w << ", " << h << ") not packed!\n";
        }
    }

    outputFile.write_png();


    return 0;
}