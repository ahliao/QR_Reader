#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv.h>
#include <iostream>
#include <zbar.h>

using namespace cv;
using namespace std;
using namespace zbar;

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
	inRange(HSV, Scalar(0, 0, 0), Scalar(10, 10, 10), threshold);

	// Get rid of noise
	Mat erodeElement = getStructuringElement( MORPH_RECT, Size(1,1));
	Mat dilateElement = getStructuringElement( MORPH_RECT, Size(3,3));

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
				if (area > 10 && area < 10000 && area > refArea) {
					x = moment.m10/area;
					y = moment.m01/area;
					objectFound = true;
					refArea = area;
				} 
			}
			if (objectFound == true) {
				cout << "Found at (" << x << ", " << y << ")" << endl;
				cout << "Area: " << refArea << endl;
				length = refArea / 5;
				cout << "Side Length: " << length << endl;
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

int main( int argc, char** argv )
{
    if( argc != 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    Mat image;
    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file
	Mat outImg = image;

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

	// TODO: put these all into seperate functions
	
	// Find where each QR code is located and do a crop of that
	// Start with creating a binary to find the black parts
	Mat threshold;
	thresholdMorph(image, threshold);
	
	// find the x and y coordinates
	int qr_x, qr_y, qr_sideLength;
	findObjects(qr_x, qr_y, qr_sideLength, threshold);

	// Crop the image to a general area around the located code
	Mat cropped;
	cropAroundObject(qr_x, qr_y, qr_sideLength, image, cropped);

	// Grayscale copy of the cropped image
	Mat grayImg;
	cvtColor(cropped, grayImg, CV_RGB2GRAY);

	int width = grayImg.cols;
	int height = grayImg.rows;
	uchar *raw = (uchar *)grayImg.data;

	Image img(width, height, "Y800", raw, width * height);

	zbar::ImageScanner scanner;
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

	int n = scanner.scan(img);
	cout << "n = " << n << endl;

	 // extract results
	for(Image::SymbolIterator symbol = img.symbol_begin();
		symbol != img.symbol_end();
		++symbol) {
		// do something useful with results
		cout << "decoded " << symbol->get_type_name()
			 << " symbol \"" << symbol->get_data() << '"' << endl;


		vector<Point> vp;
		int s = symbol->get_location_size();  
		for(int i=0;i<s;i++){  
			vp.push_back(Point(symbol->get_location_x(i),symbol->get_location_y(i))); 
		}  
		RotatedRect r = minAreaRect(vp);  
		Point2f pts[4];  
		r.points(pts);  
		for(int i=0;i<4;i++){  
			line(outImg,pts[i],pts[(i+1)%4],Scalar(255,255,0),2);  
		}  
		cout<<"Angle: "<<r.angle<<endl;  
	}

    imshow( "Test", outImg );                   // Show our image inside it.
	imshow( "Threshold", threshold);
	imshow( "Cropped" , cropped);

    while(waitKey(0) != 27);         // Wait for escape key 

	// Clean up
	img.set_data(NULL, 0);

    return 0;
}
