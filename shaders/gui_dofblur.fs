#version 330 core

out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D textureMap;
uniform sampler2D distanceMap;
uniform sampler2D shepardsMap;

uniform float time;

float triangle(float inp)
{
  return abs(mod(inp, 1.0) - .5);
}

void main()
{  
  vec4 frag = texture(textureMap, TexCoord);
  float yval = 1.0 - abs(2.0 * (TexCoord.y - .5));
  float dist = min(max(0.0, texture(distanceMap, TexCoord).z - yval),1.0);
  float dist2 = min(max(0.0, texture(shepardsMap, TexCoord).z),1.0);

  vec3 tex = frag.xyz;

/*
  for (int i = -3; i <= 3; i++) {
      for (int j = -3; j <= 3; j++) {
	vec3 col = vec3(0.5 * i, 0.0, 0.0);	
	vec3 loc = texture(textureMap, TexCoord + vec2(0.002 * i, 0.002 * j)).xyz;
	tex += dist/(7.0*7.0) * (col + loc);
      }
  }
*/

  float l = max(frag.x, max(frag.y, frag.z));
  float len =  0.5 * length(TexCoord);
  float phase1 = triangle(1.5 * time + 0.3 + len + 3 * texture(distanceMap, TexCoord).z);
  float phase2 = triangle(1.0 * time + 0.1 + len + 3 * texture(distanceMap, TexCoord).z);
  float phase3 = triangle(-1.0 * time +  len + 3 * texture(distanceMap, TexCoord).z);
  FragColor = vec4(mix(tex, vec3(phase1, phase2, phase3), 0.8 * dist2),1.0);// * TexCoord.y * l * (1.0 - dist)), 1.0);
}
