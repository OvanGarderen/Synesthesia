#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D textureMap;
uniform sampler2D distanceMap;
uniform sampler2D shepardsMap;
uniform sampler2D mixin1Map;
uniform sampler2D mixin2Map;
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

#define M_PI 3.1415926535897932384626433832795

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

#define quality 2.0

vec4 low_pass(in sampler2D tex, in vec2 pos, in float scale) {
  // use a 5x5 Gaussian kernel
  vec4 col = texture(tex, pos);

  for(float d = 0.0; d < M_PI; d += M_PI/8.0) {
    for(float i = .5; i <= 1.0; i += 0.5) {      
      col += texture(tex, pos + vec2(cos(d),sin(d)) * scale * i);
    }
  }

  return col;
}

void main()
{  
  // inputs
  vec4 frag = texture(textureMap, TexCoord);
  vec4 fragBlur = low_pass(textureMap, TexCoord, 0.1);
  vec4 depth = texture(distanceMap, TexCoord);

  // absolute distance from the centre
  float mx = TexCoord.x * 2 - 1.0;
  float my = TexCoord.y * 2 - 1.0;
  float dist = max(abs(mx), abs(my));

  // get real color info
  vec3 hsv = rgb2hsv(fragBlur.xyz);

  // new hsv values
  float hue = hsv.x + ampHue * triangle(freqHue * time + (5/wlenHue) * (depth.z - 0.3 * dist));
  float val = offVal + ampVal * triangle(freqVal * time + .5 + (5/wlenVal) * (depth.z - 0.3 * dist));
  float alpha = offSat + ampSat * triangle(freqSat * time + .5 + (5/wlenSat) * (depth.z - 0.3 * dist));   

  FragColor = vec4(hsv2rgb(vec3(hue, 1.0 - .3 * hsv.y, val)), alpha);
}
