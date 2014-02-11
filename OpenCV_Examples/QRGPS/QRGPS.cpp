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
		double qr_angle_rad = qr_angle * 3.1415 / 180;
		Point mid((pts[0].x + pts[2].x) / 2, (pts[0].y + pts[2].y)/2);
		Point p2(mid.x + 25*cos(qr_angle), mid.y - 25*sin(qr_angle));
		line(image2,mid, p2, Scalar(0,255,0),2);

		// Get the relative location based on the data of the QR code
		// QR format: x y
		// x and y are seperated by a single space
		// Check if the QR is in the right format
		// Defining "North" to be at 0 degrees

		// Assume the QR reads 0 0 for now
		int x = 300;
		int y = 300;
		line(image2,mid, Point(x,y), Scalar(30,0,30),2);

		// Relative position (in pixel)
		double dis2Mid = sqrt((mid.x - x) * (mid.x - x) + (mid.y - y) * (mid.y - y));
		cout << "Distance to Quad: " << dis2Mid << endl;
		
		//cout << "(" << x - 0 << ", " << y - 0 << ")" << endl;
		//cout << "X: " << sin(qr_angle)*dis2Mid + 0 << endl;
		//cout << "Y: " << cos(qr_angle)*dis2Mid + 0 << endl;
		double theta1 = atan2(y - mid.y, x - mid.x) * 180/3.1415;
		double theta2 = 90 - theta1 - qr_angle;
		cout << "qr_angle: " << qr_angle << endl;
		cout << "Theta1: " << theta1 << endl;
		cout << "Theta2: " << theta2 << endl;
		double theta2_rad = theta2 * 3.1415 / 180;
		cout << "X dis: " << dis2Mid * sin(theta2_rad) << endl;
		cout << "Y dis: " << dis2Mid * cos(theta2_rad) << endl;
	
		// Find the seconds it took to process
		debug_t = ((double) getTickCount() - debug_t) / getTickFrequency();
		cout << "Process time (s): " << debug_t << endl;
	}

    namedWindow( "Test", WINDOW_NORMAL );// Create a window for display.
	resizeWindow("Test",600,600);
    imshow( "Test", image2 );                   // Show our image inside it.

    waitKey(0);                          // Wait for a keystroke in the window

	// Clean up
	img.set_data(NULL, 0);

    return 0;
}
