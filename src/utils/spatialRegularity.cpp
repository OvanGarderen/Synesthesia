#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

int main(int argc, char **argv) {
  if (argc < 3) return 1;

  cv::VideoCapture instream(argv[1]);

  // get resolution
  int frameW = instream.get(cv::CAP_PROP_FRAME_WIDTH);
  int frameH = instream.get(cv::CAP_PROP_FRAME_HEIGHT);
  int fps = instream.get(cv::CAP_PROP_FPS);
  int frames = instream.get(cv::CAP_PROP_FRAME_COUNT);

  int fourcc = cv::VideoWriter::fourcc('M','P','E','G');
  cv::VideoWriter outstream(argv[2], fourcc, fps, cv::Size(frameW, frameH));

  cv::namedWindow("Canny edge");

  bool empty = true;
  uint index = 0;
  int member = 5;
  cv::Mat past[member];

  int i = 0;
  while(true) {
    cv::Mat frame;
    instream >> frame;
    
    if (frame.empty()) break;
    
    cv::Mat gray, out[3];
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY );
      
    // magic params
    int lowThreshold = 20;
    int highThreshold = 5;
    int kernel_size = 3;

    cv::adaptiveThreshold(gray, out[0], 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 
			  3 + 2 * (lowThreshold/8), 2);
    cv::adaptiveThreshold(gray, out[1], 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 
			  3 + 8 * (lowThreshold/8), 2);
    cv::adaptiveThreshold(gray, out[2], 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 
			  3 + 16 * (lowThreshold/8), 2);
    
    /*
    cv::Mat shepards;
    BuildFlowMap(canny, shepards);
    */
    /*
    if (empty) {
      for(int i=0; i<member; i++)
	canny.copyTo(past[i]);
      index = 1;
      empty = false;
    } else {
      canny.copyTo(past[index]);
      index = (index + 1) % member;
    }
    */

    // average over the last few frames
    //cv::Mat oframe = cv::Mat::zeros(canny.size(), CV_8U); //CV_64FC3, cv::Scalar(0.0));
    //cv::Mat out[3];
    //for (int i = 0; i < member; i++) {
    //  int j = goodmod(index - i, member);
    //  oframe +=  1/(1.0 + i) * past[j];
    //}
    //oframe = past[index]; //0.5;
    //oframe.convertTo(out, CV_8UC3, 128.0, 128.0);

    cv::Mat show;
    cv::Mat cannyBGR;
    //cv::cvtColor(canny, cannyBGR, cv::COLOR_GRAY2BGR);
    cv::merge(out, 3, cannyBGR);
    cv::hconcat(out[0], out[1], show);
    cv::hconcat(show, out[2], show);

    cv::Size size(3*600, 400);
    cv::resize(show, show, size);
    cv::imshow("Canny edge", show);
    cv::waitKey(1);
    
    outstream << cannyBGR;
    std::cout << "\033[A\033[2K" << "frame " << ++i << " of " << frames << " ( " << i/fps << "s of " << frames/fps << "s )" << std::endl;
  }

  outstream.release();
  instream.release();

  return 0;
}
