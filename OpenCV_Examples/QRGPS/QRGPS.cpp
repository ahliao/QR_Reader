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
    image = imread(argv[1], 0);   // Read the file
	Mat image2;
	cvtColor(image, image2, CV_GRAY2RGB);

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

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
			line(image2,pts[i],pts[(i+1)%4],Scalar(255,255,0),5);  
		}  
		cout<<"Angle: "<<r.angle<<endl;  
	}

    namedWindow( "Test", WINDOW_NORMAL );// Create a window for display.
    imshow( "Test", image2 );                   // Show our image inside it.

    waitKey(0);                          // Wait for a keystroke in the window

	// Clean up
	img.set_data(NULL, 0);

    return 0;
}
