/* System */
#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <memory>
#include <utility>
#include <chrono>
#include <libgen.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;
using namespace std::chrono;


/* Nanogui */
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

/* OpenCV */
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>

/* local includes */
#include "render_ffmpeg.h"
#include "loadImageDir.h"
#include "GLTexture.hpp"
#include "shader_widget.hpp"

class App : public nanogui::Screen {
public:
  App() : nanogui::Screen(Eigen::Vector2i(1024, 768), "NanoGUI Test") {
    using namespace nanogui;

    int maxVertUniformsVect;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxVertUniformsVect);
    std::cout << maxVertUniformsVect << std::endl;

    // generate framebuffer and its texture
    glGenFramebuffers(1, &frameBuffer);
    glGenTextures(1, &frameTex);

    // open video
    vidstream.open("inputs/walk.mp4");
    depthstream.open("temp/depth.mp4");
    dispstream.open("temp/shepards.mp4");

    cv::Mat out;
    vidstream.read(out);
    currentTex.load(out);

    depthstream.read(out);
    currentDepth.load(out);

    dispstream.read(out);
    currentDisp.load(out);

    // change size to match video
    glBindTexture(GL_TEXTURE_2D, frameTex);    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
		 currentTex.w, currentTex.h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  

    // bind texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameTex, 0);
    
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

    // Image panel 
    auto window = new Window(this, "File");
    window->setPosition(Vector2i(20, 15));
    window->setLayout(new GroupLayout());

    new Label(window, "Image panel & scroll panel", "sans-bold");
    PopupButton *imagePanelBtn = new PopupButton(window, "Image Panel");
    imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
    auto popup = imagePanelBtn->popup();
    VScrollPanel *vscroll = new VScrollPanel(popup);
    ImagePanel *imgPanel = new ImagePanel(vscroll);
    imgPanel->setImages(images);
    popup->setFixedSize(Vector2i(300, 100));

    // Change the active textures.
    imgPanel->setCallback([this](int i) {
	this->setCurrentImage(i);
	cout << "Selected item " << i << std::endl;
      });
    
    Button *b = new Button(window, "Output to file", ENTYPO_ICON_CLAPPERBOARD);
    b->setCallback([this](){
	// render 400 frames
	this->renderToFile(2000);
	//this->hideWindows();
	cout << "Started rendering to file" << std::endl;
      });

    // Distortion control panel
    shader = new ShaderWidget(this, "shaders/gui_lsd.json");
    shader->setPosition(Vector2i(20, 115));

    /*
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

    // HSV control panel 
    window = new Window(this, "HSV control");
    window->setPosition(Vector2i(800, 15));
    window->setLayout(new GroupLayout());

    new Label(window, "Hue (frequency/wavelength/amplitude/offset)", "sans-bold");
    new UniformSlider(this, window, "freqHue", std::make_pair(0.5f,10.0f), 3.0f);
    new UniformSlider(this, window, "wlenHue", std::make_pair(0.1f,5.0f), 3.0f);
    new UniformSlider(this, window, "ampHue", std::make_pair(0.0f,.5f), .1f);
    new UniformSlider(this, window, "offHue", std::make_pair(-.5f,.5f), 0.0f);

    new Label(window, "Saturation (frequency/wavelength/amplitude/offset)", "sans-bold");
    new UniformSlider(this, window, "freqSat", std::make_pair(0.5f,10.0f), 5.5f);
    new UniformSlider(this, window, "wlenSat", std::make_pair(0.1f,5.0f), 3.0f);
    new UniformSlider(this, window, "ampSat", std::make_pair(0.0f,.5f), .1f);
    new UniformSlider(this, window, "offSat", std::make_pair(0.0f,.5f), 0.3f);

    new Label(window, "Value (frequency/wavelength/amplitude/offset)", "sans-bold");
    new UniformSlider(this, window, "freqVal", std::make_pair(0.5f,10.0f), 1.5f);
    new UniformSlider(this, window, "wlenVal", std::make_pair(0.1f,5.0f), 3.0f);
    new UniformSlider(this, window, "ampVal", std::make_pair(0.0f,.5f), .25f);
    new UniformSlider(this, window, "offVal", std::make_pair(0.0f,.5f), -0.1f);
    */

    performLayout();

    shaderInit();

    msprev = high_resolution_clock::now();
    lastframeTime = high_resolution_clock::now();
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

    // get the exact time
    high_resolution_clock::time_point now = high_resolution_clock::now();
    
    //The renderer slows us down a lot, so we need constant time interval per frame
    if (renderer != nullptr) {
      // advance time by one frame unit
      time += 0.05;
    } else {
      time += duration_cast<milliseconds>(now - msprev).count()/1000.0;
    }

    // update clock
    msprev = now;

    // advance a frame every tick (24 fps), if done, rewind
    int ticks = 24 * duration_cast<milliseconds>(now - lastframeTime).count()/1000.0;
    if ( ticks > 0 || renderer != nullptr) {
    cv::Mat outVid, outDepth, outDisp;
    bool gotFrame = true;
    //  // try to get a couple of frames
    //  for (int i = 0; i < ticks; i++) {
    if (!(gotFrame = (vidstream.read(outVid) && depthstream.read(outDepth) && dispstream.read(outDisp)))) {
      // stop the renderer at the end of the video
      if (renderer != nullptr) renderer->stop();
      
      // rewind stream and try again
      vidstream.set(cv::CAP_PROP_POS_FRAMES, 0);
      depthstream.set(cv::CAP_PROP_POS_FRAMES, 0);
      dispstream.set(cv::CAP_PROP_POS_FRAMES, 0);
      gotFrame = vidstream.read(outVid) && depthstream.read(outDepth) && dispstream.read(outDisp);
    }
    //}
    if (gotFrame) {
      currentTex.load(outVid);
      currentDepth.load(outDepth);
      currentDisp.load(outDisp);
    }
    lastframeTime = now;
    }

    // Draw the window contents using OpenGL
    shader->activate(time);  

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, currentTex.w, currentTex.h);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, currentTex.texture());

    auto lookup = mDataLayers.find("distance");
    //if (lookup == mDataLayers.end()) std::cout << "distance map not found" << std::endl;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, currentDepth.texture()); //lookup->second.first.texture());

    lookup = mDataLayers.find("shepards");
    //if (lookup == mDataLayers.end()) std::cout << "flow map not found" << std::endl;
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, currentDisp.texture()); //lookup->second.first.texture());

    lookup = mDataLayers.find("mixin1");
    if (lookup == mDataLayers.end()) std::cout << "mixin layer 1 not found" << std::endl;
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, lookup->second.first.texture());

    lookup = mDataLayers.find("mixin2");
    if (lookup == mDataLayers.end()) std::cout << "mixin layer 2 not found" << std::endl;
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, lookup->second.first.texture());

    shader->draw();
    
    if (renderer != nullptr) {
      renderFrame();
    }

    // now render to the actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, mFBSize(0), mFBSize(1));

    trivialShader.bind();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, frameTex);

    // make sure the shader knows that the frame is a different size
    float frameRatioX = ((float) currentTex.w)/mFBSize(0);
    float frameRatioY = ((float) currentTex.h)/mFBSize(1);
    trivialShader.setUniform("frameRatioX", frameRatioX);
    trivialShader.setUniform("frameRatioY", frameRatioY);
    trivialShader.drawIndexed(GL_TRIANGLES, 0, 2);
  }

  void shaderInit() {
    using namespace nanogui;
    
    if (shader != nullptr)
      shader->shaderInit();
    
    nanogui::MatrixXu indices(3, 2); // Draw 2 triangles 
    indices.col(0) << 0, 1, 3;
    indices.col(1) << 1, 2, 3;
    
    nanogui::MatrixXf positions(3, 4);
    positions.col(0) <<  1.0, -1.0, 0.0;
    positions.col(1) <<  1.0,  1.0, 0.0;
    positions.col(2) << -1.0,  1.0, 0.0;
    positions.col(3) << -1.0, -1.0, 0.0;
    
    nanogui::MatrixXf texCoords(2, 4);
    texCoords.col(0) <<  1.0,  1.0;
    texCoords.col(1) <<  1.0,  0.0;
    texCoords.col(2) <<  0.0,  0.0;
    texCoords.col(3) <<  0.0,  1.0;

    trivialShader.initFromFiles("Trivial shader", "shaders/gui.vs", "shaders/gui_trivial.fs");
    
    trivialShader.bind();
    trivialShader.uploadIndices(indices);
    trivialShader.uploadAttrib("position", positions);
    trivialShader.uploadAttrib("texcoord", texCoords);
    trivialShader.setUniform("frame", 1);
  }
  
  void renderFrame() {
    renderer->step();
    if (!renderer->active()) {
      renderer = nullptr;
      std::cout << "finished rendering" << std::endl;
    }
  }

  void setCurrentImage(uint i) {
    currentImage = i;
  }

  void renderToFile(uint i) {
    renderer = std::make_unique<Renderer>("out.mpg", frameBuffer, i, currentTex.w, currentTex.h);

    // rewind the video
    vidstream.set(cv::CAP_PROP_POS_FRAMES, 0);
    depthstream.set(cv::CAP_PROP_POS_FRAMES, 0);
    dispstream.set(cv::CAP_PROP_POS_FRAMES, 0);
  }

  void hideWindows() {
      hideAllWindows = true;
  }

private:
  nanogui::ProgressBar *mProgress;
  nanogui::GLShader trivialShader;

  ShaderWidget *shader = nullptr;
    
  using imageData = pair<GLTexture, GLTexture::handleType>;
  vector<imageData> mImages;
  uint currentImage = 8;
  std::map<std::string, imageData> mDataLayers;

  cv::VideoCapture vidstream;
  cv::VideoCapture depthstream;
  cv::VideoCapture dispstream;
  GLTexture currentTex;
  GLTexture currentDepth;
  GLTexture currentDisp;
  high_resolution_clock::time_point lastframeTime;

  GLuint frameBuffer;
  GLuint frameTex;

  high_resolution_clock::time_point msprev;
  float time = 0;

  bool hideAllWindows = false;
  std::unique_ptr<Renderer> renderer = nullptr;
};

int main(int argc, char ** argv) {
  try {
    nanogui::init();

    // scoped variables 
    {
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
