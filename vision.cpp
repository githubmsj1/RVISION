#include<iostream>
#include"opencv2/opencv.hpp"
#include"vision.h"
#include"serial.h"

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
#define CAMNUM 2

#define SCALE 2
#define WIDTH 720
#define HIGHT 480
#define PORTWIDTH 160 
#define PORTHIGHT 120

#if RUN_STATUS==DEBUG
	#define PAUSE 	DISABLE
	#define IMG_SOURCE CAMERA//VIDEO
	#define TEAM_DEFAULT BLUE
	#define PORT ENABLE
#else
	#define PAUSE DISABLE       
	#define IMG_SOURCE PICTURE
	#define TEAM_DEFAULT RED
	#define PORT DISABLE
#endif



const char* srcPath="6.jpg";
const char* videoPath="p5.avi";

Mat src,chel,tmpImg;
int tmpVar=0,tmpVar1=0,tmpMin=0,tmpMax=0;
int timeMs=0;
int team;
bool onView=false;
int reduction=1;



int detectEnemy(Mat src,vector<ConnectObj> &cO);
int connectedComponents(Mat src,vector<ConnectObj> &cO);
int recognizeEnemy(vector<ConnectObj> cO);
int detectFeatures(Mat src,Mat &dst);
int kmeansThresh(Mat src,Mat &dst);
void regulate(int,void*);
int drawCon(vector<ConnectObj> srcCon);
int combineCon(Mat src,Mat &dst,vector<ConnectObj> cO);
int lightBarDetect(Mat src,Rect &roi,Mat& lightMask);
int carShellDetect(Mat src,Rect roi,Rect &shell,Rect &roi1,Point input,Point &output,Mat lightMask);
int filter(vector<Point>& objs,Point input,Point& output);


int main()
{
	
	team=TEAM_DEFAULT;
	
	VideoCapture cap;
	Serial serial;
	unsigned char sendBuff[10];

	if(IMG_SOURCE==CAMERA)
	{
		
		cap.open(CAMNUM);
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
	
	if(PORT==ENABLE)
	{
		serial.init();
		reduction=WIDTH/SCALE/PORTWIDTH;
		
	}
	

	namedWindow("Source",0);
	namedWindow("thresh",CV_WINDOW_AUTOSIZE );
	createTrackbar( "Thresh1","thresh",&tmpVar, 255,regulate );
	createTrackbar( "Thresh2","thresh",&tmpVar1, 255,regulate );
	vector<Point>objs;
	TrackObj track=TrackObj();
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
		pyrDown(src,src,Size(src.cols/SCALE,src.rows/SCALE));
		
		
		Rect region,region1,region2;
		Mat src1;
		
		src.copyTo(src1);
		
		Mat lightMask;
		int light=lightBarDetect(src,region,lightMask);
		
		Point objCenter=Point(region.x+region.width/2,region.y+region.height/2);
		static Point oldObjCenter=objCenter;
		
		if(light==0)
		{

			track.initObj(src,region);
			if(onView==true)
			{
				if(carShellDetect(src1,region,region1,region2,oldObjCenter,objCenter,lightMask)!=0)
				{
					objCenter=Point(region.x+region.width/2,region.y+region.height/2);
					
				}
				
				filter(objs,objCenter,objCenter);
				rectangle(src1,region1.tl(),region1.br(),Scalar(0,255,0),1);
				rectangle(src1,region.tl(),region.br(),Scalar(0,0,255),1);
				circle(src1,objCenter,4,Scalar(0,0,255),-1);
			}
		}
		else
		{
			if(onView==true)
			{
				//cout<<"track"<<endl;
				track.track(src,region);
				objCenter=Point(region.x+region.width/2,region.y+region.height/2);
				rectangle(src1,region.tl(),region.br(),Scalar(255,0,0),2);
				circle(src1,objCenter,4,Scalar(0,0,255),-1);
			}
		}
		 
		oldObjCenter=objCenter;
		if(PORT==ENABLE)
		{
			
			sendBuff[0]=0xf2;
			sendBuff[1]=objCenter.x/reduction;
			sendBuff[2]=objCenter.y/reduction;
			
			serial.send_data_tty(sendBuff,3);
			//cout<<(int)sendBuff[1]<<" "<<(int)sendBuff[2]<<endl;
		}


		//cout<<objCenter<<endl;
		
		imshow("src1",src1);
		

		//detectFeatures(src,srcF);
		//imshow("feature",srcF);
		//connectedComponents(srcF,cO);


		//control the processing period
		char waitTime=0;
		if(PAUSE==ENABLE)
		{	
			waitTime=-1;
		}
		else
		{
			waitTime=10;
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
	inRange(tmpImg,tmpMax,255,tmpImg1);
	imshow("thresh",Mat::zeros(400,400,CV_8UC1));
	imshow("output",tmpImg1);

}

int filter(vector<Point>& objs,Point input,Point& output)
{	
	
	if(objs.size()==3)
	{
		
		//objs.erase(objs.begin());
		
		//int dis1=abs(objs[2].x+objs[2].y-objs[1].x-objs[1].y);
		//int dis2=abs(objs[1].x+objs[1].y-objs[0].x-objs[0].y);
		//if(dis1>2*dis2)
		//{
		//	objs.pop_back();
		//}
		//else
		//{
		//	objs.erase(objs.begin());
		//}
		output=objs.back()*0.8+input*0.2;
		objs.push_back(output);
		objs.erase(objs.begin());
	}
	else
	{
		objs.push_back(input);	
	}
	
	return 0;

}

//lightmask:the image where the light aera are sat 255
int lightBarDetect(Mat src,Rect &roi,Mat& lightMask)
{
	static Rect oldRoi;

	Mat srcHSV,hsvBin,hsvBinR,hsvBinR1,hsvBinR2,lightBar;
	vector<Mat>srcCh;
	cvtColor(src,srcHSV,CV_BGR2HSV);
	//inRange(srcHSV,Scalar(65,170,150),Scalar(86,255,255),hsvBin);
	inRange(srcHSV,Scalar(220/2,255*0.5,255*0.7),Scalar(235/2,255*1,255*1),hsvBin);
	inRange(srcHSV,Scalar(0/2,255*0.5,255*0.7),Scalar(10/2,255*1,255*1),hsvBinR1);
	inRange(srcHSV,Scalar(335/2,255*0.5,255*0.7),Scalar(360/2,255*1,255*1),hsvBinR2);
	inRange(srcHSV,Scalar(0/2,255*0,255*0.85),Scalar(360/2,255*0.2,255*1),lightBar);
	dilate(lightBar,lightBar,getStructuringElement(0,Size(10,10)));
	
	hsvBin=(hsvBin|hsvBinR1|hsvBinR2)&lightBar;//&lightBar;
	
	dilate(hsvBin,hsvBin,getStructuringElement(0,Size(10,10)));
	hsvBin.copyTo(lightMask);//copy the light bin to mask for shell detect
	lightMask=~lightMask;

	imshow("hsv",hsvBin);
	imshow("lightBar",lightBar);

	vector<ConnectObj> cO;
	connectedComponents(hsvBin,cO);
	if(cO.size()>0)
	{	
		onView=true;

		roi.width=MIN(cO[0].bound.width*4,src.cols);
		roi.height=MIN(cO[0].bound.height*6,src.rows);
		roi.y=cO[0].bound.y+5*cO[0].bound.width;
		roi.x=cO[0].bound.x-(roi.width-cO[0].bound.width)/2;
		roi.x=MAX(roi.x,0);
		roi.y=MAX(roi.y,0);
		if(src.cols-roi.x<roi.width)
		{
			roi.x=src.cols-roi.width;
		}
		if(src.rows-roi.y<roi.height)
		{
			roi.y=src.rows-roi.height;
		}
		//int oldWidth=roi.width;
		//roi=cO[0].bound;
		//roi.y=roi.y+roi.width/2;
		//roi.height=3*roi.width;
		//roi.width=4*roi.width;
		//roi.x=roi.x-(roi.width-oldWidth)/2;
		
		//roi.x=MIN(MAX(roi.x,0),src.rows-1);
		//roi.y=MIN(MAX(roi.y,0),src.cols-1);
		oldRoi=roi;
		
		
		//cout<<src.size()<<roi<<endl;
		Mat tmp(src,roi);
		imshow("lightbarregion",tmp);
		
		return 0;
	}
	else
	{	
		if(onView==true)
		{
			roi=oldRoi;
		}	
		return -1;
	}
}


//origin:last point
int carShellDetect(Mat src,Rect roi,Rect &shell,Rect &roi1,Point input,Point &output,Mat lightMask)
{
	//2:gray 1:crcb
	//static Point origin=Point(roi.x+roi.width/2,roi.y+roi.height);

	


	bool grayDetect=false;
	bool ycrcbDetect=false;
	Point origin=input;
	Point origin1,origin2;
	int dis1=999,dis2=999;
	Rect shell1,shell2;
	
	Rect roii;
	Point br=roi.br();
	br.x=MIN(MAX(br.x,0),src.cols-1);
	br.y=MIN(MAX(br.y,0),src.rows-1);
	roii.x=MIN(MAX(roi.x,0),src.cols-1);
	roii.y=MIN(MAX(roi.y,0),src.rows-1);
	roii.width=br.x-roii.x+1;
	roii.height=br.y-roii.y+1;
	
	

	Mat src1(src,roii),srcYCrCb,ycrcbBin,ycrcbBin1;
	Mat lightMask1(lightMask,roii);

	Mat temp=Mat::zeros(src.size(),CV_8UC3);
	
	//process the car color
	vector<Mat>srcCh;
      	cvtColor(src1,srcYCrCb,CV_BGR2YCrCb);
	split(srcYCrCb,srcCh);   
	srcYCrCb=srcCh[1];
	tmpImg=srcYCrCb;
	
	imshow("lightMask",lightMask);
	//cout<<srcYCrCb.type()<<" "<<lightMask.type()<<endl;

	
	inRange(srcYCrCb,165,255,ycrcbBin);//imshow("ycrcbb",ycrcbBin);//136,225
	inRange(srcYCrCb,0,100,ycrcbBin1);
	ycrcbBin=(ycrcbBin|ycrcbBin1)&lightMask1;

	dilate(ycrcbBin,ycrcbBin,getStructuringElement(0,Size(3,3)));
	vector<ConnectObj> cO;
	connectedComponents(ycrcbBin,cO); //imshow("ycrcb",srcYCrCb);
	
	//imshow("ycrcbbin1",ycrcbBin1);



	Mat  srcGray,grayBin;
	cvtColor(src1,srcGray, CV_BGR2GRAY);//imshow("gray",srcGray)
	inRange(srcGray,0,31,grayBin);imshow("gray",grayBin);
	//dilate(srcGray,srcGray,getStructuringElement(0,Size(10,10)));

	
	Moments mu;
	mu=moments(grayBin,false);
	Point center=Point(mu.m10/mu.m00,mu.m01/mu.m00);

	if(center.x>0&&center.y>0)
	{
		int dx=center.x-roi.width/2;
		int dy=center.y-roi.height/2;
		shell2=roi;
		shell2.x=roi.x+dx;
		shell2.y=roi.y+dy;
		origin2.x=shell2.x+shell2.width/2;	
		origin2.y=shell2.y+shell2.height/2;
		dis2=abs(origin2.x+origin2.y-origin.x-origin.y);
		//cout<<center<<endl;
		grayDetect=true;
	}
	if(cO.size()>0)//cO.size()>0)
	{
		
		if(cO.size()>1)
		{
			int mindis;
			int minidx=0;
			int cOX=cO[0].bound.x+cO[0].bound.width/2+roi.x;
			int cOY=cO[0].bound.y+cO[0].bound.height/2+roi.y;
			mindis=abs(cOX+cOY-origin.x-origin.y);
			for(int i=1;i<cO.size();i++)
			{
				cOX=cO[i].bound.x+cO[i].bound.width/2+roi.x;
				cOY=cO[i].bound.y+cO[i].bound.height/2+roi.y;
				int dis=abs(cOX+cOY-origin.x-origin.y);

				if(dis<mindis)
				{
					minidx=i;
					mindis=dis;
				}
			}
		
			shell1=cO[minidx].bound;
		}
		else
		{
			shell1=cO[0].bound;
		}
		shell1.x=shell1.x+roi.x;
		shell1.y=shell1.y+roi.y;
		origin1.x=shell1.x+shell1.width/2;
		origin1.y=shell1.y+shell1.height/2;
		dis1=abs(origin1.x+origin1.y-origin.x-origin.y);
		
			

		//circle(temp,origin1,4,Scalar(0,0,255),-1);
		//circle(temp,origin2,4,Scalar(0,255,0),-1);
		//circle(temp,input,5,Scalar(255,0,0),-1);
		//circle(temp,output,5,Scalar(255,0,0),2);
		ycrcbDetect=true;
	}

	if(grayDetect==true&&ycrcbDetect==true)
	{
		if(dis1>dis2)
		{
			output.x=origin2.x;
			output.y=origin2.y;
			shell=shell2;
			cout<<"gray"<<endl;

		}
		else
		{
			output.x=origin1.x;
			output.y=origin1.y;
			shell=shell1;
			cout<<"crcb"<<endl;

		}
		return 0;
		
	}
	if(grayDetect==true&&ycrcbDetect==false)
	{
		output.x=origin2.x;
		output.y=origin2.y;
		shell=shell2;
		cout<<"gray"<<endl;

		return 0;
	
	}
	if(grayDetect==false&&ycrcbDetect==true)
	{
		output.x=origin1.x;
		output.y=origin1.y;
		shell=shell1;
		cout<<"crcb"<<endl;

		return 0;
	}
	if(grayDetect==false&&ycrcbDetect==false)
	{
		return -1;
	}
	
		
		
		//cout<<dis1<<"  "<<dis2<<endl;
		
		
		
		imshow("temp",temp);
		return 0;
	
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
	inRange(srcYCrCb,136,255,ycrcbBin);//imshow("ycrcbb",ycrcbBin);
	//dilate(ycrcbBin,ycrcbBin,getStructuringElement(0,Size(2,2)));
	//vector<ConnectObj> ycrcbO;
	//connectedComponents(ycrcbBin,ycrcbO);//drawCon(grayO);

	split(srcYCrCb,srcCh);//tmpImg=srcYCrCb;
	
      	//Gray
	inRange(srcGray,0,31,grayBin);
	//morphologyEx(srcYCrCb,srcYCrCb,MORPH_OPEN,getStructuringElement(0,Size(5,5)));
	//imshow("grayb",grayBin);
	//vector<ConnectObj> grayO;
	//connectedComponents(grayBin,grayO);//drawCon(grayO);
	tmpImg=srcGray;

	//HSV
	Mat fullCar;
	inRange(srcHSV,Scalar(65,170,150),Scalar(86,255,255),hsvBin);
	dilate(hsvBin,hsvBin,getStructuringElement(0,Size(15,15)));
	//vector<ConnectObj> hsvO;
	//connectedComponents(hsvBin,hsvO);//drawCon(grayO);
	imshow("hsv",hsvBin);

	//combineCon(grayBin,ycrcbO);
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
	//combineCon(grayOYcrcb,fullCar,hsvO);imshow("full",fullCar);
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
	
	Mat dst1=Mat::zeros(src.size(),CV_8UC1);
	//cout<<cO.size()<<endl;
	for(int i=0;i<cO.size();i++)
	{
		int h=cO[i].bound.height;
		int w=cO[i].bound.width;
		int rowh=cO[i].bound.y+h;
		int colh=cO[i].bound.x-w/2;
		int rowe=rowh+h;
		int cole=colh+w*4;
		for(int r=rowh;r<rowe;r++)
		{
			for(int c=colh;c<cole;c++)
			{
				//cout<<c<<endl;
				dst1.at<uchar>(r,c)=src.at<uchar>(r,c);//x,y

			}
		}

	}//at(row,col)
	
	dilate(dst1,dst1,getStructuringElement(0,Size(7,7)));
	dst=dst1;
	//imshow("combine",dst);
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

