// This file is part of the Pillar Technology MDP 2014 quadcopter project
// 
// File: QR_Reader.cpp
// Language: C++
// Libs: OpenCV
// Authors: A. LaBerge, A. Liao
// Date Modified: 1/30/2014

// Include the libraries needed by OpenCV and file reading
#include <iostream>
#include <string>
#include <highgui.h>
#include <cv.h>
#include <zbar.h>

using namespace cv;
using namespace zbar;
using namespace std;

// The capture dimensions
const int FRAME_WIDTH  = 640;
const int FRAME_HEIGHT = 640;

// Linear variables for the qr_length to inches (distance)
const double DISTANCE_M = -0.15;
const int DISTANCE_B = 85;

// Window names
const string windowName = "Camera Feed";

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

	// Data extracted from the ZBar Image
	int width = 0;
	int height = 0;
	uchar *raw = 0;

	// Data from the QR code and its position/angle
	int qr_length = 0;
	double qr_distance = 0;
	double qr_angle = 0;

	// DEBUG Timing Test
	double debug_t = 0;

	// Infinite loop where our scanning is down on each camera frame
	while (1) 
	{
		debug_t = (double) getTickCount();

		// store image to our matrix
		capture.read(cameraFeed);
		cvtColor(cameraFeed, outputimg, CV_RGB2GRAY);

		// Extract data from the Mat
		width = outputimg.cols;
		height = outputimg.rows;
		raw = (uchar *) outputimg.data;

		Image img(width, height, "Y800", raw, width * height);
		ImageScanner scanner;
		scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

		int n = scanner.scan(img);
		cout << "num of codes = " << n << endl;
		cout << "width = " << width << endl;

		for (Image::SymbolIterator symbol = img.symbol_begin();
				symbol != img.symbol_end();
				++symbol) {

			// TODO: Handle multiple symbols

			cout << "decoded " << symbol->get_type_name()
				 << " symbol \"" << symbol->get_data() << '"' << endl;

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
				line(cameraFeed, pts[i], pts[(i+1)%4], 
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
				cout << "Blah1" << endl;
			}
			else if (vp[0].x == vp[3].x && vp[0].y == vp[1].y) {
				if (vp[0].x < vp[1].x && vp[0].y > vp[3].y) 
					qr_angle = 90;
				else qr_angle = 270;
				cout << "Blah2" << endl;
			}
			cout<< "Angle: " << qr_angle <<endl;  
			// There is a bug at the 90n angles

			//Draw a line on the angle
			qr_angle = qr_angle * 3.1415 / 180;
			Point mid((pts[0].x + pts[2].x) / 2, (pts[0].y + pts[2].y)/2);
			Point p2(mid.x + 25*cos(qr_angle), mid.y - 25*sin(qr_angle));
			line(cameraFeed,mid, p2, Scalar(0,255,0),5);

			// Get the relative location based on the data of the QR code
			// QR format: x y
			// x and y are seperated by a single space
			// Check if the QR is in the right format

			// Assume the QR reads 0 0 for now
			int x = FRAME_WIDTH / 2;
			int y = FRAME_HEIGHT / 2;
			line(cameraFeed,mid, Point(x,y), Scalar(255,0,0),5);

			// Relative position (in pixel)
			double dis2Mid = sqrt((mid.x - x) * (mid.x - x) + 
					(mid.y - y) * (mid.y - y));
			cout << "Distance to Quad: " << dis2Mid << endl;
			
			double theta1 = atan2(y - mid.y, x - mid.x) * 180/3.1415;
			double qr_angle_deg = qr_angle * 180/3.1415;
			double theta2 = 90 - theta1 - qr_angle_deg;
			cout << "qr_angle: " << qr_angle_deg << endl;
			cout << "Theta1: " << theta1 << endl;
			cout << "Theta2: " << theta2 << endl;
			double theta2_rad = theta2 * 3.1415 / 180;
			double x_d = dis2Mid * sin(theta2_rad);
			double y_d = dis2Mid * cos(theta2_rad);
			cout << "X dis: " << x_d << endl;
			cout << "Y dis: " << y_d << endl;

			// Find the seconds it took to process
			debug_t = ((double) getTickCount() - debug_t) / getTickFrequency();
			cout << "Process time (s): " << debug_t << endl;
		}

		imshow(windowName, cameraFeed);

		// Delay so screen can refresh
		if ((char) waitKey(40) == 27) break;
	}

	// Release the resources
	cvDestroyAllWindows();
	capture.release();

	return 0;
}
