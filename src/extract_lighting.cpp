#include "segmentation.h"

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

  // Let's play around with the value channel
  //imshow("value", chans[1]);
  //waitKey(0);

  Mat blur1;
  cv::medianBlur(chans[1], blur1, 5);
  cv::medianBlur(blur1, blur1, 11);
  cv::medianBlur(blur1, blur1, 15);
  cv::medianBlur(blur1, blur1, 21);
  //imshow("blur1", blur1);
  //waitKey(0);

  Mat blur2;
  cv::medianBlur(chans[2], blur2, 5);
  cv::medianBlur(blur2, blur2, 11);
  //cv::medianBlur(blur2, blur2, 15);
  //cv::medianBlur(blur2, blur2, 21);
  //imshow("blur2", blur2);
  //waitKey(0);

  Mat thresh;
  cv::adaptiveThreshold(chans[2],thresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,11,2);
  
  chans[0] = blur2;
  chans[1] = blur2;
  chans[2] = blur2;

  //cv::subtract(blur, thresh, chans[2]);
  //cv::divide(chans[2], blur2, chans[2], 30.0);
/*  Mat background;
  Mat foreground;
  cv::multiply(thresh, chans[2], background, 1/255.0);
  cv::bitwise_not(thresh, thresh);
  cv::multiply(thresh, chans[2], foreground, 1/255.0);
  cv::medianBlur(background, background, 11);
  cv::medianBlur(background, background, 15);
  cv::medianBlur(background, background, 21);

  cv::bitwise_not(thresh, thresh);
  cv::multiply(thresh, background, background, 1/255.0);

  cv::add(background, foreground, chans[2]);
*/
  Mat m;
  cv::merge(chans, 3, m);
  //cv::cvtColor(m, m, CV_HSV2BGR);
  display(m);

  imwrite("out.jpg", m);
}
