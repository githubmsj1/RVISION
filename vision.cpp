#include<iostream>
#include"opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#define ENABLE 0
#define DISABLE -1

#define DEBUG 1
#define RUN_STATUS DEBUG

#define VIDEO 0
#define CAMERA 1
#define PICTURE 2

#define RED 0
#define BLUE 1

#define NCOM "COM3"

#if RUN_STATUS==DEBUG
	#define PAUSE ENABLE
	#define IMG_SOURCE VIDEO
	#define TEAM_DEFAULT RED
#else
	#define PAUSE ENABLE       
	#define IMG_SOURCE PICTURE
	#define TEAM_DEFAULT RED
#endif



const char* srcPath="6.jpg";
const char* videoPath="v1.wmv";

Mat src,chel;	
int timeMs=0;
int team;

int detectEnemy(Mat src);
int connectedComponents(Mat src);

int main()
{
	
	team=TEAM_DEFAULT;
	
	VideoCapture cap;
	if(IMG_SOURCE==CAMERA)
	{
		
		cap.open(1);
		if(!cap.isOpened())
		{
			cout<<"cannot open the camera";
			return -1;
		}
	}
	else if(IMG_SOURCE==VIDEO)
	{
		cap.open(videoPath);
		if(!cap.isOpened())
		{
			cout<<"cannot open the video";
		}
	}

	namedWindow("Source",0);
	while(true)
	{	
		//aquire the source picture
		timeMs=getTickCount();
		if(IMG_SOURCE==CAMERA||IMG_SOURCE==VIDEO)
		{
			cap>>src;
		}
		else if(IMG_SOURCE==PICTURE)
		{
			src=imread(srcPath,1);
		}

		imshow("Source",src);
		
		detectEnemy(src);

		//control the processing period
		char waitTime=0;
		if(PAUSE==ENABLE)
		{	
			waitTime=-1;
		}
		else
		{
			waitTime=40;
		}
		if((char)waitKey(waitTime)=='q')
		{
			break;
		}
		

	}
	return 0;
}

int detectEnemy(Mat src)
{
	if(src.empty())
	{
		cout<<"Dnemy cannot be detected,because the source pirture is null"<<endl;
		return -1;
	}
	

 	//cout<<src.size()<<endl;
      	Mat srcYCrCb;
      	Mat srcSigCh;
      	cvtColor(src,srcYCrCb,CV_BGR2YCrCb);
     	vector<Mat>srcCh;
      	split(srcYCrCb,srcCh);      	
	if(team==RED)
      	{
      		srcSigCh=srcCh[1];
      	}
      	else if(team==BLUE)
      	{
      		srcSigCh=srcCh[2];
      	}
      	
	//imshow("sig",srcSigCh);
	Mat srcBin;
	threshold(srcSigCh,srcBin,180,255,THRESH_BINARY);
	imshow("Bin",srcBin);
	connectedComponents(srcBin);

      	return 0;

}

int connectedComponents(Mat src)
{
	Mat srcMorpho;

	Mat shapeEx=getStructuringElement(0,Size(3,3));
	morphologyEx(src,srcMorpho,MORPH_OPEN,shapeEx);
	morphologyEx(srcMorpho,srcMorpho,MORPH_CLOSE,shapeEx);
	imshow("Morpho",srcMorpho);

	Mat srcCanny;
	Canny(srcMorpho,srcCanny,50,100,3);
	vector<vector<Point> >contours;
	vector<Vec4i> hierarchy;
	findContours(srcCanny,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,Point(0,0));
	

	
	
	cout<<"Contours1: "<<contours.size()<<endl;
	Mat srcContour=Mat::zeros(src.size(),CV_8UC3);
	float perimscale=64;
	for(int i=0;i<contours.size();i++)
	{
		double len=arcLength(contours[i],1);
		double lenThreshold=(src.size().height+src.size().width)/perimscale;
		if(len<lenThreshold)
		{	
			cout<<">>"<<contours[i].size()<<endl;
			contours[i].clear();
			cout<<contours[i].size()<<endl;
		}

		

		//Scalar color=Scalar(255,255,255);
		//drawContours(srcContour,contours,i,color,2,8,hierarchy,0,Point());
	}
	cout<<"Contours2: "<<contours.size()<<endl;

	imshow("contour",srcContour);
	

	return 0;
}
