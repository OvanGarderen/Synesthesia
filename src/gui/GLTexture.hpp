#include <iostream>
#include <string>

#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;
using namespace std::chrono;

class GLTexture {
public:
  int w,h,channels;

  // a handle to the actual pixel data
  using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;

  GLTexture() = default;
  GLTexture(const std::string& textureName)
    : textureName(textureName), textureId(0) {}
  GLTexture(const std::string& textureName, GLint textureId)
    : textureName(textureName), textureId(textureId) {}
  GLTexture(const GLTexture& other) = delete;
  GLTexture(GLTexture&& other) noexcept
    : textureName(std::move(other.textureName)),
      textureId(other.textureId) {
    other.textureId = 0;
  }

  GLTexture& operator=(const GLTexture& other) = delete;
  GLTexture& operator=(GLTexture&& other) noexcept {
    textureName = std::move(other.textureName);
    std::swap(textureId, other.textureId);
    return *this;
  }

  ~GLTexture() noexcept {
    if (textureId)
      glDeleteTextures(1, &textureId);
  }

  GLuint texture() const { return textureId; }
  const std::string& name() const { return textureName; }

  /**
   *  Wrapper for the neccessary OpenGL calls
   **/
  void reloadTexture(handleType &textureData) {
    if (!textureData)
      throw std::invalid_argument("Could not reload texture data");

    /* free old GL texture and allocate a new one */
    if (textureId) {
      glDeleteTextures(1, &textureId);
      textureId = 0;
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    /* set number of channels in OpenGL */
    GLint internalFormat;
    GLint format;
    switch (channels) {
    case 1: internalFormat = GL_R8; format = GL_RED; break;
    case 2: internalFormat = GL_RG8; format = GL_RG; break;
    case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
    case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
    default: internalFormat = 0; format = 0; break;
    }
    
    // generate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());

    // set appropriate wrapping and filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
    

  /**
   *  Load a file in memory and creates an OpenGL texture.
   *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
   */
  handleType load(const std::string& fileName) {

    int force_channels = 0;

    // the stbi library loads the image and knows how to deallocate it
    handleType textureData(stbi_load(fileName.c_str(), &w, &h, &channels, force_channels), 
			   stbi_image_free);

    reloadTexture(textureData);
    return textureData;
  }

  // if no deallocation is necessary
  static void noop(void *) {};

  /**
   *  Wraps a cv::Mat to create an OpenGL texture.
   *  Returns a handle to the loaded pixels.
   *  The handle is only valid for the lifetime of the cv::Mat, (use as jit wrapper before rendering)
   */
  handleType load(cv::Mat image) {
    if(image.empty())
      throw std::invalid_argument("image empty");

    if (textureId) {
      glDeleteTextures(1, &textureId);
      textureId = 0;
    }

    int force_channels = 0;
    w = image.cols;
    h = image.rows;
    channels = image.channels();

    // return a pointer to the image data,
    handleType textureData(image.ptr(), GLTexture::noop);

    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

    reloadTexture(textureData);
    return textureData;
  }

private:
  std::string textureName;
  GLuint textureId;
};

