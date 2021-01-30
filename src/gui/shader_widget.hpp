#include <nanogui/window.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

/**
 *  A widget wrapping the actual shader class, 
 *  Exposes shader uniforms to the user
 **/
class ShaderWidget : public nanogui::Window {

public:
  ShaderWidget(nanogui::Widget *parent, const std::string &config);

  void bind(float time)
  {
    mShader.bind();      
    mShader.setUniform("time", time); 
  }

  void draw()
  {
    mShader.drawIndexed(GL_TRIANGLES, 0, 2);
  }

private:
  void loadShader();
  void setUniform(const std::string uniform, float value);

  json config;
  std::string shaderPath;
  
  nanogui::GLShader mShader;

  class UniformSlider : public nanogui::Widget {
    public: 
    UniformSlider(ShaderWidget *shader,
		  nanogui::Widget *parent, 
		  const std::string &name, 
		  std::pair<float,float> range, float value);
  };
};
