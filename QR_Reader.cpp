// This file is part of the Pillar Technology MDP 2014 quadcopter project
// 
// File: QR_Reader.cpp
// Language: C++
// Libs: OpenCV
// Authors: A. LaBerge, A. Liao
// Date Modified: 2/17/2014

// Include the libraries needed by OpenCV and file reading
#include <iostream>
#include <cstdio>
#include <string>
#include <highgui.h>
#include <cv.h>
#include <zbar.h>

using namespace cv;
using namespace zbar;
using namespace std;

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
const string windowName = "Camera Feed";

struct QR_Data
{
	double distance;
	double angle;
	double x;
	double y;
};

void process_QR(Mat& img, QR_Data& data, Mat& outimg)
{
	// Data extracted from the ZBar Image
	int width = 0;
	int height = 0;
	uchar *raw = 0;

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

	// Extract data from the Mat
	width = img.cols;
	height = img.rows;
	raw = (uchar *) img.data;

	// ZBar scanner
	Image z_img(width, height, "Y800", raw, width * height);
	ImageScanner scanner;
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

	int n = scanner.scan(z_img);
	cout << "num of codes = " << n << endl;

	// Loop through all the symbols found in the image
	for (Image::SymbolIterator symbol = z_img.symbol_begin();
			symbol != z_img.symbol_end();
			++symbol) {

		// TODO: Handle multiple symbols

		cout << "decoded " << symbol->get_type_name()
			 << " symbol \"" << symbol->get_data() << '"' << endl;

		int qr_x, qr_y;	// The data from the QR Code
		sscanf(symbol->get_data().c_str(), "%d %d", &qr_x, &qr_y);
		cout << "QR_X: " << qr_x << endl;
		cout << "QR_Y: " << qr_y << endl;

		// Draw the surrounding rectangle
		vector<Point> vp;
		int s = symbol->get_location_size();
		for (int i = 0; i < s; ++i) {
			vp.push_back(Point(symbol->get_location_x(i),
						symbol->get_location_y(i)));
		}
		RotatedRect r = minAreaRect(vp);
		Point2f pts[4];
		r.points(pts);
		for (int i = 0; i < 4; ++i) {
			line(outimg, pts[i], pts[(i+1)%4], 
					Scalar(255, 255, 0), 5);
		}
		
		// Get the distance from the code to the camera
		qr_length = sqrt(abs(pts[0].x * pts[0].x - pts[1].x * pts[1].x) +
				abs(pts[0].y * pts[0].y - pts[1].y * pts[1].y));
		qr_distance = qr_length * DISTANCE_M + DISTANCE_B;
		cout << "Length: " << qr_length << endl;
		cout << "Distance: " << qr_distance << endl;

		// Find the relative location
		// Get the angle of the circled rectangle
		qr_angle = -r.angle;
		if (vp[0].x > vp[3].x && vp[0].y > vp[3].y) qr_angle += 90;
		else if (vp[0].x > vp[3].x && vp[0].y < vp[3].y) qr_angle += 180;
		else if (vp[0].x < vp[3].x && vp[0].y < vp[3].y) qr_angle += 270;
		else if (vp[0].x == vp[1].x && vp[0].y == vp[3].y) {
			if (vp[0].x < vp[3].x && vp[0].y < vp[1].y)
				qr_angle = 0;
			else qr_angle = 180;
		}
		else if (vp[0].x == vp[3].x && vp[0].y == vp[1].y) {
			if (vp[0].x < vp[1].x && vp[0].y > vp[3].y) 
				qr_angle = 90;
			else qr_angle = 270;
		}
		cout<< "Angle: " << qr_angle <<endl;  
		// There is a bug at the 90n angles

		//Draw a line on the angle
		qr_angle = qr_angle * 3.1415 / 180;
		Point mid((pts[0].x + pts[2].x) / 2, (pts[0].y + pts[2].y)/2);
		Point p2(mid.x + 25*cos(qr_angle), mid.y - 25*sin(qr_angle));
		line(outimg,mid, p2, Scalar(0,255,0),5);

		// Get the relative location based on the data of the QR code
		// QR format: x y
		// x and y are seperated by a single space
		// Check if the QR is in the right format

		// Assume the QR reads 0 0 for now
		line(outimg,mid, Point(MID_X,MID_Y), Scalar(255,0,0),5);

		// Relative position (in pixel)
		dis2Mid = sqrt((mid.x - MID_X) * (mid.x - MID_X) + 
				(mid.y - MID_Y) * (mid.y - MID_Y));
		cout << "Distance to Quad: " << dis2Mid << endl;
		
		theta1 = atan2(MID_Y - mid.y, MID_X - mid.x) * 180 / MATH_PI;
		qr_angle_deg = qr_angle * 180 / MATH_PI;
		theta2_deg = 90 - theta1 - qr_angle_deg;
		theta2 = theta2_deg * MATH_PI / 180;
		x_d = dis2Mid * sin(theta2);
		y_d = dis2Mid * cos(theta2);

		// TODO: put this into a function
		// Display messages onto the window
		stringstream strstream;
		strstream << "Attitude: " << qr_distance;
		string x = strstream.str();
		strstream.str(string());
		putText(outimg, x, cvPoint(30,30), 
				FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 
				1, CV_AA);

		strstream << "Angle: " << qr_angle_deg;
		x = strstream.str();
		strstream.str(string());
		putText(outimg, x, cvPoint(30,50), 
				FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 
				1, CV_AA);

		strstream << "Rel. Pos: " << "(" << x_d << ", " << y_d << ")";
		x = strstream.str();
		strstream.str(string());
		putText(outimg, x, cvPoint(30,70), 
				FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 
				1, CV_AA);

		// Display where you are in the grid (absolute position)
		double x_ab = x_d + qr_x;
		double y_ab = y_d + qr_y;
		strstream << "Abs. Pos: " << "(" << x_ab << ", " << y_ab << ")";
		x = strstream.str();
		strstream.str(string());
		putText(outimg, x, cvPoint(30,90), 
				FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 
				1, CV_AA);
	}
}

int main(int argc, char* argv[])
{
	// Matrix for frames from the camera
	Mat cameraFeed;

	// Matric for the window output
	Mat outputimg;

	// Video capture object to get the camera feed
	VideoCapture capture;
	
	// Open the capture object (0 = webcam)
	capture.open(0);

	// Set the dimensions of the capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	// DEBUG Timing Test
	double debug_t = 0;

	// Infinite loop where our scanning is down on each camera frame
	while (1) 
	{
		debug_t = (double) getTickCount();

		// store image to our matrix
		capture.read(cameraFeed);
		cvtColor(cameraFeed, outputimg, CV_RGB2GRAY);
		QR_Data data;
		process_QR(outputimg, data, cameraFeed);

		debug_t = ((double) getTickCount() - debug_t) / getTickFrequency();
		cout << "Process time (s): " << debug_t << endl;

		imshow(windowName, cameraFeed);

		// Delay so screen can refresh
		if ((char) waitKey(30) == 27) break;
	}

	// Release the resources
	cvDestroyAllWindows();
	capture.release();

	return 0;
}
