// This file is part of the Pillar Technology MDP 2014 quadcopter project
// 
// File: QR_Reader.cpp
// Language: C
// Libs: OpenCV
// Authors: A. Liao
// Date Modified: 2/17/2014

// Include the libraries needed by OpenCV and file reading
#include <stdlib.h>
#include <stdio.h>
#include <highgui.h>
#include <cv.h>
#include <zbar.h>

// The capture dimensions
const int FRAME_WIDTH  = 800;
const int FRAME_HEIGHT = 640;

// Where the camera origin is (quadcopter view)
const int MID_X = 400;
const int MID_Y = 320;

// Linear variables for the qr_length to inches (distance)
const double DISTANCE_M = -0.15;
const int DISTANCE_B = 85;

// PI for the trig calculations
const double MATH_PI = 3.1415942854;

// Window names
const char * windowName = "Camera Feed";

typedef struct
{
	double distance;
	double angle;
	double x;
	double y;
} QR_Data;

void process_QR(IplImage* img, QR_Data * data, IplImage* outimg)
{
	// Data extracted from the ZBar Image
	int width = 0;
	int height = 0;
	void *raw = NULL;

	// Data from the QR code and its position/angle
	int qr_length = 0;		// The length of the code in pixels
	double qr_distance = 0; // How far the qr code is (altitude)
	double qr_angle = 0;	// Angle of the code from the right x-axis
	double qr_angle_deg = 0;// Same as above but in degrees
	double dis2Mid = 0;		// Distance from the camera middle to code
	double theta1 = 0;		// the arctan of the y' and x' axes
	double theta2 = 0;		// the angle between the two axes
	double theta2_deg = 0;	// theta2 in radians
	double x_d = 0;
	double y_d = 0;
	double x_ab = 0;
	double y_ab = 0;
	int qr_x, qr_y;	// The data from the QR Code
	char text[80];

	// ZBar Scanner for C
	zbar_image_scanner_t* scanner = zbar_image_scanner_create();
	// configure the scanner
	zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

	// Extract data from the image
	width = img->width;
	height = img->height;
	raw = (void *) img->imageData;

	// Wrap the image data
	zbar_image_t *image = zbar_image_create();
	zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);

	// Scan the image for QR
	int n = zbar_scan_image(scanner, image);

	 /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
		// Cycle through each symbol found
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n",
			zbar_get_symbol_name(typ), data);

		sscanf(data, "%d %d", &qr_x, &qr_y);
		printf("QR_X: %i\n", qr_x);
		printf("QR_Y: %i\n", qr_y);

		// Find the angle between the lines
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC|CV_32SC2, 
				sizeof(CvContour), sizeof(CvPoint), storage);

		CvPoint pts[4];
		int i = 0;
		for (i = 0; i < 4; ++i) {
			CvPoint point = cvPoint(zbar_symbol_get_loc_x(symbol,i),
					zbar_symbol_get_loc_y(symbol,i));
			cvSeqPush(ptseq, &point);
			pts[i] = point;
		}
		CvBox2D rect = cvMinAreaRect2(ptseq, 0);
		// Draw the outline rectangle

		for (i = 0; i < 4; ++i) {
			cvLine(outimg, pts[i], pts[(i+1)%4], 
					CV_RGB(0, 0, 255), 5, 8, 0);
		}

		// Get the distance from the code to the camera
		qr_length = sqrt(abs(pts[0].x * pts[0].x - pts[1].x * pts[1].x) +
				abs(pts[0].y * pts[0].y - pts[1].y * pts[1].y));
		qr_distance = qr_length * DISTANCE_M + DISTANCE_B;
		printf("Length: %i\n", qr_length);
		printf("Distance: %f\n", qr_distance);

		// Find the relative location
		// Get the angle of the circled rectangle
		qr_angle = -rect.angle;
		if (pts[0].x > pts[3].x && pts[0].y > pts[3].y) qr_angle += 90;
		else if (pts[0].x > pts[3].x && pts[0].y < pts[3].y) qr_angle += 180;
		else if (pts[0].x < pts[3].x && pts[0].y < pts[3].y) qr_angle += 270;
		else if (pts[0].x == pts[1].x && pts[0].y == pts[3].y) {
			if (pts[0].x < pts[3].x && pts[0].y < pts[1].y)
				qr_angle = 0;
			else qr_angle = 180;
		}
		else if (pts[0].x == pts[3].x && pts[0].y == pts[1].y) {
			if (pts[0].x < pts[1].x && pts[0].y > pts[3].y) 
				qr_angle = 90;
			else qr_angle = 270;
		}
		printf("Angle: %f\n", qr_angle);

		//Draw a line on the angle
		qr_angle = qr_angle * 3.1415 / 180;
		CvPoint mid = cvPoint((pts[0].x + pts[2].x) / 2, 
				(pts[0].y + pts[2].y)/2);
		CvPoint p2 = cvPoint(mid.x + 25*cos(qr_angle), 
				mid.y - 25*sin(qr_angle));
		cvLine(outimg,mid, p2, CV_RGB(0,255,0),5,8,0);

		// Get the relative location based on the data of the QR code
		// QR format: x y
		// x and y are seperated by a single space
		// Check if the QR is in the right format
		cvLine(outimg,mid, cvPoint(MID_X,MID_Y), CV_RGB(255,0,0),5,8,0);

		// Relative position (in pixel)
		dis2Mid = sqrt((mid.x - MID_X) * (mid.x - MID_X) + 
				(mid.y - MID_Y) * (mid.y - MID_Y));
		printf("Distance to Quad: %f\n", dis2Mid);
		
		theta1 = atan2(MID_Y - mid.y, MID_X - mid.x) * 180 / MATH_PI;
		qr_angle_deg = qr_angle * 180 / MATH_PI;
		theta2_deg = 90 - theta1 - qr_angle_deg; 
		theta2 = theta2_deg * MATH_PI / 180;
		x_d = dis2Mid * sin(theta2);
		y_d = dis2Mid * cos(theta2);

		// Display message onto the image
		CvFont font;
		cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, 8);
		sprintf(text, "Attitude: %f", qr_distance);
		cvPutText(outimg, text, cvPoint(30,30), 
				&font, cvScalar(255, 255, 255, 0));
		sprintf(text, "Angle: %f", qr_angle_deg);
		cvPutText(outimg, text, cvPoint(30,50), 
				&font, cvScalar(255, 255, 255, 0));
		x_ab = x_d + qr_x;
		y_ab = y_d + qr_y;
		sprintf(text, "Abs. Pos: (%f, %f)", x_ab, y_ab);
		cvPutText(outimg, text, cvPoint(30,70), 
				&font, cvScalar(255, 255, 255, 0));
    }
}

int main(int argc, char* argv[])
{
	// Matrix for frames from the camera
	CvCapture* capture = cvCaptureFromCAM( CV_CAP_ANY );

	// Infinite loop where our scanning is down on each camera frame
	while (1) 
	{
		IplImage* frame = cvQueryFrame(capture);
		if (!frame) {
			fprintf(stderr, "ERROR: Frame is null\n");
			getchar();
			break;
		}

		IplImage* outputimg = cvCreateImage(cvGetSize(frame), frame->depth,1);
		IplImage* test = cvCreateImage(cvGetSize(frame), frame->depth,frame->nChannels);
		
		double a[9]={-1,20,1,-1,20,1,-1,20,1};
		CvMat kernel= cvMat(3,3,CV_32FC1,a);
		cvFilter2D(frame,test,&kernel,cvPoint(-1,-1));

		cvCvtColor(frame, outputimg, CV_RGB2GRAY);
		QR_Data data;
		process_QR(outputimg, &data, frame);

		cvShowImage("Camera", frame);
		cvShowImage("Sharp", test);

		// Delay so screen can refresh
		if ((char) cvWaitKey(30) == 27) break;
	}

	// Release the resources
	cvReleaseCapture(&capture);
	cvDestroyWindow("Camera");

	return 0;
}
