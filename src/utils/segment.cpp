#include "segmentation.h"

int main(int argc, char **argv) 
{
 
  //Read the file
  Mat image = imread(argv[1], CV_LOAD_IMAGE_COLOR);
 
  // display the merged segments blended with the image
  Mat m = watershedWithMarkersAndHistograms(image);

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
  imshow("Final Result", dst);
 
  // wait until user press a key
  waitKey(0);
 
  //imwrite(output_file, wshedWithImage);
}
