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

	// Video capture object to get the camera feed
	cv::VideoCapture capture;
	
	// Open the capture object (0 = webcam)
	capture.open(0);

	// Set the dimensions of the capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	// Infinite loop where our scanning is down on each camera frame
	while (1) 
	{
		// store image to our matrix
		capture.read(cameraFeed);

		cv::imshow(windowName, cameraFeed);

		// Delay so screen can refresh
		if ((char) cv::waitKey(30) == 27) break;
	}

	cvDestroyAllWindows();

	return 0;
}
