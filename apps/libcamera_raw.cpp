
#include "libcamera-raw.hpp"

using namespace cv;
int main(int argc, char *argv[])
{
	try
	{
		Mat frame, framv = Mat(480, 640, CV_8UC3);
        LibcameraRaw cam_raw;
        cam_raw.startVideo(argc, argv);
        sleep(2);  
        namedWindow("tst", 1);
        int c = 0;
        while(c != 27) {
            cam_raw.getFrame(frame, 100);
            resize(frame, framv, framv.size());
            imshow("tst", framv);
            c = waitKey(200);
            //frame = 100;
            //std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

        cam_raw.stopVideo();
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
