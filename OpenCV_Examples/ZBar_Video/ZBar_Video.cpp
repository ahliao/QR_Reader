// Test program to read qr codes from video
#include <iostream>
#include <zbar.h>

using namespace std;
using namespace zbar;

// Currently using C++ as it's easier, C is also possible though
class MyHandler : public Image::Handler
{
	void image_callback(Image &image) 
	{
		
		/*for(SymbolIterator symbol = image.symbol_begin(); 
				symbol != image.symbol_end();
				++symbol)*/
		SymbolIterator symbol = image.symbol_begin();
		if (symbol != image.symbol_end())
		{
			//cout << "decoded " << symbol->get_type_name() << " symbol "
			//	<< "\"" << symbol->get_data() << "\"" << endl;
			cout << "I see something" << endl;
		}
	}
};

int main( int argc, char** argv )
{
	// create a Processor
	const char *device = "/dev/video0";
	if (argc > 1)
		device = argv[1];
	Processor proc(true, device);

	proc.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	MyHandler my_handler;
	proc.set_handler(my_handler);

	proc.set_visible();
	proc.set_active();

	try {
		proc.user_wait();
	} catch(ClosedError &e) { 
	}

    return 0;
}
