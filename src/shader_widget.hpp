#include <fstream>

#include <nanogui/window.h>

/* JSON */
#include <nlohmann/json.hpp>
using json = nlohmann::json;

/**
 *  A widget wrapping the actual shader class, 
 *  Exposes shader uniforms to the user
 **/
class ShaderWidget : public nanogui::Window {

public:
  ShaderWidget(nanogui::Widget *parent, const std::string &config) 
    : nanogui::Window(parent)  
  {
    std::ifstream configStream;
    configStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      configStream.open(config.c_str());
      configStream >> this->config;
      configStream.close();
    } 
    catch (std::ifstream::failure &e) {
      std::cerr << "Error reading file " << config << std::endl;
    }
    catch (nlohmann::json::exception &e) {
      std::cerr << "Error reading file " << config << " : Invalid JSON" << std::endl;
      std::cerr << e.what() << std::endl;      
    }
    
    this->setTitle(this->config["name"]);
    this->shaderPath = this->config["path"];

    this->setLayout(new nanogui::GroupLayout());

    for (auto& [key, value] : this->config["parameters"].items()) {
      auto slider = new UniformSlider(this,
                            key, 
			    std::make_pair(value["min"],value["max"]),
			    value["val"]);
    }
  }

  ~ShaderWidget() 
  {
    mShader.free();
  }

  void shaderInit() {
    mShader.initFromFiles(this->config["name"], "shaders/gui.vs", this->shaderPath);

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
    
    mShader.bind();
    mShader.uploadIndices(indices);
    mShader.uploadAttrib("position", positions);
    mShader.uploadAttrib("texcoord", texCoords);

    std::cout << "TEST" << std::endl;
    GLuint tex = 1;
    for (auto &texture : this->config["textures"]) {
      mShader.setUniform(texture, tex);
      tex++;
    }

    std::cout << "TEST 2" << std::endl;
    for (auto& [key, value] : this->config["parameters"].items()) {
      std::cout << key << " : " << value << std::endl;
      mShader.setUniform(key, (float) value["val"]);
    }
    std::cout << "TEST 3" << std::endl;
  }

  void setUniform(const std::string uniform, float value) {
    std::cout << "TEST 4" << std::endl;
    mShader.bind();
    mShader.setUniform(uniform, value);
  }

  void activate(float time) 
  {
    mShader.bind();      
    mShader.setUniform("time", time); 
  }

  void draw()
  {
    mShader.drawIndexed(GL_TRIANGLES, 0, 2);
  }

private:
  json config;
  std::string shaderPath;
  
  nanogui::GLShader mShader;

  class UniformSlider : public nanogui::Widget {
    public: 
    UniformSlider(ShaderWidget *parent, 
		  const std::string &name, 
		  std::pair<float,float> range, 
		  float value)
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

      slider->setCallback([parent, name, textBox](float value){
	    parent->setUniform(name, value);
	    textBox->setValue(std::to_string(value));
	});
    }
  };
};
