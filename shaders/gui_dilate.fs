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

#define M_PI 3.1415926535897932384626433832795

vec2 radialSymmtex(int n, float phase, vec2 pos)
{
  float angle = atan(pos.y, pos.x);
  float dist = length(pos);

  angle = (4 * M_PI/n) * triangle(phase + n * (angle + M_PI) / (4 * M_PI)) -  n * (2 * M_PI/n)* phase;

  return vec2(dist * cos(angle)/2 + .5, dist * sin(angle)/2 + .5);
}

vec2 affineSymmtex(float hrepeat, float vrepeat, vec2 pos)
{
  return vec2(4 * triangle(hrepeat * (pos.x + 1.0)/4 + .5) - 1.0, 
	      4 * triangle(vrepeat * (pos.y + 1.0)/4 + .5) - 1.0);
}

void main()
{  
  float mx = TexCoord.x * 2 - 1.0;
  float my = TexCoord.y * 2 - 1.0;
  //vec2 pos = radialSymmtex(16, .05 * time, vec2(mx,my));
  //  mx = pos.x;
  //  my = pos.y;
  vec2 posT = vec2(mx/2 + .5, my/2 + .5);
  float dist = pow(mx, 2.0) + pow(my, 2.0);

  // apply a distortion
  vec2 disp = vec2(0.0, 0.0);

  vec4 valr = texture(distanceMap, posT);
  float angle = atan(valr.z - .5, valr.y -.5);

  float i = (pow(valr.x + 2, 2.0) + angle/3.1415);
  float j = (pow(valr.x + 2, 2.0) );

  vec4 frag = texture(textureMap, posT);
  vec3 hsv = rgb2hsv(vec3(frag.x,frag.y, frag.z));


  // determine new hsv values
  float maxsat = frag.r * .3 + frag.g * .59 + frag.b * .11;
  float minsat = frag.r * .3 + frag.g * .59 + frag.b * .11;
  vec3 pixelmax = frag.xyz;
  vec3 pixelmin = frag.xyz;
  for (int a = -2; a <= 2; a+= 1) {
    for (int b = -2; b <= 2; b+= 1) {
      vec3 thispix = texture(textureMap, posT + vec2(.001 * ampDistort * a,.001 * ampDistort * b)).xyz;
      
      if (thispix.r * .3 + thispix.g * .59 + thispix.b * .11 > maxsat) {
	pixelmax = thispix;
	maxsat = thispix.r * .3 + thispix.g * .59 + thispix.b * .11;
      }	      
      if (thispix.r * .3 + thispix.g * .59 + thispix.b * .11 < minsat) {
	pixelmin = thispix;
	minsat = thispix.r * .3 + thispix.g * .59 + thispix.b * .11;
      }
      
    }
  } 

  float mixx = sin(5 * freqDistort * time + 5 * wobbleDistort * i);
  FragColor = vec4(mix(mix(pixelmax, frag.xyz, max(0.0,mixx)), pixelmin, -min(0.0,mixx)),1.0);
}
