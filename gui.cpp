#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <string>
#include <cstring>

#include <cstdint>
#include <memory>
#include <utility>
#include <chrono>
#include <libgen.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "loadImageDir.h"

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
  using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
  GLTexture() = default;
  GLTexture(const std::string& textureName)
    : mTextureName(textureName), mTextureId(0) {}

  GLTexture(const std::string& textureName, GLint textureId)
    : mTextureName(textureName), mTextureId(textureId) {}

  GLTexture(const GLTexture& other) = delete;
  GLTexture(GLTexture&& other) noexcept
    : mTextureName(std::move(other.mTextureName)),
      mTextureId(other.mTextureId) {
    other.mTextureId = 0;
  }
  GLTexture& operator=(const GLTexture& other) = delete;
  GLTexture& operator=(GLTexture&& other) noexcept {
    mTextureName = std::move(other.mTextureName);
    std::swap(mTextureId, other.mTextureId);
    return *this;
  }
  ~GLTexture() noexcept {
    if (mTextureId)
      glDeleteTextures(1, &mTextureId);
  }

  GLuint texture() const { return mTextureId; }
  const std::string& textureName() const { return mTextureName; }

  /**
   *  Load a file in memory and create an OpenGL texture.
   *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
   */
  handleType load(const std::string& fileName) {
    if (mTextureId) {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
    }
    int force_channels = 0;
    int w, h, n;
    handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
    if (!textureData)
      throw std::invalid_argument("Could not load texture data from file " + fileName);
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    GLint internalFormat;
    GLint format;
    switch (n) {
    case 1: internalFormat = GL_R8; format = GL_RED; break;
    case 2: internalFormat = GL_RG8; format = GL_RG; break;
    case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
    case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
    default: internalFormat = 0; format = 0; break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return textureData;
  }

private:
  std::string mTextureName;
  GLuint mTextureId;
};

class App : public nanogui::Screen {
public:
  App() : nanogui::Screen(Eigen::Vector2i(1024, 768), "NanoGUI Test") {
    using namespace nanogui;

    int maxVertUniformsVect;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxVertUniformsVect);
    std::cout << maxVertUniformsVect << std::endl;

    /**
     * Load image data
     **/
    string resourcesFolderPath("./");
    vector<pair<int, string>>
      images = loadImageDir(mNVGContext, "images", ".jpg");
    vector<pair<int, string>>
      temps = loadImageDir(mNVGContext, "temp", ".png");

    for (auto& image : images) {
      GLTexture texture(image.second);
      try {
	auto data = texture.load(resourcesFolderPath + image.second + ".jpg");
	mImages.emplace_back(std::move(texture), std::move(data));
      } catch (std::invalid_argument &error) {
	std::cout << error.what() << std::endl;
      }
    }

    for (auto& layer : temps) {
      GLTexture texture(layer.second);
      auto data = texture.load(resourcesFolderPath + layer.second + ".png");
      mDataLayers.emplace(std::strrchr(layer.second.c_str(),'/') + 1,std::make_pair(std::move(texture), std::move(data)));
      std::cout << layer.second << std::endl;
    }

    /**
     * Widgets
     **/
    auto window = new Window(this, "File selection");
    window->setPosition(Vector2i(200, 15));
    window->setLayout(new GroupLayout());

    new Label(window, "Image panel & scroll panel", "sans-bold");
    PopupButton *imagePanelBtn = new PopupButton(window, "Image Panel");
    imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
    auto popup = imagePanelBtn->popup();
    VScrollPanel *vscroll = new VScrollPanel(popup);
    ImagePanel *imgPanel = new ImagePanel(vscroll);
    imgPanel->setImages(images);
    popup->setFixedSize(Vector2i(245, 150));

    // Change the active textures.
    imgPanel->setCallback([this](int i) {
	this->setCurrentImage(i);
	cout << "Selected item " << i << '\n';
      });

    /* Distortion control panel */
    window = new Window(this, "Distortion");
    window->setPosition(Vector2i(200, 15));
    window->setLayout(new GroupLayout());

    new Label(window, "Speedup", "sans-bold");
    auto slider = new UniformSlider(this, window, "freqDistort", std::make_pair(0.1f,10.0f), 1.0f);

    new Label(window, "Distortion", "sans-bold");
    new UniformSlider(this, window, "ampDistort", std::make_pair(0.0f,2.0f), 1.0f);

    new Label(window, "Wobble", "sans-bold");
    new UniformSlider(this, window, "wobbleDistort", std::make_pair(-2.0f,2.0f), 0.0f);
    new UniformSlider(this, window, "wobbleAngDistort", std::make_pair(0.0f,3.1415f), 0.0f);

    new Label(window, "Distort Phase", "sans-bold");
    new UniformSlider(this, window, "edgePhaseDistort", std::make_pair(-20.0f,20.0f), 0.0f);
    new UniformSlider(this, window, "distxPhaseDistort", std::make_pair(-20.0f,20.0f), 0.0f);
    new UniformSlider(this, window, "distyPhaseDistort", std::make_pair(-20.0f,20.0f), 0.0f);

    /* HSV control panel */
    window = new Window(this, "HSV control");
    window->setPosition(Vector2i(200, 15));
    window->setLayout(new GroupLayout());

    new Label(window, "Hue (frequency/wavelength/amplitude/offset)", "sans-bold");
    new UniformSlider(this, window, "freqHue", std::make_pair(0.5f,10.0f), 3.0f);
    new UniformSlider(this, window, "wlenHue", std::make_pair(0.1f,5.0f), 3.0f);
    new UniformSlider(this, window, "ampHue", std::make_pair(0.0f,.5f), .1f);
    new UniformSlider(this, window, "offHue", std::make_pair(-.5f,.5f), 0.0f);

    new Label(window, "Saturation (frequency/wavelength/amplitude/offset)", "sans-bold");
    new UniformSlider(this, window, "freqSat", std::make_pair(0.5f,10.0f), 5.5f);
    new UniformSlider(this, window, "wlenSat", std::make_pair(0.1f,5.0f), 3.0f);
    new UniformSlider(this, window, "ampSat", std::make_pair(0.1f,.5f), .1f);
    new UniformSlider(this, window, "offSat", std::make_pair(-.5f,.5f), 0.3f);

    new Label(window, "Value (frequency/wavelength/amplitude/offset)", "sans-bold");
    new UniformSlider(this, window, "freqVal", std::make_pair(0.5f,10.0f), 1.5f);
    new UniformSlider(this, window, "wlenVal", std::make_pair(0.1f,5.0f), 3.0f);
    new UniformSlider(this, window, "ampVal", std::make_pair(0.1f,.5f), .25f);
    new UniformSlider(this, window, "offVal", std::make_pair(-.5f,.5f), -0.1f);

    performLayout();

    shaderInit("shaders/gui_lsd.fs");

    msprev = high_resolution_clock::now();
  }

  ~App() {
    mShader.free();
  }

  virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      setVisible(false);
      return true;
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
      hideAllWindows = !hideAllWindows;
      std::cout << (hideAllWindows ? "true" : "false") << std::endl;
      return true;
    }

    if (Screen::keyboardEvent(key, scancode, action, modifiers))
      return true;

    return false;
  }

  virtual void draw(NVGcontext *ctx) {
    if (!hideAllWindows) Screen::draw(ctx);
  }

  virtual void drawContents() {
    using namespace nanogui;
      
    high_resolution_clock::time_point now = high_resolution_clock::now();
    time += (now - msprev).count()/2000000000.0;
    msprev = now;
      
    /* Draw the window contents using OpenGL */
    mShader.bind();
      
    mShader.setUniform("time", time); 
      
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mImages.at(currentImage).first.texture());

    auto lookup = mDataLayers.find("distance");
    if (lookup == mDataLayers.end()) std::cout << "distance map not found" << std::endl;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, lookup->second.first.texture());

    lookup = mDataLayers.find("shepards");
    if (lookup == mDataLayers.end()) std::cout << "flow map not found" << std::endl;
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, lookup->second.first.texture());

    mShader.drawIndexed(GL_TRIANGLES, 0, 2);
  }

  void shaderInit(const std::string fragment) {
    using namespace nanogui;
    
    mShader.initFromFiles("LSD shader","shaders/gui.vs", fragment);
    
    MatrixXu indices(3, 2); /* Draw 2 triangles */
    indices.col(0) << 0, 1, 3;
    indices.col(1) << 1, 2, 3;
    
    MatrixXf positions(3, 4);
    positions.col(0) <<  1.0, -1.0, 0.0;
    positions.col(1) <<  1.0,  1.0, 0.0;
    positions.col(2) << -1.0,  1.0, 0.0;
    positions.col(3) << -1.0, -1.0, 0.0;
    
    MatrixXf texCoords(2, 4);
    texCoords.col(0) <<  1.0,  1.0;
    texCoords.col(1) <<  1.0,  0.0;
    texCoords.col(2) <<  0.0,  0.0;
    texCoords.col(3) <<  0.0,  1.0;
    
    mShader.bind();
    mShader.uploadIndices(indices);
    mShader.uploadAttrib("position", positions);
    mShader.uploadAttrib("texcoord", texCoords);
    mShader.setUniform("textureMap", 1);
    mShader.setUniform("distanceMap", 2);
    mShader.setUniform("shepardsMap", 3);

    mShader.setUniform("ampDistort", 1.0f);
    mShader.setUniform("freqDistort", 1.0f);
    mShader.setUniform("wobbleDistort", 1.0f);
    mShader.setUniform("wobbleAngDistort", 0.0f);
    mShader.setUniform("edgePhaseDistort", 1.0f);
    mShader.setUniform("distxPhaseDistort", 1.0f);
    mShader.setUniform("distyPhaseDistort", 1.0f);

    mShader.setUniform("freqHue", 3.0f);
    mShader.setUniform("wlenHue", 3.0f);
    mShader.setUniform("ampHue", .1f);
    mShader.setUniform("offHue", 0.0f);

    mShader.setUniform("freqSat", 5.5f);
    mShader.setUniform("wlenSat", 3.0f);
    mShader.setUniform("ampSat", .1f);
    mShader.setUniform("offSat", 0.3f);

    mShader.setUniform("freqVal", 1.5f);
    mShader.setUniform("wlenVal", 3.0f);
    mShader.setUniform("ampVal", .25f);
    mShader.setUniform("offVal", -0.1f);

  }
  
  void setUniform(const std::string uniform, float value) {
    mShader.bind();
    mShader.setUniform(uniform, value);
  }

  void setCurrentImage(uint i) {
    currentImage = i;
  }

private:
  nanogui::ProgressBar *mProgress;
  nanogui::GLShader mShader;
  
  using imageData = pair<GLTexture, GLTexture::handleType>;
  vector<imageData> mImages;
  uint currentImage = 8;
  std::map<std::string, imageData> mDataLayers;
  
  high_resolution_clock::time_point msprev;
  float time = 0;

  bool hideAllWindows = false;

  class UniformSlider : public nanogui::Widget {
    public: 
    UniformSlider(App * app, nanogui::Widget *parent, const char * name, std::pair<float,float> range, float value)
      : nanogui::Widget(parent) {
      using namespace nanogui;
      
      this->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Middle, 0, 40));
      
      Slider *slider = new Slider(this);
      slider->setValue(value);
      slider->setRange(range);
      slider->setFixedWidth(100);

      TextBox *textBox = new TextBox(this);
      textBox->setFixedSize(Vector2i(100, 25));
      textBox->setValue(std::to_string(value));

      slider->setCallback([app, name, textBox](float value){
	  app->setUniform(name, value);
	  textBox->setValue(std::to_string(value));
	});
    }
  };
};

int main(int argc, char ** argv) {
  try {
    nanogui::init();

    /* scoped variables */ {
      nanogui::ref<App> app = new App();
      app->drawAll();
      app->setVisible(true);
      nanogui::mainloop();
    }

    nanogui::shutdown();
  } catch (const std::runtime_error &e) {
    std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
    std::cerr << error_msg << endl;
    return -1;
  }

  return 0;
}
