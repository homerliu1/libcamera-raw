#pragma once
#include "core/libcamera_app.hpp"
#include "core/video_options.hpp"
#include <opencv2/opencv.hpp>

#include <chrono>
#include <mutex>
#include <pthread.h>
#include <atomic>

class LibcameraRaw : public LibcameraApp
{
public:
	LibcameraRaw() : LibcameraApp() {
        running.store(false, std::memory_order_release);
        frm_ready.store(false, std::memory_order_release);
        stream = 0;
    }

    void stopVideo() {        
        running.store(false, std::memory_order_release);;
        void *status;
        int ret = pthread_join(event_thread, &status);
        if(ret<0) throw std::string("Error joining thread");
     
      	StopCamera();
	    //StopEncoder();    
        frm_ready.store(false, std::memory_order_release);   
    }

    bool getFrame(cv::Mat &frame, unsigned int timeout_ms) {
        if(frm_ready.load(std::memory_order_acquire) == false) return false; //wait untill timeout_ms
        cv::Mat frmraw2;
        mtx.lock();
        frmraw10.copyTo(frmraw2);
        mtx.unlock();
        frame = cv::Mat(cv::Size(width, height), CV_8UC3);
        for(int j = 0; j < frmBayer.rows; j++) {
            raw10shift8(frmBayer.cols, frmraw10.ptr(j), frmBayer.ptr(j));
        }
        //imwrite("frmBayer.bmp", frmBayer);
        cv::cvtColor(frmBayer, frame, cv::COLOR_BayerRG2BGR);
        frm_ready.store(false, std::memory_order_release);
        return true;
    }

    bool startVideo(int argc, char *argv[]) {
		try	{
        VideoOptions *options = GetOptions();
 		if (options->Parse(argc, argv) == false) {
            throw std::string("parse cmd line fail.");
        }

        options->denoise = "off";
        options->rawfull = true;
        options->nopreview = true;

		if (options->verbose)	options->Print();

        frm_ready.store(false, std::memory_order_release);

        OpenCamera();
        ConfigureVideo(LibcameraRaw::FLAG_VIDEO_RAW);
        //StartEncoder();
        StartCamera();

        stream = RawStream();
        libcamera::StreamConfiguration const &cfg = stream->configuration();
        std::cerr << "Raw stream: " << cfg.size.width << "x" << cfg.size.height << " stride " << cfg.stride
                  << " format " << cfg.pixelFormat.toString() << std::endl;
        width = cfg.size.width; height = cfg.size.height; stride = cfg.stride;
        frmBayer = cv::Mat(cv::Size(width, height), CV_8UC1);
     
        int ret = pthread_create(&event_thread, NULL, &event_loop, this);
        if (ret != 0) throw std::string("Error starting view thread");
         
        }
    	catch (std::exception const &e)
	    {
		    std::cerr << "ERROR: *** " << e.what() << " ***" << std::endl;
		    return false;
	    }    
        return true; 
    }

protected:
    VideoOptions *GetOptions() const { return static_cast<VideoOptions *>(options_.get()); }

    static void * event_loop(void *p) {
        LibcameraRaw* papp = (LibcameraRaw*) p;
        std::cerr << "StartCamera" << std::endl;
         
        papp->running.store(true, std::memory_order_release);
        //int count = 0;
        while(papp->running.load(std::memory_order_acquire))	{
            LibcameraRaw::Msg msg = papp->Wait();

            if (msg.type != LibcameraRaw::MsgType::RequestComplete)
                throw std::runtime_error("unrecognised message!");
            CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);

            const std::vector<libcamera::Span<uint8_t>> mem = papp->Mmap(completed_request->buffers[papp->stream]);
            uint8_t *praw = (uint8_t *)mem[0].data();
            cv::Mat tmp = cv::Mat(cv::Size(papp->stride, papp->height), CV_8UC1, praw);
            papp->mtx.lock();
            tmp.copyTo(papp->frmraw10);
            papp->mtx.unlock();
            papp->frm_ready.store(true, std::memory_order_release);
            //std::cerr << " Request" << count++ << std::endl;
        }

        return NULL;
    }

    void raw10shift8(int n, const void* src, void* dst)    {
        char* psrc = (char*)src;
        char* pdst = (char*)dst;
        size_t sz = sizeof(char) * 4;
        for (int i = 0; i < n/4; i++) {
            memcpy(pdst, psrc, sz);
            psrc += 5;
            pdst += 4;
        }
    }

    std::mutex mtx;
    std::atomic<bool> running, frm_ready;
    cv::Mat frmraw10, frmBayer;
    int width, height, stride;
    LibcameraApp::Stream *stream;

    pthread_t event_thread;    
};
