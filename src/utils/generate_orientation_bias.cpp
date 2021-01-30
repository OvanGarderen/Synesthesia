#include "segmentation.h"
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <time.h>

float clamp(float lower, float val, float upper)
{
  return max(lower,min(val,upper));
}

int goodmod(int val, int max)
{
  int m = val % max;
  if (m < 0) return m + max;
  return m;
}

void display(const Mat &m)
{
  // display at comfortable size
  Mat dst;
  int w=m.size().width, h=m.size().height;
  if (w==0 || h==0) {
    std::cout << "got empty image" << std::endl;
    return;
  }

  std::cout << w << " " << h << std::endl;
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

float Ricker_wavelet(float sigma, float x, float y)
{
  float factor = pow(1/(sqrt(2)*sigma),2.);
  return 2*factor * (1 - 2* x*x*factor) * exp(-(x*x+y*y)*factor); 
}

void Ricker_wavelet_kernel(float angle, int size, Mat &kernel)
{
  kernel = Mat::zeros(Size(size,size), CV_32F);
  float total = .0;

  float ox = cos(angle);
  float oy = sin(angle);
  for(int i=0; i<size; i++)
    for(int j=0; j<size; j++) {
      float pixel = Ricker_wavelet(0.1 * size, ox * (j-size/2) - oy * (i-size/2), ox * (i-size/2) + oy * (j-size/2));
      kernel.at<float>(j,i) = pixel;
      total += pixel;
    }
  double min, max;
  cv::minMaxLoc(kernel, &min, &max);
  std::cout << "min : " << min << " max: " << max << std::endl;
  kernel = kernel/max;
}

void reconstruct(Mat orig, Mat *v1, int *alpha, Mat *kernels, int num_kernels, int size, Mat &out)
{
  orig.convertTo(out, CV_32F);
  out = .0 * out;///255.0;
  int w = v1[0].size().width;
  int h = v1[0].size().height;

  for(int ix = 0; ix < size; ix++) {
    for(int iy = 0; iy < size; iy++) {
      for(int i=0; i<num_kernels; i++) {
	float val = kernels[i].at<float>(iy,ix) / (size * num_kernels);
	
	out.forEach<float>([&](float &pixel, const int position[]){
			     pixel += (alpha[i]-25.) * val * v1[i].at<float>(max(0,min(h-1,position[0]+iy-size/2)),
						max(0,min(w-1,position[1]+ix-size/2)));
	});
      }
    }
  }
}

int s_kernels = 11;
#define n_kernels 4
Mat kernels[n_kernels];
Mat gray;
Mat out_channel[n_kernels];
Mat hsv;
Mat chans[4];

int edgec[n_kernels] = {50,50,50,50};
Mat tempchans[3];
Mat temphsv;

void ontrackbar(int value)
{
  chans[0].copyTo(tempchans[0]);
  chans[1].copyTo(tempchans[1]);
  chans[2].copyTo(tempchans[2]);

  Mat rec, kek;
  reconstruct(gray, out_channel, edgec, kernels, n_kernels, s_kernels, kek);
  kek.convertTo(tempchans[2], CV_8U, 255);
  merge(tempchans,3,temphsv);
  cv::cvtColor(temphsv, rec, CV_HSV2BGR);

  std::cout<< "showing with value " << value << std::endl;
  imshow("Edges", rec);
}

int main(int argc, char* argv[])
{
  srand(time(NULL));

  for(int i=0; i<n_kernels; i++) {
    Ricker_wavelet_kernel(M_PI/n_kernels * i, s_kernels, kernels[i]);
    //display(.5*kernels[i]+.5);
  }

  Mat img = imread(argv[1]);
  //display(img);
  cv::cvtColor(img, hsv, CV_BGR2HSV);
  cv::split(hsv, chans);

  gray = chans[2];
  //cvtColor(img,gray,CV_BGR2GRAY);

  float fovea = 1.0;
  for(int i=0; i<n_kernels; i++) {
    out_channel[i] = Mat::zeros(gray.size(), CV_32F);
    int w = gray.size().width;
    int h = gray.size().height;

    out_channel[i].forEach<float>([&](float &pixel, const int position[]){
        int y=position[0];
	int x=position[1];
	
	for(int ix=0; ix< s_kernels; ix++)
	  for(int iy=0; iy< s_kernels; iy++) {

	    float tx = x + ix - s_kernels/2; //(w/8) * radius * cos(angle) + w/2;
	    float ty = y + iy - s_kernels/2; //(h/2) * radius * sin(angle) + h/2;

	    if (tx > 0.0 && tx < w && ty > 0.0 && ty < h)
	      pixel += ((float)gray.at<uchar>(ty, tx))/255.0 * kernels[i].at<float>(iy, ix);
	  }
    });
    std::cout << "Out total: " << sum(out_channel[i])[0] << std::endl;
    //display(out_channel[i]);
  }

  Mat m1, m2;
  merge(out_channel, 3, m1);
  //display(m1);
  if (n_kernels >= 6) {
    merge(out_channel+3, 3, m2);
    //display(m2);
  }

  Size size(s_kernels, s_kernels);
  for (int i = 2; i < 2 + n_kernels/2; i++)
    blur(out_channel[i], out_channel[i], size);
  
  std::cout << "result" << std::endl;
  Mat out1,out2;
  m1.convertTo(out1, CV_8U, 255);
  m2.convertTo(out2, CV_8U, 255);

  imwrite("edges1.png",out1);
  imwrite("edges2.png",out2);
  
  cvNamedWindow("Edges");
  imshow("Edges", out_channel[0]);

  Mat temp_channel[s_kernels];
  for (int i = 0; i<s_kernels; i++)
	 out_channel[i].copyTo(temp_channel[i]);

  float time_step = .05;
  int alpha = 10;
  int mu = 10;
  int nu = 1;
  int w = temp_channel[0].size().width;
  int h = temp_channel[0].size().height;
  int step = 0;

  cv::createTrackbar("Alpha", "Edges", &alpha, 150);
  cv::createTrackbar("Mu", "Edges", &mu, 200);
  cv::createTrackbar("Nu", "Edges", &nu, 150);
  while(true) {
    step++;
    for (int i = 0; i<n_kernels; i++) {
      float angle = M_PI/n_kernels * i;
      float dx = cos(angle);
      float dy = sin(angle);
      temp_channel[i].forEach<float>(
	 [&](float &pixel, const int position[]){
	   float diff = .0;
	   diff += -alpha * pixel;
	   
	   for (int j = 0; j < n_kernels; j++) {
	     diff += mu * Ricker_wavelet(n_kernels/4, i-j, 0) * 
	       temp_channel[j].at<float>(goodmod(position[0],h),goodmod(position[1],w));
	   }

	   for (int j = -10; j < 10; j++) {
	     diff += nu * Ricker_wavelet(8, j, 0) * temp_channel[i].at<float>(goodmod(position[0]+j*dy, h), goodmod(position[1]+j*dx, w));
	   }
	   pixel = pixel + time_step * .5 * diff + .003 * (rand() % 10 - 5);
	 });
    }
    
    Mat kek;
    reconstruct(gray, temp_channel, edgec, kernels, n_kernels, s_kernels, kek);
    imshow("Edges", kek);
    std::cout << "step " << step << std::endl;
    cvWaitKey(50);
  }
  Mat rec, kek;
  reconstruct(gray, out_channel, edgec, kernels, n_kernels, s_kernels, kek);
  kek.convertTo(chans[2], CV_8U, 255);
  merge(chans,3,hsv);
  cv::cvtColor(hsv, rec, CV_HSV2BGR);

  imwrite("reconstruct.png", rec);
}
