#include <iostream>
#include <fstream>
#include "TextureSpaceArranger.h"
#include "MyPngWriter.h"
#include "BoundingGenerator.h"

using namespace std;

struct OriginFile {
    std::string path;
};

bool IsPointInside(const SpriteInfo& sprite, int x, int y);
inline bool InnerOn(int x0, int y0, int x1, int y1, int xp, int yp);

bool inSide(int poly[], int n, int x, int y)
{
    bool inside = true;
    for (int i = 0; i < n; ++i)
    {
        int x0 = poly[i * 2], y0 = poly[i * 2 + 1];
        int j = (i + 1) % n;
        int x1 = poly[j * 2], y1 = poly[j * 2 + 1];
        if (!InnerOn(x0, y0, x1, y1, x, y))
        {
            inside = false;
            break;
        }
    }
    return inside;
}

void testInnerOn()
{
    int polygon[] = {
        165, 0,
        0, 116,
        0, 212,
        170, 380,
        261, 380,
        382, 290,
        382, 233,
        244, 0,        
    };
    
    if (inSide(polygon, 8, 165, 116))
        printf ("test point (165, 116) inside OK!");
    else
        printf ("test point (165, 116) inside WRONG!");
}

int main()
{
    //testInnerOn();
    //return 0;
    //

    TextureSpaceArranger arrange(4096, 4096);
    std::vector<SpriteInfo> spriteInfos;

    std::string dir("C:\\dev\\old things\\in-game\\Result\\in-game\\");
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

            // 画出辅助调试的线框
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

            //outputFile.line(info.x, info.y, info.x + w, info.y, 255, 0, 255, 255);
            //outputFile.line(info.x + w, info.y + h, info.x + w, info.y, 255, 0, 255, 255);
            //outputFile.line(info.x, info.y + h, info.x + w, info.y + h, 255, 0, 255, 255);
            //outputFile.line(info.x, info.y + h, info.x, info.y, 255, 0, 255, 255);
        }
        else
        {
            logFile << "File " << filename << "with size(" << w << ", " << h << ") not packed!\n";
        }
    }

    outputFile.write_png();


    return 0;
}