#ifndef VISON_H
#define VISON_H

#include<iostream>
#include"opencv2/opencv.hpp"
using namespace std;
using namespace cv;

typedef struct
{
	Point center;
	Rect bound;
	vector<Point>contour;
}ConnectObj;

typedef struct
{
	float redRatio;
	float blueRatio;
	float synthesizeRtio;
}colorRatio;

class CObj
{
	public:
		CObj(ConnectObj Area);
		void imshowArea();
		int getAreaAsize();
		float getWHR();
	private:
		ConnectObj area;
		int areaSize;
		float WHRatio;
		float wheelRatio;
		Mat img;
		colorRatio cloRatio;
		float lightBarRatio;
		
};

class TrackObj
{
	public:
		TrackObj();
		int initObj(Mat src,Rect trackWindow);
		int track(Mat src,Rect& outputWindow );
		int updateObj(Mat src,Rect trackWindow);
		int updateHist(Mat srcHist,Mat curHist,Mat& dstHist);
		
	private:
		Rect trackWindow1,searchRange;
		Mat hist1,hist2,hist3,hist4;
		float hranges[2];
		const float* phranges;
		int hsize;
		

};
#endif
