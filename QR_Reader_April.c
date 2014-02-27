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

// Include the Libraries for AprilTags
#include "apriltags/apriltag.h"
#include "apriltags/image_u8.h"
#include "apriltags/tag36h11.h"
#include "apriltags/zarray.h"
//#include "apriltags/homography.h"

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

	// Instantiate a tag family and pass to detector

	april_tag_family_t *tf = tag36h11_create();
	april_tag_detector_t *td = april_tag_detector_create(tf);
	td->nthreads = 2;

	// Form an image_u8 with the image
	image_u8_t *im = image_u8_create_from_rgb3(img->width, img->height, 
		(uint8_t *)img->imageData, img->widthStep);
	//image_u8_t *im = image_u8_create_from_pnm("testtag.pnm");
	zarray_t * detections = april_tag_detector_detect(td, im);

	int i = 0;
	for (i = 0; i < zarray_size(detections); ++i) {
		april_tag_detection_t *det;
		zarray_get(detections, i, &det);

		printf("%i code\n", i);
		printf("detection %3d: id %4d, hamming %d, goodness %f\n", i,
				det->id, det->hamming, det->goodness);

		//matd_t *M = homography_to_pose(det->H, img->width,
		//		img->height, det->c[0], det->c[1]);
		

		// get the four corners
		// Find the angle between the lines
		CvMemStorage* storage = cvCreateMemStorage(0);
		CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC|CV_32SC2, 
				sizeof(CvContour), sizeof(CvPoint), storage);

		CvPoint pts[4];
		double ptnx;
		double ptny;
		int a = 0;
		for (a = 0; a < 4; ++a) {
		    ptnx = det->p[a][0];
			ptny = det->p[a][1];
			printf("%f, %f\n", ptnx, ptny);
			CvPoint point = cvPoint(ptnx, ptny);
			pts[a] = point;
			//cvLine(outimg, point, center,
			//		CV_RGB(0,0,255), 5, 8,0);
			cvSeqPush(ptseq, &point);
		}
		CvBox2D rect = cvMinAreaRect2(ptseq, 0);

		// Draw the outline rectangle
		for (a = 0; a < 4; ++a) {
			cvLine(outimg, pts[a], pts[(a+1)%4], 
					CV_RGB(0, 0, 255), 5, 8, 0);
		}

		// Get the distance from the code to the camera
		qr_length = sqrt((pts[0].x - pts[1].x) * (pts[0].x - pts[1].x) +
				(pts[0].y - pts[1].y) * (pts[0].y - pts[1].y));
		qr_distance = qr_length * DISTANCE_M + DISTANCE_B;
		printf("Length: %i\n", qr_length);
		printf("Distance: %f\n", qr_distance);

		// Get the angle of the square/rectangle
		for(int b = 0; b < 4; ++b)
			printf("%i, %i\n", pts[b].x, pts[b].y);
		qr_angle = -rect.angle;
		printf("BAngle: %f\n", qr_angle);
		if (pts[0].x > pts[1].x && pts[0].y > pts[3].y &&
				pts[0].y > pts[2].y && pts[0].x < pts[3].x) qr_angle += 90;
		else if (pts[0].x > pts[1].x && pts[0].y < pts[1].y &&
				pts[0].y > pts[3].y && pts[0].x > pts[2].x) qr_angle += 180;
		else if (pts[0].x < pts[1].x && pts[0].x > pts[3].x &&
				pts[0].y < pts[2].y && pts[0].y < pts[3].y) qr_angle += 270;
		else if (pts[0].x == pts[3].x && pts[0].y == pts[1].y &&
				pts[0].x < pts[1].x && pts[0].y < pts[3].y) qr_angle = 0;
		else if (pts[0].x == pts[1].x && pts[0].y == pts[3].y &&
				pts[0].x < pts[3].x && pts[0].y > pts[1].y) qr_angle = 90;
		else if (pts[0].x == pts[3].x && pts[0].y == pts[1].y &&
				pts[0].x > pts[1].x && pts[0].y > pts[3].y) qr_angle = 180;
		else if (pts[0].x == pts[1].x && pts[0].y == pts[3].y &&
				pts[0].x > pts[3].x && pts[0].y < pts[1].y) qr_angle = 270;
		printf("Angle: %f\n", qr_angle);

		// Draw a line for the angle
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
				&font, cvScalar(0, 5, 55, 0));
		sprintf(text, "Angle: %f", qr_angle_deg);
		cvPutText(outimg, text, cvPoint(30,60), 
				&font, cvScalar(0, 5, 55, 0));
		x_ab = x_d + qr_x;
		y_ab = y_d + qr_y;
		sprintf(text, "Abs. Pos: (%f, %f)", x_ab, y_ab);
		cvPutText(outimg, text, cvPoint(30,90), 
				&font, cvScalar(0, 55, 55, 0));

		// TODO: put the data into the QR_Data struct

		april_tag_detection_destroy(det);
	}

	zarray_destroy(detections);

    april_tag_detector_destroy(td);

    tag36h11_destroy(tf);
}

int main(int argc, char* argv[])
{
	// Matrix for frames from the camera
	CvCapture* capture = cvCaptureFromCAM( CV_CAP_ANY );

	// Infinite loop where our scanning is down on each camera frame
	while (1) 
	{
		//IplImage* frame = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);//cvQueryFrame(capture);
		IplImage* frame = cvQueryFrame(capture);
		if (!frame) {
			fprintf(stderr, "ERROR: Frame is null\n");
			//getchar();
			//break;
		}

		IplImage* inputimg = cvCreateImage(cvGetSize(frame), frame->depth,3);

		cvCvtColor(frame, inputimg, CV_RGB2BGR);
		QR_Data data;
		process_QR(inputimg, &data, frame);

		cvShowImage("Camera", frame);

		// Delay so screen can refresh
		//while (!((char) cvWaitKey(30) == 27));
		if((char) cvWaitKey(1) == 27) break;
	}

	// Release the resources
	cvReleaseCapture(&capture);
	cvDestroyWindow("Camera");

	return 0;
}
