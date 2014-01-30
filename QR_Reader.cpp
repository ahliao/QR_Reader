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

// The capture dimensions
const int FRAME_WIDTH  = 640;
const int FRAME_HEIGHT = 480;

// Window names
const std::string windowName = "Camera Feed";

int main(int argc, char* argv[])
{
	cv::namedWindow( windowName, cv::WINDOW_NORMAL );	// Create a window

	cv::waitKey(0);	// Wait for a keystroke

	return 0;
}
