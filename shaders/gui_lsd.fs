#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D textureMap;
uniform sampler2D distanceMap;
uniform sampler2D shepardsMap;
uniform float time;

uniform float ampDistort;
uniform float freqDistort;
uniform float wobbleDistort;
uniform float wobbleAngDistort;
uniform float edgePhaseDistort;
uniform float distxPhaseDistort;
uniform float distyPhaseDistort;

uniform float freqHue;
uniform float wlenHue;
uniform float ampHue;
uniform float offHue;

uniform float freqSat;
uniform float wlenSat;
uniform float ampSat;
uniform float offSat;

uniform float freqVal;
uniform float wlenVal;
uniform float ampVal;
uniform float offVal;

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
    vec3 p = abs(fract(clamp(c.xxx,0.0,1.0) + K.xyz) * 6.0 - K.www);
    return clamp(c.z,0.0,1.0) * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), clamp(c.y,0.0,1.0));
}

float triangle(float inp)
{
  return abs(mod(inp, 1.0) - .5);
}

void main()
{  
  float mx = TexCoord.x * 2 - 1.0;
  float my = TexCoord.y * 2 - 1.0;
  float dist = pow(mx, 2.0) + pow(my, 2.0);

  // apply a distortion
  vec2 disp = vec2(0.0, 0.0);
  //disp += .0005 * cos(1.5 + 3 * time + 40 * dist) * (vec2(valr.y - .5, valr.z - .5));	

  // calculate distortion
  for(int k=0; k < 20; k++)
  {
    vec4 valdist = texture(distanceMap, TexCoord + disp);
    vec4 flow    = texture(shepardsMap, TexCoord + disp);
    float phase  = edgePhaseDistort * valdist.x + distxPhaseDistort * valdist.z * my * my + distyPhaseDistort * valdist.y * mx * mx;
    disp        += 0.001 * ampDistort
      * (1.0 + sin(wobbleAngDistort) * wobbleDistort * my + cos(wobbleAngDistort) * wobbleDistort * mx)
      * cos(freqDistort * time + phase) 
      * vec2(flow.y - .5, flow.x - .5);
  }  

  vec4 valr = texture(distanceMap, TexCoord + disp);
  float angle = atan(valr.z - .5, valr.y -.5);
  float i = (pow(valr.x + 2, 2.0) + angle/3.1415);
  float j = (pow(valr.x + 2, 2.0) );

  vec4 frag = texture(textureMap, TexCoord + disp);
  vec3 hsv = rgb2hsv(vec3(frag.x,frag.y, frag.z));

  // determine new hsv values
  float hue = hsv.x + offHue + ampHue * sin(freqHue * time + (5 / wlenHue) * i + 3 * (mx * mx/(my*my + 1.0)) + 3 * pow(my * mx,2.0));

  float sat = hsv.y + offSat + ampSat * sin(freqSat * time + (5 / wlenSat) * j + 10 * dist);

  float val = hsv.z  + offVal + ampVal * triangle(freqVal * time + .5+ (5/wlenVal) * j);

  FragColor = vec4(hsv2rgb(vec3(hue,sat,val)),1.0);
}
