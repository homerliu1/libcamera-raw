
#include "libcamera-raw.hpp"
using namespace cv;

int main(int argc, char *argv[])
{
	try
	{
		Mat frame, framv = Mat(480, 640, CV_8UC3);
        LibcameraRaw cam;
        cam.open(0);
        sleep(2);  
        namedWindow("tst", 1);
        int c = 0;
        while(c != 27) {
            cam.read(frame); //the full reso image for real proc
            
            //just for show
            resize(frame, framv, framv.size());
            imshow("tst", framv);
            c = waitKey(200);            
            //std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

        cam.release();
        destroyWindow("tst");
        //imwrite("frm.jpg", frame);
    }
	catch (std::exception const &e)
	{
		std::cerr << "ERROR: *** " << e.what() << " ***" << std::endl;
		return -1;
	}
	return 0;
}
