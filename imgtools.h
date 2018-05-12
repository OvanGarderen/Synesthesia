#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

/* BuildFlowMap
 * Given a canny edge detection of an image, extract displacement vectors along
 * edges and draw them into the image as a colourvalue
 * Some very easy interpolation is performed by blurring to make the map smoother
 * 
 * canny - canny edge detected matrix
 * dst - returned map
 */
void BuildFlowMap(const cv::Mat &canny, cv::Mat &map);

/* ShepardsMap
 * src - matrix with the right size for the Shepards Displacement map
 * dst - destination matrix
 * n_vectors - the number of displacements to use in the calculation
 * data - contiguous block of memory with at least n_vectors blocks of the form:
 *    [position x, position y, displacement x, displacement y]
 */
void ShepardsMap(const cv::Mat &src, cv::Mat &dst, uint n_vectors, double *data);

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
uint GenDisplacements(const cv::Mat &canny, uint max, double *data);
