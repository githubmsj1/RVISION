#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case CV_EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;
        break;
    }
}

static void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
            "You select a color objects such as your face and it tracks it.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   ./camshiftdemo [camera number]\n";

    cout << "\n\nHot keys: \n"
            "\tESC - quit the program\n"
            "\tc - stop the tracking\n"
            "\tb - switch to/from backprojection view\n"
            "\th - show/hide object histogram\n"
            "\tp - pause video\n"
            "To initialize tracking, select the object with mouse\n";
}

const char* keys =
{
    "{1|  | 0 | camera number}"
};

int main( int argc, const char** argv )
{
    help();

    VideoCapture cap;
    Rect trackWindow;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;
    CommandLineParser parser(argc, argv, keys);
    //int camNum = parser.get<int>("2");

    cap.open(1);//"v2.mp4"

    if( !cap.isOpened() )
    {
        help();
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
        parser.printParams();
        return -1;
    }

    namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    setMouseCallback( "CamShift Demo", onMouse, 0 );
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
	Mat chD1,chD2,chD3;
	Mat hist1,hist2,hist3;
	Mat backproj1,backproj2,backproj3;
    bool paused = false;
	cap >> frame;
    for(;;)
    {
        if( !paused )
        {
            cap >> frame;
            if( frame.empty() )
                break;
        }
	
	Mat image1;
        frame.copyTo(image);
	

	Rect searchRange;
	if(selection.width!=0&&selection.height!=0)
	{
		if(trackWindow.width==0||trackWindow.height==0)
		{
			trackWindow=selection;
		}
		int sca=2;
		searchRange=trackWindow;
		searchRange.height=searchRange.height*sca;
		searchRange.width=searchRange.width*sca;
		searchRange.x-=searchRange.width/4;
		searchRange.y-=searchRange.height/4;
		searchRange.x=MAX(searchRange.x,0);
		searchRange.y=MAX(searchRange.y,0);
		//cout<<searchRange.x<<endl;

		if(image.cols-searchRange.x<searchRange.width)
		{
			searchRange.x=image.cols-searchRange.width;
		}
		if(image.rows-searchRange.y<searchRange.height)
		{
			searchRange.y=image.rows-searchRange.height;
		}

		//image1=Mat(image,searchRange);
	}
	else
	{
		//image1.copyTo(image);
	}


		
        //Mat image(image1, searchRange);//,maskroi(mask, selection);
	if( !paused )
        {
            //cvtColor(image, hsv, COLOR_BGR2HSV);

            if( trackObject )
            {
                int _vmin = vmin, _vmax = vmax;

                //inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),Scalar(180, 256, MAX(_vmin, _vmax)), mask);
                int ch1[] = {0, 0};
		int ch2[] = {1, 0};
		int ch3[] = {2, 0};
                //hue.create(hsv.size(), hsv.depth());

		chD1.create(image.size(), image.depth());
		chD2.create(image.size(), image.depth());
		chD3.create(image.size(), image.depth());
		mask = Mat::ones(image.size(), CV_8U);

                mixChannels(&image, 1, &chD1, 1, ch1, 1);
				mixChannels(&image, 1, &chD2, 1, ch2, 1);
				mixChannels(&image, 1, &chD3, 1, ch3, 1);

		if( trackObject < 0 )
                {
					Mat maskroi(mask, selection);
                    Mat roi1(chD1, selection);//,maskroi(mask, selection);
					Mat roi2(chD2, selection);
					Mat roi3(chD3, selection);

					
					//calcHist(&roi3, 1, 0, maskroi, hist3, 1, &hsize, &phranges,true,false);
					calcHist(&roi3, 1, 0, maskroi, hist3, 1, &hsize, &phranges);
					calcHist(&roi2, 1, 0, maskroi, hist2, 1, &hsize, &phranges);
					calcHist(&roi1, 1, 0, maskroi, hist1, 1, &hsize, &phranges);
					

					//cout<<hist.size()<<endl;
                    normalize(hist1, hist1, 0, 255, CV_MINMAX);
					normalize(hist2, hist2, 0, 255, CV_MINMAX);
					normalize(hist3, hist3, 0, 255, CV_MINMAX);
				
                    trackWindow = selection;
                    trackObject = 1;

					//hist draw
                    histimg = Scalar::all(0);
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);
                    for( int i = 0; i < hsize; i++ )
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf, buf, CV_HSV2BGR);

					for( int i = 0; i < hsize; i++ )
                    {
                        int val = saturate_cast<int>(hist1.at<float>(i)*histimg.rows/255);
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                   Point((i+1)*binW,histimg.rows - val),
                                   Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }

                		calcBackProject(&chD1, 1, 0, hist1, backproj1, &phranges);
				calcBackProject(&chD2, 1, 0, hist2, backproj2, &phranges);
				calcBackProject(&chD3, 1, 0, hist3, backproj3, &phranges);

				imshow("back1",backproj1);
				imshow("back2",backproj2);
				imshow("back3",backproj3);
				
               // backproj &= mask; 
				//imshow("back1",backproj1);
                //RotatedRect trackBox = 
				backproj1.copyTo(backproj);
				for(int r=0;r<backproj1.rows;r++)
				{
					for(int c=0;c<backproj1.cols;c++)
					{
						backproj.at<uchar>(r,c)=MIN(backproj.at<uchar>(r,c),backproj2.at<uchar>(r,c));
						backproj.at<uchar>(r,c)=MIN(backproj.at<uchar>(r,c),backproj3.at<uchar>(r,c));
					}
				}
				//backproj=(backproj1+backproj2+backproj3)/3;
				imshow("bcakproj",backproj);
				

				//backproj=Mat(backproj,searchRange);
				Mat regiontrack=Mat(backproj,searchRange);
				

				Rect trackWindow1=trackWindow;

				trackWindow1.x=regiontrack.cols/2-trackWindow1.width/2;
				trackWindow1.y=regiontrack.rows/2-trackWindow.height/2;
				//meanShift(backproj1, trackWindow,TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
				meanShift(regiontrack, trackWindow1,TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
                		rectangle(regiontrack,Point(trackWindow1.x,trackWindow1.y),Point(trackWindow1.x+trackWindow1.width,trackWindow1.y+trackWindow1.height),Scalar(0,0,255),2,CV_AA);
				imshow("region",regiontrack);
                		trackWindow.x=trackWindow1.x+searchRange.x;
				trackWindow.y=trackWindow1.y+searchRange.y;

		if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);
                }

                if( backprojMode )
                    cvtColor( backproj, image, COLOR_GRAY2BGR );
                rectangle(image,Point(trackWindow.x,trackWindow.y),Point(trackWindow.x+trackWindow.width,trackWindow.y+trackWindow.height),Scalar(0,0,255),2,CV_AA);
				//ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
            }
        }
        else if( trackObject < 0 )
            paused = false;

        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }

        imshow( "CamShift Demo", image );
        imshow( "Histogram", histimg );

        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch(c)
        {
        case 'b':
            backprojMode = !backprojMode;
            break;
        case 'c':
            trackObject = 0;
            histimg = Scalar::all(0);
            break;
        case 'h':
            showHist = !showHist;
            if( !showHist )
                destroyWindow( "Histogram" );
            else
                namedWindow( "Histogram", 1 );
            break;
        case 'p':
            paused = !paused;
            break;
        default:
            ;
        }
    //waitKey(-1);

    }

    return 0;
}
