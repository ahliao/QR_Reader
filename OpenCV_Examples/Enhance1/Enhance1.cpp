#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv.h>
#include <iostream>
#include <zbar.h>

using namespace cv;
using namespace std;
using namespace zbar;

int main( int argc, char** argv )
{
    if( argc != 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    Mat image;
    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file
	Mat outimg = image;
	//cvtColor(image, outimg, CV_GRAY2RGB);

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

	// Do the enhancements on the input image

	// TODO: put these all into seperate functions
	
	// Find where each QR code is located and do a crop of that
	Mat enhancedimg;
	// Start with creating a binary to find the black parts
	Mat enhancedHSV;
	Mat threshold;
	// convert to get HSV
	cvtColor(image, enhancedHSV, CV_RGB2HSV);
	inRange(enhancedHSV, Scalar(0, 0, 0), Scalar(10, 10, 10), threshold);

	// Get rid of noise
	Mat erodeElement = getStructuringElement( MORPH_RECT, Size(1,1));
	Mat dilateElement = getStructuringElement( MORPH_RECT, Size(3,3));

	// erode away any small noise
	erode(threshold, threshold, erodeElement);
	
	// dilate to enlarge the wanted parts
	dilate(threshold, threshold, dilateElement);

	// find the x and y coordinates
	// find the moment
	Mat temp;
	threshold.copyTo(temp);
	int qr_x, qr_y, qr_width, qr_height;
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
					qr_x = moment.m10/area;
					qr_y = moment.m01/area;
					objectFound = true;
					refArea = area;
				} 
			}
			if (objectFound == true) {
				cout << "Found at (" << qr_x << ", " << qr_y << ")" << endl;
				cout << "Area: " << refArea << endl;
				// Saying that area/5 is about the width atm
				cout << "Width: " << refArea/5 << endl;
			} else cout << "not found" << endl;
		}
	}

	// Crop the image to a general area around the located code



	// Let's sharpen the image
	Mat kern = (Mat_<char>(3, 3) << 0, -1,  0,
								   -1,  5, -1,
								    0, -1,  0);
	filter2D(image, enhancedimg, image.depth(), kern);

	int width = image.cols;
	int height = image.rows;
	uchar *raw = (uchar *)image.data;

	Image img(width, height, "Y800", raw, width * height);

	zbar::ImageScanner scanner;
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

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
			line(outimg,pts[i],pts[(i+1)%4],Scalar(255,255,0),2);  
		}  
		cout<<"Angle: "<<r.angle<<endl;  
	}

    imshow( "Test", outimg );                   // Show our image inside it.
	imshow( "Threshold", threshold);
	imshow( "Enhanced", enhancedHSV);

    waitKey(0);                          // Wait for a keystroke in the window

	// Clean up
	img.set_data(NULL, 0);

    return 0;
}
