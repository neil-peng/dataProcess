#pragma once
#include "opencv.h"
#include <string>
struct Color
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};

class MyImage
{
	IplImage* _myImg;
	IplImage* _dstImg;
    bool _saveImg(IplImage* in,std::string path , std::string dir);
public:
	MyImage():_myImg(NULL),_dstImg(NULL){}
	MyImage(IplImage* img);
	MyImage(std::string imgPath);
    bool reSize(int width,int heigth);
    bool saveSrcImg(std::string path, std::string dir);
    bool saveDstImg(std::string path, std::string dir);
	MyImage(int width,int height,int depth);
	MyImage& operator=(const MyImage& in);
	MyImage(const MyImage& in);
	~MyImage();
	int setColorPoint(int x ,int y,const Color& in);
	Color getColorPoint(int,int);
	IplImage* getImage() const;
    std::string getWidthHeight();
};

extern Color getLimitColor(int in,int choose);
extern Color getRandColor();
