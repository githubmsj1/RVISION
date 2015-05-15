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

#endif
