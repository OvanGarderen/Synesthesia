#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "shader.h"
#include <unistd.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <sys/stat.h>

using namespace std;
using namespace cv;

#define NUM_DEFORMS 100

uint window_w = 1600;
uint window_h = 900;

const string wName= "gl";
unsigned int VBO, VAO;
float rot = 0.0;
float param = 0.0;

void canny_threshold(const char * img, int thresh);
bool fileExists(const char* file) {
    struct stat buf;
    return (stat(file, &buf) == 0);
}

int main(int argc, char * argv[])
{
  if (argc < 2) {
    std::cout << "No image provided" << std::endl;
    return -1;
  }

  if (!fileExists("temp/distance.png")) {
    if (argc > 2) 
      canny_threshold(argv[1], atoi(argv[2]));
    else
      canny_threshold(argv[1], 40);
  }

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(window_w, window_h, wName.c_str(), NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  
  std::cout << "Init shader" << std::endl;
  Shader shader("shaders/trivial.vs","shaders/undulate.fs");
  std::cout << "Have shader" << std::endl;
  
  float vertices[] = {
    // positions          // colors           // texture coords
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
  };
  unsigned int indices[] = {  
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };
  struct datapoint {
    float x;
    float y;
  } displacements[NUM_DEFORMS];

  unsigned int VBO, VAO, EBO, PTBUF;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenVertexArrays(1, &PTBUF);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture coord attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

//LOAD IMAGE TEXTURE
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);  
  unsigned char *data = stbi_load(argv[1], &width, &height, &nrChannels, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);
//IMAGE LOADED

//LOAD DISTANCE MAP
  unsigned int texture2;
  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);
  // set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load and generate the texture
  int width2, height2, nrChannels2;
  stbi_set_flip_vertically_on_load(true);  
  unsigned char *data2 = stbi_load("temp/distance.png", &width2, &height2, &nrChannels2, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
  stbi_image_free(data2);
//DISTANCE MAP LOADED
  
  std::cout << "starting loop" << std::endl;
  
  glViewport(0, 0, window_w, window_h);
  
  while(!glfwWindowShouldClose(window)) {
    rot += 0.01;
    param += 0.01;

    // clear the colorbuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned int paramLoc = glGetUniformLocation(shader.ID, "param");
    glUniform1f(paramLoc, param);

    int texLoc = glGetUniformLocation(shader.ID, "ourTexture");
    glUniform1i(texLoc, 0);

    texLoc = glGetUniformLocation(shader.ID, "distanceMap");
    glUniform1i(texLoc, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // render the quad
    shader.use();

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
    usleep(100);
  }
  
  glfwTerminate();
  return 0;
}


void canny_threshold(const char * img, int thresh)
{
  Mat src = imread(img);

  if( !src.data )
  { exit(1); }

  /// Convert the image to grayscale and blur slightly to reduce noise
  Mat gray, sobx, soby;
  cvtColor( src, gray, CV_BGR2GRAY );
  blur( gray, gray, Size(3,3) );

  // magic params
  int lowThreshold = thresh;
  int ratio = 3;
  int kernel_size = 3;

  /// Canny detector
  Canny( gray, gray, lowThreshold, lowThreshold*ratio, kernel_size );

  // write to a temporary file
  std::vector<int> compression_params;
  compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
  compression_params.push_back(9);

  imwrite("temp/canny.png", gray, compression_params);

  // do a distance transform
  Mat dst;

  bitwise_not (gray, gray);
  distanceTransform(gray, dst, CV_DIST_L2, CV_DIST_MASK_PRECISE);

  Sobel( dst, sobx, CV_8U, 1, 0, 3, 2, 128, BORDER_DEFAULT);
  Sobel( dst, soby, CV_8U, 0, 1, 3, 2, 128, BORDER_DEFAULT);
  Mat smoothx, smoothy;
  GaussianBlur(sobx, smoothx, Size(5,5), 0, 0);
  GaussianBlur(soby, smoothy, Size(5,5), 0, 0);

  Mat r,g,b,conv;
  vector<Mat> list;
  smoothx.convertTo(r, CV_8UC1);
  smoothy.convertTo(g, CV_8UC1);
  dst.convertTo(b, CV_8UC1);
  list.push_back(r);
  list.push_back(g);
  list.push_back(b);
  merge(list, conv);

  imwrite("temp/distance.png", conv, compression_params);
}
