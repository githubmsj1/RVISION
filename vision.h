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
#endif
