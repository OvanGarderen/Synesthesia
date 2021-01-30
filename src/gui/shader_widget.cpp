#include <iostream>
#include <fstream>

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/slider.h>
#include <nanogui/tabwidget.h>
#include <nanogui/textbox.h>

#include "shader_widget.hpp"

ShaderWidget::ShaderWidget(nanogui::Widget *parent, const std::string &cfg) 
  : nanogui::Window(parent)  
{
  std::cout << "------------------------------" << std::endl << "configuring " << config << std::endl << std::endl;

  std::ifstream configStream;
  configStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    configStream.open(cfg.c_str());
    configStream >> this->config;
    configStream.close();
  } 
  catch (std::ifstream::failure &e) {
    std::cerr << "Error reading file " << cfg << std::endl;
  }
  catch (nlohmann::json::exception &e) {
    std::cerr << "Error reading file " << cfg << " : Invalid JSON" << std::endl;
    std::cerr << e.what() << std::endl;      
  }
  
  this->setTitle(this->config["name"]);
  this->shaderPath = this->config["path"];
  
  std::cout << this->shaderPath << std::endl;

  this->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical));

  loadShader();

  auto *tabs = this->add<nanogui::TabWidget>();
  for (auto& group : this->config["parameters"]) {
    
    auto *tab = tabs->createTab(group["name"]);
    tab->setLayout(new nanogui::GroupLayout());

    for (auto& child : group["children"]) {
      
      mShader.setUniform(child["id"], (float) child["val"]);

      new nanogui::Label(tab, child["name"]);      
      new UniformSlider(this, tab, child["id"], std::make_pair(child["min"],child["max"]), child["val"]);
    }
  }

  std::cout << std::endl << "initialised " << cfg << std::endl << std::endl;
  std::cout << "------------------------------";
}

void ShaderWidget::loadShader() 
{
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

  uint tex = 1;
  for (auto &texture : this->config["textures"]) {
    mShader.setUniform(texture, tex);
    tex++;
  }
}

void ShaderWidget::setUniform(const std::string uniform, float value) {
  mShader.bind();
  mShader.setUniform(uniform, value);
}

ShaderWidget::UniformSlider::UniformSlider(ShaderWidget *shader,
					   nanogui::Widget *parent, 
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
  
  slider->setCallback([shader, name, textBox](float value){
			shader->setUniform(name, value);
			textBox->setValue(std::to_string(value));
		      });
}

