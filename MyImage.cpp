#include "MyImage.h"
#include "util.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

MyImage::MyImage(IplImage* img):_myImg(NULL),_dstImg(NULL)
{
	_myImg = cvCreateImage(cvSize(img->width,img->height),8,img->depth);
	_myImg = cvCloneImage(img);
}
MyImage::MyImage(int width,int height,int depth):_myImg(NULL),_dstImg(NULL)
{
	_myImg = cvCreateImage(cvSize(width,height),8,depth);
}

MyImage::MyImage(std::string path):_dstImg(NULL)
{
    _myImg = cvLoadImage(path.c_str(), CV_LOAD_IMAGE_UNCHANGED);
}
MyImage& MyImage::operator=(const MyImage& in)
{
	if(_myImg!=NULL)
		cvReleaseImage(&_myImg);
	_myImg = cvCreateImage(cvSize(in.getImage()->width,in.getImage()->height),8,in.getImage()->depth);
	_myImg = cvCloneImage(in.getImage());
	return *this;
}

MyImage::MyImage(const MyImage& in)
{
	if(&in==this)
		return;
	_myImg = cvCreateImage(cvSize(in.getImage()->width,in.getImage()->height),8,in.getImage()->depth);
	_myImg = cvCloneImage(in.getImage());
}
std::string MyImage::getWidthHeight()
{

    if(_myImg==NULL)
        return "";
 
    char tmp[1024]={0};
    snprintf(tmp,1024,"%d_%d",_myImg->width,_myImg->height);
    return std::string(tmp);
}

bool MyImage::reSize(int width,int height)
{
    if(_myImg==NULL)
        return false;
    _dstImg = cvCreateImage(cvSize(width,height),_myImg->depth, _myImg->nChannels);
    cvResize(_myImg,_dstImg ,CV_INTER_AREA  );
    int ret = (_dstImg==NULL ?  false: true);
    return ret;
}

bool MyImage::_saveImg(IplImage* in,std::string path , std::string dir)
{
   if(in==NULL)
        return false;
   if(access(dir.c_str(),0))
        mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
   bool ret = (cvSaveImage(path.c_str(),in) == 1 ? true:false );
   return ret;
}


bool MyImage::saveSrcImg(std::string path , std::string dir)
{
    return  _saveImg(_myImg,path,dir);
}

bool MyImage::saveDstImg(std::string path , std::string dir)
{
    return  _saveImg(_dstImg,path,dir);
}

MyImage::~MyImage()
{
	if(_myImg!=NULL)
	{
		cvReleaseImage(&_myImg);
		_myImg=NULL;
	}

	if(_dstImg!=NULL)
	{
		cvReleaseImage(&_dstImg);
		_dstImg=NULL;
	}

}
Color MyImage::getColorPoint(int x ,int y)
{
	Color tmp;
	tmp.blue = ((char*)(_myImg->imageData+y*_myImg->widthStep))[3*x];
	tmp.green = ((char*)(_myImg->imageData+y*_myImg->widthStep))[3*x+1];
	tmp.red = ((char*)(_myImg->imageData+y*_myImg->widthStep))[3*x+2];
	return tmp;
}

int MyImage::setColorPoint(int x ,int y,const Color& in)
{
	if(x>_myImg->width || x<0 || y>_myImg->height || y<0)
		return -1;
	((char*)(_myImg->imageData+y*_myImg->widthStep))[3*x] = in.blue;
	((char*)(_myImg->imageData+y*_myImg->widthStep))[3*x+1] = in.green;
	((char*)(_myImg->imageData+y*_myImg->widthStep))[3*x+2] = in.red;
	return 0;
}
IplImage* MyImage::getImage() const
{
	return _myImg;
}

Color getRandColor()
{
	Color tmp;
	tmp.red = getRandInt(0,255);
	tmp.green = getRandInt(0,255);
	tmp.blue = getRandInt(0,255);
	//printf("db:%d,%d,%d\n",tmp.red,tmp.green,tmp.blue);
	return tmp;
}

Color getLimitColor(int in,int choose)
{
	Color tmp;
	tmp = getRandColor();
	switch(choose)
	{
		case 0:
			tmp.blue=in;
		break;
		case 1:
			tmp.green=in;
			break;
		case 2:
			tmp.red=in;
			break;
		default:
			break;
	}
	//printf("db:%d,%d,%d\n",tmp.red,tmp.green,tmp.blue);
	return tmp;
}
