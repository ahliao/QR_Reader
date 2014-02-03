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

// The capture dimensions
const int FRAME_WIDTH  = 640;
const int FRAME_HEIGHT = 480;

// Window names
const std::string windowName = "Camera Feed";

int main(int argc, char* argv[])
{
	// Matrix for frames from the camera
	cv::Mat cameraFeed;
	// Matrix storage for the HSV image
	cv::Mat HSV;
	// Matrix storage for the binary threshold image
	cv::Mat threshold;

	// Matric for the window output
	cv::Mat outputimg;

	// Video capture object to get the camera feed
	cv::VideoCapture capture;
	
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
		cv::cvtColor(cameraFeed, outputimg, CV_RGB2GRAY);

		// Extract data from the Mat
		width = outputimg.cols;
		height = outputimg.rows;
		raw = (uchar *) outputimg.data;

		zbar::Image img(width, height, "Y800", raw, width * height);
		zbar::ImageScanner scanner;
		scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);

		int n = scanner.scan(img);
		std::cout << "num of codes = " << n << std::endl;
		std::cout << "width = " << width << std::endl;

		for (zbar::Image::SymbolIterator symbol = img.symbol_begin();
				symbol != img.symbol_end();
				++symbol) {
			std::cout << "decoded " << symbol->get_type_name()
				 << " symbol \"" << symbol->get_data() << '"' << std::endl;

			std::vector<cv::Point> vp;
			int s = symbol->get_location_size();
			for (int i = 0; i < s; ++i) {
				vp.push_back(cv::Point(symbol->get_location_x(i),
							symbol->get_location_y(i)));
			}
			cv::RotatedRect r = minAreaRect(vp);
			cv::Point2f pts[4];
			r.points(pts);
			for (int i = 0; i < 4; ++i) {
				cv::line(cameraFeed, pts[i], pts[(i+1)%4], 
						cv::Scalar(255, 255, 0), 5);
			}
			
		}

		cv::imshow(windowName, cameraFeed);

		// Delay so screen can refresh
		if ((char) cv::waitKey(40) == 27) break;
	}

	cvDestroyAllWindows();

	return 0;
}
