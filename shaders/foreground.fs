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

void main()
{  
  vec4 dmap = texture(distanceMap, TexCoord);

  float mx = TexCoord.x * 2 - 1.0;
  float my = TexCoord.y * 2 - 1.0;
  float dist = max(abs(mx),abs(my));
  float depth = smoothstep(.2, .3, 1.0 - dmap.r);

  FragColor = mix(texture(textureMap, TexCoord), vec4(0.0,0.0,0.0,1.0), 1.0 - depth);
}
