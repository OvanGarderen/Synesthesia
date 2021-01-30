/**
 * 
 * Takes a single channel image and returns a three channel image with R: original, G: x derivative, B: y derivative
 * 
 **/

#include "segmentation.h"
#include <cmath>
#include <iostream>
#include <string>

void display(const Mat &m)
{
  // display at comfortable size
  Mat dst;
  int w=m.size().width, h=m.size().height;
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
  destroyWindow("Display");
}

/**
 *  Given a grayscale image g(x,y), construct a best local linear representation at each pixel (x,y)
 *  as a function f(x+dx,y+dy) = a(x,y) dx + b(x,y) dy + g(x,y) such that the squared norm of 
 *           (f(x+dx,y+dy) - g(x+dx,y+dy))/(max(dx/size,dy/size,g(x,y) - g(x+dx,y+dy)) 
 *  is minimized. Pixels contribute more if they are in a close neighbourhood and hard edges are ignored
 *
 *  approx contains the three channels (g, a, b) 
 **/
void LocalLinearFit(Mat &gray, Mat &approx, double size)
{
  typedef Point3_<uint8_t> Pixel;

  // approx has the same size as gray but 3 channels
  Mat out(gray.size(), CV_8UC3);
  
  int w = gray.size().width, h = gray.size().height;
  std::cout << "<<" << w << "," << h << ">>";
  out.forEach<Pixel>([&](Pixel &p, const int*pos){			    
      // copy original into first channel
      p.x = gray.at<uint8_t>(pos[0], pos[1]);

      // calculate the matrix ((dx^2 xy)(xy dy^2)) with proper weights
      // and the distance vector (f(x,y) - f(x+dx,y+dy)) * (x,y) 
      int isize = (int) size;
      
      // find the centroid 
      typedef Point3_<double> vec3;
      typedef Matx<double,3,3> mat3;

      mat3 coefs(0.0, 0.0, 0.0,
		 0.0, 0.0, 0.0,
		 0.0, 0.0, 0.0);
      vec3 dist(0.0, 0.0, 0.0);

      int n = 0;
      for(int dx = -isize; dx < isize; dx++) {
	for(int dy = -isize; dy < isize; dy++) {
	  if (pos[0] + dx < 0 || pos[1] + dy < 0 || pos[0] + dx >= h || pos[1] + dy >= w)
	    continue;

	  double val = (double) (p.x - gray.at<uint8_t>(pos[0] + dx,pos[1] + dy));

	  // skip hard edges
	  double modulo = dx*dx + dy*dy;
	  if (abs(val) > fmax(1.0, 0.5 * sqrt(modulo)))
	    continue;

	  double weight = 1.0/fmax(0.1, (dx*dx + dy*dy)/(size * size));

	  coefs(0,0) += weight * dx*dx;
	  coefs(0,1) += weight * dy*dx;
	  coefs(0,2) += 0.0; // weight * dx;

	  coefs(1,0) += weight * dy*dx;
	  coefs(1,1) += weight * dy*dy;
	  coefs(1,2) += 0.0; //weight * dy;

	  coefs(2,0) += 0.0; //weight * dx;
	  coefs(2,1) += 0.0; //weight * dy;
	  coefs(2,2) += weight;

	  dist.x += weight * val * dx;
	  dist.y += weight * val * dy;
	  dist.z += 0.0; //weight * val;

	  n++;
	}
      }
      coefs = 1/((double) n) * coefs;

      // calculate inverse and multiply
      vec3 res = coefs.solve(dist, cv::DECOMP_SVD);
      double a = res.x;
      double b = res.y;
      
      p.x = p.x + res.z;
      //double sign = a > 0.0 ? 1.0 : -1.0;
      p.y = fmin(255.0, fmax(0.0, 128 + a));
      p.z = fmin(255.0, fmax(0.0, 128 + b));
  });

  std::cout << "all alright here" << std::endl;
  approx = out;
}

int main(int argc, char* argv[])
{
  Mat img = imread(argv[1]);

  Mat chans[4];
  cv::split(img, chans);
  chans[0].convertTo(chans[0], CV_8U);

  Mat blur;
  cv::medianBlur(chans[0], blur, 5);
  cv::medianBlur(blur, blur, 11);
  cv::medianBlur(blur, blur, 15);
  cv::medianBlur(blur, blur, 21);
  cv::medianBlur(blur, blur, 25);

  Mat lin;
  display(blur);
  LocalLinearFit(blur, lin, 10.0);
  cv::medianBlur(lin, lin, 25);

  Mat lchans[4];
  cv::split(lin, lchans);
  lchans[0] = chans[0];

  Mat out;
  cv::merge(lchans, 3, out);
  display(out);

  std::string fn(argv[1]);
  fn.erase(fn.size()-4);
  fn.append("_dxdy.png");
  imwrite(fn.c_str(), out);

  return 0;


  Mat temp;
  merge(chans,3,temp);

/*
  Mat out;
  temp.convertTo(out, CV_8U);
  display(out);

  std::string fn(argv[1]);
  fn.erase(fn.size()-4);
  fn.append("_dxdy.png");
  imwrite(fn.c_str(), out);
*/
}
