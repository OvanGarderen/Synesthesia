#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D textureMap;
uniform sampler2D distanceMap;
uniform sampler2D shepardsMap;
uniform float time;

uniform float distort;

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

float layerphase(float time, float offset, vec4 dist) {
  float speed = -0.5;
  return mod(time * speed + offset + dist.z, 2.0);
}

float effectStren(float time, float radius) {
  return min(1.0 - 0.2 * radius, max(0.0,0.2 + 2.0));
}

vec2 layerpos(float phase, vec2 m, vec4 dist, float time) {
  float radius = max(m.x * m.x, m.y * m.y);
  return (mix(1.0 + effectStren(time, radius) * 0.6 * min(phase, 2.0), 1.0, dist.z)) * m; 
}

float layeralpha(float phase, vec2 pos, float time) {
  float radius = max(pos.x * pos.x, pos.y * pos.y);
  float alphaAdjust = pow(sin(M_PI/2.0 * phase),2.0) * effectStren(time, radius);

  return alphaAdjust;
}

float getmix(vec2 pos, float time, vec4 dist, vec4 shep) {
      float d = length(dist.z);
      float a = atan(pos.x,pos.y)/acos(0.); //atan(dist.x, dist.y)/acos(0.);

      return 0.7 + 0.3 * cos(3.0 * time + 8.0 * d); //+ 0.05 * smoothstep(0.0, 1.0, sin(4.0*time + M_PI*a + 30 * d));

/*
vec4 col2 = mix(col1, vec4(0.3,1.0,0.6,1.0), min(1.0,max(0.0,(0.5 + 0.5 *hsv.z)*3.5 * triangle(-4.0*(time + 0.1) + 2.0 * log(0.1+dist.x) + scale * cos(-time) * log(0.1+abs(mx)) + scale * sin(-time)*log(0.1+abs(my)))-0.9)));
*/
}

void main()
{  
  vec4 dist = texture(distanceMap, TexCoord);
  vec4 shep = texture(shepardsMap, TexCoord);

  vec2 m = TexCoord * 2 - 1.0;
  float scale = 0.8;

  //vec3 col = .7 * (0.5 + 0.5 * sqrt(length(m))) * texture(textureMap, TexCoord).xyz;
  //vec3 hsv = rgb2hsv(col);

  float mixr = getmix(m, time, dist, shep);
  //  vec4 col1 = vec4(col, 1.0);
  //  FragColor = col1;

  vec4 col = vec4(0.0,0.0,0.0,0.0);
  float totalalpha = 0.0;

  int n = 4;
  for(int i=0; i < n; i++) {
    float phase_ = layerphase(time, (2.0 * i)/n, dist);
    vec2 pos_ = layerpos(phase_, m, dist, time);
    float alpha_ = layeralpha(phase_, pos_, time);
    
    vec4 dist_ = texture(distanceMap, .5 * (pos_ + 1.0));
    mixr = getmix(pos_, time, dist_, shep);
    col += mixr * min(1.0 - totalalpha, alpha_) * texture(textureMap, .5 * vec2(1.0 + pos_.x, 1.0 + pos_.y));
    totalalpha = min(1.0, totalalpha + alpha_);
  }
  
  vec2 posmax = layerpos(2.0, m, dist, time);
  float radius = max(m.x * m.x, m.y * m.y) * distort;

  mixr = getmix(m, time, dist, shep);
  FragColor = mix(texture(textureMap, .5 * (vec2(1.0,1.0) + m)) * mixr,
  (1.0 - totalalpha) * mix(
  texture(textureMap, .5 * (vec2(1.0,1.0) + posmax * distort)),
  texture(textureMap, .5 * (vec2(1.0,1.0) + m)), radius * distort) + col, 1.0 - radius);

}
