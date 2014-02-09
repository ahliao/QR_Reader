#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cv.h>
#include <iostream>
#include <zbar.h>

using namespace cv;
using namespace std;
using namespace zbar;

// Linear variables for the qr_length to inches (distance)
const double DISTANCE_M = -0.15;
const int DISTANCE_B = 85;

int main( int argc, char** argv )
{
    if( argc != 2)
    {
     cout <<" Usage: display_image ImageToLoadAndDisplay" << endl;
     return -1;
    }

    Mat image;
    image = imread(argv[1], 0);   // Read the file
	Mat image2;
	cvtColor(image, image2, CV_GRAY2RGB);

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

	// Data from the QR code and its position/angle
	int qr_length = 0;
	double qr_distance = 0;
	double qr_angle = 0;

	// DEBUG Timing Test
	double debug_t = (double) getTickCount();

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
		for(int i=0;i<s;++i){  
			vp.push_back(Point(symbol->get_location_x(i),symbol->get_location_y(i))); 
		}  
		for (int i = 0; i < 4; ++i) 
			cout << vp[i] << endl;

		RotatedRect r = minAreaRect(vp);  
		Point2f pts[4];  
		r.points(pts);  
		for(int i=0;i<4;++i){  
			line(image2,pts[i],pts[(i+1)%4],Scalar(255,255,0),5);  
		}  

		// Get the distance from the code to the camera
		qr_length = sqrt(abs(pts[0].x * pts[0].x - pts[1].x * pts[1].x) +
				abs(pts[0].y * pts[0].y - pts[1].y * pts[1].y));
		qr_distance = qr_length * DISTANCE_M + DISTANCE_B;
		cout << "Length: " << qr_length << endl;
		cout << "Distance: " << qr_distance << endl;

		// Get the angle of the circled rectangle
		if (vp[0].x < vp[2].x) {
			qr_angle = -r.angle;
			if (vp[0].y > vp[2].y) qr_angle += 90;
			else qr_angle += 270;
		} 
		else {
			qr_angle = -r.angle + 90;
			if (vp[0].x > vp[2].x) qr_angle += 90;
		}
		cout<< "Angle: " << qr_angle <<endl;  
		cout << "rAngle: " << r.angle<< endl;

		// Find the real angle
		// Check the four corners for lines
		if ((int)(image.at<uchar>(pts[0].x - 1, pts[0].y - 1)) <= 50) 
			cout << "Bottom-right is a hit" << endl;
		if ((int)(image.at<uchar>(pts[1].x + 3, pts[1].y - 2)) <= 50) 
			cout << "Bottom-left is a hit" << endl;
		if ((int)(image.at<uchar>(pts[2].x + 1, pts[2].y + 1)) <= 50) 
			cout << "Top-left is a hit" << endl;
		if ((int)(image.at<uchar>(pts[3].x - 1, pts[3].y + 1)) <= 50) 
			cout << "Top-right is a hit" << endl;

		cout << "Points of the code" << endl;
		for (int i = 0; i < 4; ++i) {
			cout << pts[i] << endl;
		}

		// Find the relative location
		// Defining "North" to be at 0 degrees
		

		// Find the seconds it took to process
		debug_t = ((double) getTickCount() - debug_t) / getTickFrequency();
		cout << "Process time (s): " << debug_t << endl;
	}

    namedWindow( "Test", WINDOW_NORMAL );// Create a window for display.
    imshow( "Test", image2 );                   // Show our image inside it.

    waitKey(0);                          // Wait for a keystroke in the window

	// Clean up
	img.set_data(NULL, 0);

    return 0;
}
