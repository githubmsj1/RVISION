
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "vision.h"

#include <iostream>
#include <ctype.h>

TrackObj::TrackObj()
{
    hsize = 16;
    hranges[0]=0;
    hranges[1]=255;
    phranges = hranges;
		
}

int TrackObj::initObj(Mat image,Rect trackWindow)
{	

	//take the search image
	Mat image1=Mat(image,trackWindow);
	trackWindow1=trackWindow;

	//Mat mask = Mat::ones(image.size(), CV_8U);
	//Mat maskroi(mask, trackWindow);
	Mat maskroi=Mat::ones(image1.size(),CV_8U);
        int ch1[] = {0, 0};
	int ch2[] = {1, 0};
	int ch3[] = {2, 0};
	Mat chD1,chD2,chD3;
	chD1.create(image1.size(), image1.depth());
	chD2.create(image1.size(), image1.depth());
	chD3.create(image1.size(), image1.depth());

        mixChannels(&image1, 1, &chD1, 1, ch1, 1);
	mixChannels(&image1, 1, &chD2, 1, ch2, 1);
	mixChannels(&image1, 1, &chD3, 1, ch3, 1);
	calcHist(&chD1, 1, 0, maskroi, hist1, 1, &hsize, &phranges);
	calcHist(&chD2, 1, 0, maskroi, hist2, 1, &hsize, &phranges);
	calcHist(&chD3, 1, 0, maskroi, hist3, 1, &hsize, &phranges);
        
	normalize(hist1, hist1, 0, 255, CV_MINMAX);
	normalize(hist2, hist2, 0, 255, CV_MINMAX);
	normalize(hist3, hist3, 0, 255, CV_MINMAX);
	
	return 0;
                		
}

int TrackObj::updateObj(Mat image,Rect trackWindow)
{
	//take the search image
	Mat image1=Mat(image,trackWindow);
	trackWindow1=trackWindow;

	//Mat mask = Mat::ones(image.size(), CV_8U);
	//Mat maskroi(mask, trackWindow);
	Mat maskroi=Mat::ones(image1.size(),CV_8U);
        int ch1[] = {0, 0};
	int ch2[] = {1, 0};
	int ch3[] = {2, 0};
	Mat chD1,chD2,chD3;
	chD1.create(image1.size(), image1.depth());
	chD2.create(image1.size(), image1.depth());
	chD3.create(image1.size(), image1.depth());

	Mat tmphist1,tmphist2,tmphist3;
        mixChannels(&image1, 1, &chD1, 1, ch1, 1);
	mixChannels(&image1, 1, &chD2, 1, ch2, 1);
	mixChannels(&image1, 1, &chD3, 1, ch3, 1);
	calcHist(&chD1, 1, 0, maskroi, tmphist1, 1, &hsize, &phranges);
	calcHist(&chD2, 1, 0, maskroi, tmphist2, 1, &hsize, &phranges);
	calcHist(&chD3, 1, 0, maskroi, tmphist3, 1, &hsize, &phranges);
        
	normalize(tmphist1, tmphist1, 0, 255, CV_MINMAX);
	normalize(tmphist2, tmphist2, 0, 255, CV_MINMAX);
	normalize(tmphist3, tmphist3, 0, 255, CV_MINMAX);
	
	updateHist(tmphist1,hist1,hist1);
	//updateHist(tmphist2,hist2,hist2);
	//updateHist(tmphist3,hist3,hist3);
	
	//cout<<(int)hist1.at<uchar>(2)<<endl;
	return 0;
	
}

int TrackObj::updateHist(Mat srcHist,Mat curHist,Mat& dstHist)
{
	int weight=0;
	for(int i=0;i<srcHist.rows;i++)
	{
		
		dstHist.at<uchar>(i)=srcHist.at<uchar>(i)*weight/100+curHist.at<uchar>(i)*(100-weight)/100;
		//dstHist.at<uchar>(i)=srcHist.at<uchar>(i);
		
		if(i==2)
		{
			cout<<(int)srcHist.at<uchar>(2)<<" "<<(int)curHist.at<uchar>(2)<<" "<<(int)dstHist.at<uchar>(2)<<endl;
			
		}
	}

	return 0;
}

int TrackObj::track(Mat image,Rect& outputWindow)
{
	
	//Build the search range
	//outputWindow=trackWindow;
	

	int sca=2;
	searchRange=outputWindow;
	searchRange.height=MIN(image.rows,searchRange.height*sca);
	searchRange.width=MIN(image.cols,searchRange.width*sca);
	searchRange.x-=searchRange.width/4;
	searchRange.y-=searchRange.height/4;
	searchRange.x=MAX(searchRange.x,0);
	searchRange.y=MAX(searchRange.y,0);
	
	if(image.cols-searchRange.x<searchRange.width)
	{
		searchRange.x=image.cols-searchRange.width;
	}
	if(image.rows-searchRange.y<searchRange.height)
	{
		searchRange.y=image.rows-searchRange.height;
	}
	
	//take the search image
	Mat image1=Mat(image,searchRange);
	Mat chD1,chD2,chD3;
	chD1.create(image1.size(), image1.depth());
	chD2.create(image1.size(), image1.depth());
	chD3.create(image1.size(), image1.depth());
        
	int ch1[] = {0, 0};
	int ch2[] = {1, 0};
	int ch3[] = {2, 0};
        
	mixChannels(&image1, 1, &chD1, 1, ch1, 1);
	mixChannels(&image1, 1, &chD2, 1, ch2, 1);
	mixChannels(&image1, 1, &chD3, 1, ch3, 1);

	//calculate the backproject
	Mat backproj1,backproj2,backproj3;
	calcBackProject(&chD1, 1, 0, hist1, backproj1, &phranges);
	calcBackProject(&chD2, 1, 0, hist2, backproj2, &phranges);
	calcBackProject(&chD3, 1, 0, hist3, backproj3, &phranges);			
	
	Mat backproj;
	backproj1.copyTo(backproj);
	for(int r=0;r<backproj1.rows;r++)
	{
		for(int c=0;c<backproj1.cols;c++)
		{
			backproj.at<uchar>(r,c)=MIN(backproj.at<uchar>(r,c),backproj2.at<uchar>(r,c));
			backproj.at<uchar>(r,c)=MIN(backproj.at<uchar>(r,c),backproj3.at<uchar>(r,c));
		}
	}
	imshow("back",backproj);
				
	outputWindow.x=image1.cols/2-outputWindow.width/2;
	outputWindow.y=image1.rows/2-outputWindow.height/2;

	//cout<<image1.cols<<" "<<outputWindow.width<<endl;
	meanShift(backproj, outputWindow,TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
        //rectangle(image1,Point(outputWindow.x,outputWindow.y),Point(outputWindow.x+outputWindow.width,trackWindow1.y+trackWindow1.height),Scalar(0,0,255),2,CV_AA);
	
	imshow("region",image1);
        outputWindow.x=outputWindow.x+searchRange.x;
	outputWindow.y=outputWindow.y+searchRange.y;
	return 0;
}

