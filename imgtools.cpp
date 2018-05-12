#include <algorithm>    
#include <vector>       
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "imgtools.h"

void Dilate(cv::Mat &map, uint size, uint iterations)
{
  int dilation_type = cv::MORPH_ELLIPSE;
  cv::Mat element = getStructuringElement( dilation_type, cv::Size( 2*size + 1, 2*size+1 ));
  
  for (uint j = 0; j < iterations; j++) cv::dilate( map, map, element);
}

void InterpolateMap(cv::Mat &map)
{
  Dilate(map, 3, 3);
  GaussianBlur(map, map, cv::Size(5,5), 0);
  GaussianBlur(map, map, cv::Size(15,15), 0);
}

void BuildFlowMap(const cv::Mat &canny, cv::Mat &map)
{
  map = cv::Mat(canny.size(), CV_64FC3);

  typedef std::vector<cv::Point> Contour;
  std::vector<Contour> contours;
  std::vector<cv::Vec4i> hierarchy;

  findContours( canny, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
  std::sort(contours.begin(), contours.end(), 
	    [](Contour a, Contour b) {
	      return cv::arcLength(a,false) > cv::arcLength(b,false);
	    });

  double max_len = cv::arcLength(contours.at(0), false);

  for(uint i = 0; i < contours.size(); i++) {
    double len = cv::arcLength(contours.at(i), false);
    
    // fix large contours
    if (len > .8 * max_len) {
      cv::drawContours(map, contours, i, cv::Scalar(0.0, 0.0, 0.0), 1);
    
    // flow medium contours
    } else if (len > 50) {
      cv::Vec4f line;
      cv::fitLine(contours.at(i), line, cv::DIST_L2, 0, 0.01, 0.01);

      cv::drawContours(map, contours, i, cv::Scalar(0.0, line[0], line[1]), 1);
    } 
  }

  InterpolateMap(map);
}

/* ShepardsMap
 * src - matrix with the right size for the Shepards Displacement map
 * dst - destination matrix
 * n_vectors - the number of displacements to use in the calculation
 * data - contiguous block of memory with at least n_vectors blocks of the form:
 *    [position x, position y, displacement x, displacement y]
 */
void ShepardsMap(const cv::Mat &src, cv::Mat &dst, uint n_vectors, double *data)
{
  dst = cv::Mat::zeros(src.size(), CV_64FC3);
  
  typedef cv::Point3_<double> Pixel;

  // opencv can parrelelize matrix operations
  // IDW can be calculated separately, so this is good
  dst.forEach<Pixel> (
    [n_vectors, &data](Pixel &pixel, const int * position) -> void {
      double totalx = 0.0;
      double totaly = 0.0;
      double weight = 0.0;

      double x = position[1];
      double y = position[0];

      // IDW of the two displacements is the sum weighted by inverse of distance to power p
      for (uint i = 0; i < n_vectors; i++) {
	// using power p=2 means we don't have to take square roots
	double dist = (data[4*i] - x) * (data[4*i] - x)
	              + (data[4*i + 1] - y) * (data[4*i + 1] - y);
	dist = (dist < 1.0 ? 1.0 : 1/dist);

	// temp temp temp
	totalx += data[4*i + 2] * dist;
	totaly += data[4*i + 3] * dist;
	weight += dist;
      }
      
      // divide through by total weight to normalize 
      pixel.x = totalx / weight;
      pixel.y = totaly / weight;
      pixel.z = 0.0; //weight;
    }
  );
}

/* GenDisplacements
 * Given a canny edge detection of an image, extract displacement vectors along
 * edges while keeping big structures fixed
 * 
 * canny - canny edge detected matrix
 * max - the maximum number of displacements to find, should be at least 50
 * data - a buffer of size at least 4 * max to store the displacements as
 *    [position x, position y, displacement x, displacement y]
 *
 * returns the actual number of displacements generated
 */
uint GenDisplacements(const cv::Mat &canny, uint max, double *data)
{
  std::srand(std::time(nullptr));

  typedef std::vector<cv::Point> Contour;
  std::vector<Contour> contours;
  std::vector<cv::Vec4i> hierarchy;

  std::cout << " Finding contours " << std::endl;
  findContours( canny, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
  
  std::cout << " Sorting contours " << std::endl;
  // sort the vector so we can extract the largest contours (this is not very optimized)
  std::sort(contours.begin(), contours.end(), 
	    [](Contour a, Contour b) {
	      return cv::arcLength(a,false) > cv::arcLength(b,false);
	    });

  std::cout << " Finding short contours " << std::endl;
  // find all contours that are short (but not too short)
  // these are the ones that will induce some movement
  std::vector<const Contour*> short_contours;
  std::for_each(contours.begin(), contours.end(), [&short_contours](const Contour &contour) {
		 double l = cv::arcLength(contour,false); 
		 if (l > 50) short_contours.push_back(&contour);
	       });
  std::cout << "Filtered to " << short_contours.size() << std::endl;
    
  std::cout << " Fixing corners " << std::endl;
  // fix the four corners of the image by default
  cv::Size size = canny.size();
  for (uint i = 0; i < 4 * 4; i++) data[i] = 0;
  data[4] = size.width;
  data[9] = size.height;
  data[12] = size.width;
  data[13] = size.height;

  std::cout << " Fixing large contours " << std::endl;
  // we fix the five largest contours in the image
  uint c_index = 4;
  uint current = 0;
  while (current < std::min((uint)contours.size(), (uint)10) && c_index < max) {
    // fix all the points in the contour (displacement = (0,0)) 
    Contour contour = contours.at(current);
    std::for_each(contour.begin(), contour.end(), [&c_index, max, &data](cv::Point &pt){
	// make sure not to overstep our memory by accident
	if (c_index < max && std::rand() % 3 == 0) {
	  data[4 * c_index]     = (double) pt.x;
	  data[4 * c_index + 1] = (double) pt.y;
	  data[4 * c_index + 2] = 0.0;
	  data[4 * c_index + 3] = 0.0;
	  c_index++;
	}
      });

    current++;
  }
  std::cout << " fixed " << c_index << " points" << std::endl;
  std::cout << " stopped at contour " << current << std::endl;

  std::cout << " Flowing points " << std::endl;
  // next we add a displacement point for as many points as we are still allowed
  current = 0;
  while (current < std::min((uint) short_contours.size(), (uint) 100) && c_index < max) {
    const Contour *contour = short_contours.at(current);
    
    // fit a line through the contour and use that for the displacement
    // displacement vectors are then automatically normalized
    // @todo should we multiply by the length of the short curves -> different contributions?
    cv::Vec4f line;
    double l = cv::arcLength(*contour, false);
    cv::fitLine(*contour, line, cv::DIST_L2, 0, 0.01, 0.01);
    std::for_each(contour->begin(), contour->end(), [l,line,max,&data,&c_index](const cv::Point &pt) {
	if (c_index < max && std::rand() % 8 != 0) {
	  data[4 * c_index]     = (double) pt.x;
	  data[4 * c_index + 1] = (double) pt.y;
	  data[4 * c_index + 2] = (double) l/50.0 * line[0];
	  data[4 * c_index + 3] = (double) l/50.0 * line[1];
	  c_index++;
	}
    });

    current++;
  }

  std::cout << "reached " << c_index << std::endl;
  return c_index;
}

