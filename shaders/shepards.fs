#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform float param;

struct TargetPoint {
  float x;
  float y;
  float dx;
  float dy;
};
uniform TargetPoint points[40];

void main()
{  
  vec2 coord = vec2(2.0 * (TexCoord.x - .5), 2.0 *(TexCoord.y - .5));
  vec2 disp = vec2(0.0, 0.0);

  vec2 weigh = vec2(0.0, 0.0);

  float total = 0.0;
  float dist = 0.0;
  for (int i = 0; i < 40; i++) {    
    dist = (points[i].x - coord.x) * (points[i].x - coord.x) + (points[i].y - coord.y) * (points[i].y - coord.y);    
    
    weigh.x += points[i].dx / dist;
    weigh.y += points[i].dy / dist;
    total += 1.0 / dist;
  }
  float wt = weigh.x * weigh.x + weigh.y * weigh.y;
  
  FragColor = //vec4(weigh.x/total, weigh.y/total, 0.0, 1.0);
    texture(ourTexture, vec2(.5 *(coord.x + param * wt * weigh.x/total) + .5, .5 *(coord.y + param * wt * weigh.y/total) + .5));
}
