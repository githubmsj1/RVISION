#include<iostream>
#include"opencv2/opencv.hpp"
#include"vision.h"

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
	#define TEAM_DEFAULT BLUE
#else
	#define PAUSE ENABLE       
	#define IMG_SOURCE PICTURE
	#define TEAM_DEFAULT RED
#endif



const char* srcPath="6.jpg";
const char* videoPath="p1.avi";

Mat src,chel,tmpImg;
int tmpVar=0,tmpVar1=0,tmpMin=0,tmpMax=0;
int timeMs=0;
int team;


int detectEnemy(Mat src,vector<ConnectObj> &cO);
int connectedComponents(Mat src,vector<ConnectObj> &cO);
int recognizeEnemy(vector<ConnectObj> cO);
int detectFeatures(Mat src,Mat &dst);
int kmeansThresh(Mat src,Mat &dst);
void regulate(int,void*);
int drawCon(vector<ConnectObj> srcCon);
int combineCon(Mat src,Mat &dst,vector<ConnectObj> cO);

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
	namedWindow("thresh",CV_WINDOW_AUTOSIZE );
	createTrackbar( "Thresh1","thresh",&tmpVar, 255,regulate );
	createTrackbar( "Thresh2","thresh",&tmpVar1, 255,regulate );
	
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
		
		vector<ConnectObj> cO;
		Mat srcF;
		//detectEnemy(src,cO);
		
		pyrDown(src,src,Size(src.cols/2,src.rows/2));
		detectFeatures(src,srcF);
		imshow("feature",srcF);
		//connectedComponents(srcF,cO);

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


void regulate(int,void*)
{
	Mat tmpImg1;
	//threshold(tmpImg,tmpImg1,tmpVar,255,THRESH_BINARY);
	if(tmpVar>tmpVar1)
	{
		tmpMin=tmpVar1;
		tmpMax=tmpVar;
	}
	else
	{
		tmpMin=tmpVar;
		tmpMax=tmpVar1;
	}
	cout<<"Small "<<tmpVar<<endl;
	cout<<"Big "<<tmpVar1<<endl;
	//vector<int> up,down;
	//up.push_back(tmpVar1);
	//down.push_back(tmpVar);
	
	//Canny(tmpImg,tmpImg1,tmpMin,tmpMax,3);
	inRange(tmpImg,0,tmpMax,tmpImg1);
	imshow("thresh",tmpImg1);
}

int detectFeatures(Mat src,Mat &dst)
{
	
	if(src.empty())
	{
		cout<<"Dnemy cannot be detected,because the source pirture is null"<<endl;
		return -1;
	}
	
 	//cout<<src.size()<<endl;
      	Mat srcYCrCb;
	Mat srcHSV;
      	Mat srcSigCh;
	Mat srcGray;
	Mat srcEdge;

	Mat hsvBin;
	Mat grayBin;
	Mat ycrcbBin;
	
	
      	cvtColor(src,srcYCrCb,CV_BGR2YCrCb);
	cvtColor(src,srcHSV,CV_BGR2HSV);
	cvtColor(src,srcGray, CV_BGR2GRAY);//imshow("gray",srcGray);
	//Canny(srcGray,srcEdge,tmpMin,tmpMax,3);imshow("edge",srcEdge);
 	
 	vector<Mat>srcCh;

      	//YCrCb
	split(srcYCrCb,srcCh);   
	srcYCrCb.release();
	if(team==RED)
      	{	
      		srcYCrCb=srcCh[2];
      	}
      	else if(team==BLUE)
      	{
      		srcYCrCb=srcCh[1];
      	}
	inRange(srcYCrCb,136,255,ycrcbBin);imshow("ycrcbb",ycrcbBin);
	//dilate(ycrcbBin,ycrcbBin,getStructuringElement(0,Size(2,2)));
	//vector<ConnectObj> ycrcbO;
	//connectedComponents(ycrcbBin,ycrcbO);//drawCon(grayO);

	//tmpImg=srcYCrCb;
	
      	//Gray
	inRange(srcGray,0,31,grayBin);
	//morphologyEx(srcYCrCb,srcYCrCb,MORPH_OPEN,getStructuringElement(0,Size(5,5)));
	imshow("grayb",grayBin);
	//vector<ConnectObj> grayO;
	//connectedComponents(grayBin,grayO);//drawCon(grayO);
	tmpImg=srcGray;

	//HSV
	inRange(srcHSV,Scalar(65,170,150),Scalar(86,255,255),hsvBin);
	vector<ConnectObj> hsvO;
	connectedComponents(hsvBin,hsvO);//drawCon(grayO);
	//morphologyEx(srcYCrCb,srcYCrCb,MORPH_OPEN,getStructuringElement(0,Size(5,5)));
	//imshow("hsvbin",hsvBin);
	
	

	//combinCon(hsvBin,);
	//combineCon(grayBin,ycrcbO);
	//imshow("sig",srcSigCh);
	//equalizeHist(srcSigCh,srcSigCh);
	//tmpImg=srcSigCh;
	

	//imshow("sig1",srcSigCh);
	Mat srcBin;
	
	//binary
	//inRange(srcSigCh,0,81,srcBin);	
	//inRange(srcSigCh,0,50,srcBin);

	//threshold(srcSigCh,srcBin,180,255,THRESH_BINARY);
	//imshow("Bin",srcBin);

	//vector<ConnectObj> cO;

	//gray&ycrcb
	Mat grayAYcrcb;
	grayAYcrcb=grayBin&ycrcbBin;
	//dilate(grayAYcrcb,grayAYcrcb,getStructuringElement(0,Size(7,7)));imshow("&&&",grayAYcrcb);
	//vector<ConnectObj> yCO;
	//connectedComponents(grayAYcrcb,yCO);//drawCon(gYCO);
	
	Mat grayOYcrcb;
	grayOYcrcb=grayBin|ycrcbBin;imshow("|||",grayOYcrcb);
	//combineCon(grayOYcrcb,srcBin,yCO);
	//vector<ConnectObj> gYCO;
	//connectedComponents(srcBin,gYCO);

	srcBin=grayOYcrcb;//ycrcbBin;
	dst=srcBin;
	//if(cO.size()>0)
	//{
	//	CObj obj1=CObj(cO[0]);
	//	obj1.imshowArea();
	//}

	return 0;
}

int kmeansThresh(Mat src,Mat &dst)
{	
	
	Mat labels;

	//RNG rng(12345);
	//int i, sampleCount = rng.uniform(1, 1001);
	//Mat points(sampleCount, 1, CV_32FC2);
	//cout<<points.size()<<endl;
	//kmeans(points, 2,labels,TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0),3, KMEANS_PP_CENTERS);

	
	
	Mat src1=src.reshape(0,src.rows*src.cols);
	src1.convertTo(src1,CV_32F);
	cout<<src1.size();
	TermCriteria crt=TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 5, 1);
	kmeans(src1,2,labels,crt,1,KMEANS_RANDOM_CENTERS);
	cout<<labels.size()<<endl;
	return 0;
}	

int detectEnemy(Mat src,vector<ConnectObj> &cO)
{
	if(src.empty())
	{
		cout<<"Dnemy cannot be detected,because the source pirture is null"<<endl;
		return -1;
	}
	

 	//cout<<src.size()<<endl;
      	Mat srcYCrCb;
      	Mat srcSigCh;
	Mat srcHSV;
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
      	
	imshow("Featuresrc",srcSigCh);
	Mat srcBin;
	threshold(srcSigCh,srcBin,180,255,THRESH_BINARY);
	imshow("Bin",srcBin);

	//vector<ConnectObj> cO;
	connectedComponents(srcBin,cO);

	//if(cO.size()>0)
	//{
	//	CObj obj1=CObj(cO[0]);
	//	obj1.imshowArea();
	//}

	return 0;

}

int drawCon(vector<ConnectObj> srcCon)
{
	
	Mat src=Mat::zeros(500,500,CV_8UC3);
	vector<vector<Point> >contours;
	for(int i=0;i<srcCon.size();i++)
	{
		
		
		contours.push_back(srcCon[i].contour);
		drawContours(src,contours,0,Scalar(0,255,0),2,8);
		circle(src,srcCon[i].center,4,Scalar(0,0,255),-1);
		rectangle(src,srcCon[i].bound.tl(),srcCon[i].bound.br(),Scalar(0,0,255),1);
		contours.pop_back();
	}
	imshow("Draw area",src);
	return 0;
}

int connectedComponents(Mat src,vector<ConnectObj> &cO)
{
	Mat srcMorpho;

	Mat shapeEx=getStructuringElement(0,Size(3,3));
	morphologyEx(src,srcMorpho,MORPH_OPEN,shapeEx);
	morphologyEx(srcMorpho,srcMorpho,MORPH_CLOSE,shapeEx);
	//imshow("Morpho",srcMorpho);

	Mat srcCanny;
	Canny(srcMorpho,srcCanny,50,100,3);
	vector<vector<Point> >contours,contours1;
	vector<Vec4i> hierarchy;
	findContours(srcCanny,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE,Point(0,0));
	vector<vector<Point> >contoursPoly(contours.size());

	
	
	//cout<<"Contours1: "<<contours.size()<<endl;
	Mat srcContour=Mat::zeros(src.size(),CV_8UC3);
	//Mat srcContourFit=Mat::zeros(src,size(),CV_8UC3);

	float perimscale=64;
	
	//vector<ConnectObj>::iterator iter=cO.begin();
	//filer the contours and smooth it
	for(int i=0,j=0;i<contours.size();i++)
	{
		double len=arcLength(contours[i],1);
		double lenThreshold=(src.size().height+src.size().width)/perimscale;
		if(len<lenThreshold)
		{	
			//cout<<">>"<<contours[i].size()<<endl;
			//contours.erase(contours.begin()+i);
			
			//cout<<"remove the small contour"<<endl;
			//contours[i].clear();
			//cout<<contours[i].size()<<endl;
			
		}
		else//smooth it
		{

			Scalar color=Scalar(255,0,0);
			drawContours(srcContour,contours,i,color,2,8,hierarchy,0,Point());
			
			contours1.resize(contours1.size()+1);
			//approxPolyDP(Mat(contours[i]),contours[i],3,true);
			approxPolyDP(Mat(contours[i]),contours1.back(),3,true);
			//cout<<contours1.size()<<endl;

			ConnectObj c1;
			vector<Point>::iterator iter2;
			iter2=contours1.back().begin();
			for(;iter2!=contours1.back().end();iter2++)
			{
				c1.contour.push_back(*iter2);
			}
  		        
			cO.push_back(c1);
			//color.val[0]=0;color.val[1]=255;color.val[2]=0;
			drawContours(srcContour,contours1,contours1.size()-1,Scalar(0,255,0),2,8,hierarchy,0,Point());
		}


	}

	
	if(contours1.size()!=0)
	{

			//cout<<"fuck"<<endl;
		
		//Calculate the center of mass and the surranding rectangle
		vector<Moments>mu(contours1.size());
		for(int i=0;i<contours1.size();i++)
		{
			mu[i]=moments(contours1[i],false);
		}

		vector<Point>mc(contours1.size());
		//vector<ConnectObj>::iterator iter;
		//iter=cO.begin();
		for(int i=0;i<contours1.size();i++/*,iter++*/)
		{
			mc[i]=Point(mu[i].m10/mu[i].m00,mu[i].m01/mu[i].m00);
			cO[i].center=mc[i];
			circle(srcContour,mc[i],4,Scalar(0,0,255),-1);

		}	
		
		
		vector<RotatedRect>minRect(contours1.size());
		vector<Rect>boundRect(contours1.size());
		for(int i=0;i<contours1.size();i++)
		{
			//minRect[i]=minAreaRect(Mat(contours[i]));
			boundRect[i]=boundingRect(Mat(contours1[i]));
			cO[i].bound=boundRect[i];
			rectangle(srcContour,boundRect[i].tl(),boundRect[i].br(),Scalar(0,0,255),1);
		}
		
	}
	else
	{
		return -1;
	}
			

		

	//cout<<"Contours2: "<<contours.size()<<endl;
	
	imshow("contour",srcContour);
	
	return 0;
}

int combineCon(Mat src,Mat &dst,vector<ConnectObj> cO)
{
	
	if(dst.cols==0)
	{
		dst=Mat::zeros(src.size(),CV_8UC1);
	}
	//cout<<cO.size()<<endl;
	for(int i=0;i<cO.size();i++)
	{
		int h=cO[i].bound.height;
		int w=cO[i].bound.width;
		int rowh=cO[i].bound.tl().y-h/2;
		int colh=cO[i].bound.tl().x-w/2;
		int rowe=rowh+h*2;
		int cole=colh+w*2;
		for(int r=rowh;r<rowe;r++)
		{
			for(int c=colh;c<cole;c++)
			{
				//cout<<c<<endl;
				dst.at<uchar>(r,c)=src.at<uchar>(r,c);//x,y

			}
		}

	}//at(row,col)
	
	dilate(dst,dst,getStructuringElement(0,Size(7,7)));
	imshow("combine",dst);
	return 0;
}

int recognizeEnemy(vector<ConnectObj> cO)
{
	
	return 0;
}
/******************CLass CObj********************/
void CObj::imshowArea()
{
	
	Mat src=Mat::zeros(320,480,CV_8UC3);
	vector<vector<Point> >contours;
	contours.push_back(area.contour);
	drawContours(src,contours,0,Scalar(0,255,0),2,8);
	circle(src,area.center,4,Scalar(0,0,255),-1);
	rectangle(src,area.bound.tl(),area.bound.br(),Scalar(0,0,255),1);
	imshow("Draw area",src);
}

CObj::CObj(ConnectObj src)
{
	area.center=src.center;
	area.bound=src.bound;

	
	for(vector<Point>::iterator iter=src.contour.begin();iter!=src.contour.end();iter++)
	{	
		area.contour.resize(area.contour.size()+1);
		area.contour.back()=*iter;
	}
}

