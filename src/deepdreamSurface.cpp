#include "segmentation.h"
#include <iostream>

void display(const Mat &m)
{
  // display at comfortable size
  Mat dst;
  int w=m.size().width, h=m.size().height;
  float ratio = 1.0;
  if (w < h) {
    ratio = 1920.0/((float) w);
  } else {
    ratio = 1080.0/((float) h);
  }
  resize(m, dst, cv::Size(0,0), ratio, ratio);

  //Display the merged segments blended with the image
  imshow("Display", dst);
  waitKey(0); 
}

int main(int argc, char **argv) 
{
 
  //Read the file
  Mat image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);
  display(image);
 
  Mat hsv;
  cv::cvtColor(image, hsv, CV_BGR2HSV);

  Mat chans[4];
  cv::split(hsv, chans);

  chans[2].copyTo(chans[0]);
  chans[2].copyTo(chans[1]);

  Mat m;
  cv::merge(chans, 3, m);
  display(m);

  imwrite("out_value.jpg", m);

  //Read the file
  Mat chans2[4];
  Mat inval = cv::imread("in_value.jpg", CV_LOAD_IMAGE_COLOR);
  std::cout << " (" << inval.channels() << ") " << std::endl;
  cv::split(inval, chans2);

  cv::split(hsv, chans);
  chans2[2].copyTo(chans[2]);

  Mat dream;
  cv::merge(chans, 3, m);
  cv::cvtColor(m, dream, CV_HSV2BGR);
  display(dream);

  imwrite("out_dreamed.jpg", dream);
}
