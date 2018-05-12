#version 330 core
#define NUM_DEFORMS 100
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform sampler2D distanceMap;
uniform sampler2D shepardsMap;
uniform float param;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float triangle(float inp)
{
  return abs(mod(inp, 1.0) - .5);
}

void main()
{  
  float distortion = .01;

  float mx = TexCoord.x * 2 - 1.0;
  float my = TexCoord.y * 2 - 1.0;
  float dist = pow(mx, 2.0) + pow(my, 2.0);

  vec4 valdist = texture(distanceMap, TexCoord);
  // apply a distortion
  vec2 disp = vec2(0,0);
  float stretching = 0.0;

  for(int k=0; k < 20; k++)
  {
    vec4 val = texture(shepardsMap, TexCoord + disp);
    vec4 valr = texture(distanceMap, TexCoord + disp);
//    disp += .002 * cos(1.5 + 3 * param + 40 * dist) * (vec2(valr.y - .5, valr.z - .5));
    stretching += .65 - val.x * val.x - val.y * val.y;
    disp += .001 * cos(2.5 * param + 10 * valdist.x + 10 * (valr.z * my * my + valr.y * mx * mx)) * vec2(val.y - .55, val.x - .5);
  }  

  vec4 valr = .3 * texture(distanceMap, TexCoord + disp);
  float dir = atan(valr.z - .5, valr.y -.5);

  float i = (pow(3*valr.x + 2, 2.0) - 0 * clamp(dist * valr.x,0.02,6.8) - param + dir/3.1415);
  float j = (pow(3*valr.x + 2, 2.0) - 0 * clamp(dist * valr.x,0.02,6.8) - param);


  vec4 val = texture(shepardsMap, TexCoord + disp);
  vec4 frag = texture(ourTexture, TexCoord + disp);
  vec3 hsv = rgb2hsv(vec3(frag.x,frag.y, frag.z));

  vec3 blendcol = hsv2rgb(vec3(
       hsv.x + .1 * sin(1.4 * i) + .1 * sin(2.4 * i),  
       hsv.y + .15 * triangle(2 * i), 
       hsv.z + .2 * (1.0 - 2* valr.x) * triangle(.5+ 5 * j + 3.5 * param)));

  FragColor = vec4(blendcol,clamp(valr.x,0.0,.8));
}
