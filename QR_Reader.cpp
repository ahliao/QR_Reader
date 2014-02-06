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
const int FRAME_WIDTH  = 800;
const int FRAME_HEIGHT = 600;

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

	// Infinite loop where our scanning is down on each camera frame
	while (1) 
	{
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
			double length = sqrt(abs(pts[0].x * pts[0].x - pts[1].x * pts[1].x) +
					abs(pts[0].y * pts[0].y - pts[1].y * pts[1].y));
			double distance = length * DISTANCE_M + DISTANCE_B;
			cout << "Length: " << length << endl;
			cout << "Distance: " << distance << endl;
		}

		imshow(windowName, cameraFeed);

		// Delay so screen can refresh
		if ((char) waitKey(40) == 27) break;
	}
	// TODO: release the capture

	cvDestroyAllWindows();

	return 0;
}
