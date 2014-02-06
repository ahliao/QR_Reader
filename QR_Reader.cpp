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
#include <cmath>
#include <highgui.h>
#include <cv.h>
#include <zbar.h>

// This is probably bad practice
using namespace cv;
using namespace zbar;
using namespace std;

// The capture dimensions
const int FRAME_WIDTH  = 1000;
const int FRAME_HEIGHT = 1000;

// Window names
const std::string windowName = "Camera Feed";
// REQUIRES: inputImg is a Mat in RGB
// MODIFIES: thres becomes a binary of the inputImg 
//			 with threshold around the QR codes
void thresholdMorph(Mat &inputImg, Mat &threshold);

// REQUIRES: threshold is a binary Mat
// MODIFIES: stores the position and length in
//			 x, y, and length
void findObjects(int &x, int &y, int &length, const Mat &threshold);

// REQUIRES: 
// MODIFIES: crop becomes the cropped image from "image"
void cropAroundObject(const int &x, const int &y, const int &length, 
		const Mat &image, Mat &crop);

void thresholdMorph(Mat &inputImg, Mat &threshold) 
{
	// TODO: add an assert to make sure inputImg is RGB

	// Start with creating a binary to find the black parts
	Mat HSV;
	// convert to get HSV
	cvtColor(inputImg, HSV, CV_RGB2HSV);
	// TODO: Make the range const so easier to change
	inRange(HSV, Scalar(41, 49, 0), Scalar(56, 85, 63), threshold);
	//inRange(HSV, Scalar(29, 0, 97), Scalar(180, 256, 256), threshold);

	// Get rid of noise
	Mat erodeElement = getStructuringElement( MORPH_RECT, Size(1,1));
	Mat dilateElement = getStructuringElement( MORPH_RECT, Size(15,15));

	// erode away any small noise
	erode(threshold, threshold, erodeElement);
	
	// dilate to enlarge the wanted parts
	dilate(threshold, threshold, dilateElement);

}

void findObjects(int &x, int &y, int &length, const Mat &threshold)
{
	// find the x and y coordinates
	// find the moment
	Mat temp;	// needed as findContours modifies the input Mat
	threshold.copyTo(temp);
	double area;

	// needed for the findContours()
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	// find contours of filtered image
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	// use those moments to find the filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
		// if we have too many objects something is wrong
		// TODO: check and handle multiple QR codes
		if (numObjects < 500) {
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {
				Moments moment = moments((cv::Mat) contours[index]);
				area = moment.m00;
	
				// We need to tune these
				if (area > 10 && area < 600 && area > refArea) {
					x = moment.m10/area;
					y = moment.m01/area;
					objectFound = true;
					refArea = area;
				} 
			}
			if (objectFound == true) {
				cout << "Found at (" << x << ", " << y << ")" << endl;
				cout << "Area: " << refArea << endl;
				length = sqrt(area);
				//cout << "Side Length: " << length << endl;
				//cout << "Distance: " << length * 23.5 << endl;
			} else cout << "not found" << endl;
		}
	}
}

void cropAroundObject(const int &x, const int &y, const int &length, 
		const Mat &image, Mat &crop) 
{
	// Crop the image to a general area around the located code
	// Do some checking of our cropping rect
	int crop_x, crop_y, crop_width, crop_height;
	crop_x = x - length / 2;
	crop_y = y - length / 2;
	crop_width = length;
	crop_height = length;
	if (crop_x < 1) crop_x = 1;
	if (crop_x + crop_width >= image.cols) crop_width = image.cols - crop_x - 1;
	if (crop_y < 1) crop_y = 1;
	if (crop_y + crop_height >= image.rows) crop_height = image.rows - crop_y - 1;
	// Our region of interest
	Rect cropRect(crop_x, crop_y, crop_width, crop_height);
	// Crop the part we want
	crop = image(cropRect);
	
	
}

int main(int argc, char* argv[])
{
	// Matrix for frames from the camera
	Mat cameraFeed;

	// Matrices for object tracking/cropping
	Mat threshold;
	Mat cropped;

	// Video capture object to get the camera feed
	VideoCapture capture;
	
	// Open the capture object (0 = webcam)
	capture.open(0);

	// Set the dimensions of the capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	// Try sharpening image

	// location data of the QR codes

	// Data extracted from the ZBar Image
	int width = 0;
	int height = 0;
	uchar *raw = 0;

	// Infinite loop where our scanning is down on each camera frame
	while (1) 
	{
		double t = (double) getTickCount();
		// store image to our matrix
		capture.read(cameraFeed);
		Mat kern = (Mat_<char>(3,3) <<  0, -1,  0,
									   -1,  5, -1,
										0, -1,  0);
		//filter2D(cameraFeed, cameraFeed, cameraFeed.depth(), kern );

		Mat test;
		cvtColor(cameraFeed, test, CV_RGB2GRAY);

		// Get the binary image
		thresholdMorph(cameraFeed, threshold);

		// Find the location of the QR codes
		int qr_x, qr_y, qr_sideLength;
		findObjects(qr_x, qr_y, qr_sideLength, threshold);

		// Crop the image to a general area around the QR
		cropAroundObject(qr_x, qr_y, qr_sideLength, cameraFeed, cropped);

		// Convert the cropped image to grayscale
		cvtColor(cropped, cropped, CV_RGB2GRAY);
		//Mat erodeElement = getStructuringElement( MORPH_RECT, Size(1,1));

		// erode away any small noise
		//erode(cropped, cropped, erodeElement);
		//filter2D(cropped, cropped, cameraFeed.depth(), kern );
		//inRange(cropped, 90, 255, cropped);

		//filter2D(test, test, cameraFeed.depth(), kern );
		//inRange(test, 90, 255, test);
		// Extract data from the Mat
		width = cropped.cols;
		height = cropped.rows;
		raw = (uchar *) cropped.data;

		width = test.cols;
		height = test.rows;
		raw = (uchar *) test.data;
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
		t = ((double) getTickCount() - t) / getTickFrequency();
		cout << "Time: " << t << endl;

		//imshow("Threshold", threshold);
		imshow("Cropped", cropped);
		imshow("Camera", cameraFeed);
		//imshow("Test", test);

		// Delay so screen can refresh
		waitKey(40);
		//if ((char) waitKey(0) == 27) break;
	}

	cvDestroyAllWindows();

	return 0;
}
