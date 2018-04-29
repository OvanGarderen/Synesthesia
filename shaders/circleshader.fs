#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform float baseRot;

void main()
{
  vec2 coord = vec2(2.0 * (TexCoord.x - .5), 2.0 *(TexCoord.y - .5));
  float radius = sqrt(coord.x * coord.x + coord.y * coord.y);
  float angle = atan(coord.y, coord.x);
  float mult = 1.0 + .01 * sin(50.0 * angle);// * (1.5 + sin(8 * baseRot)) - .3 * cos(baseRot);

  vec3 color = vec3(1.0,1.0,1.0);

  float uniformstep = 1.0/18.0 * (
    step(.0001, mult*radius) + step(.0005, mult*radius) + step(.001, mult*radius) + step(.002, mult*radius) +
    step(.035, mult*radius) + step(.04, mult*radius) + step(.07, mult*radius) + step(.1, mult*radius) +
    step(.16, mult*radius) + step(.24, mult*radius) + step(.3, mult*radius) + step(.35, mult*radius) +
    step(.4, mult*radius) + step(.5, mult*radius) +
    step(.6, mult*radius) + step(.8, mult*radius) + step(1.0, mult*radius) + step(1.2, mult*radius));

  float texmult = 2.0 * (1.0 - 0.8 * uniformstep);
  float w = 2.5 * (1.0 - uniformstep);

  mat2 rot = mat2( cos(w), sin(w),
		   -sin(w), cos(w));
  coord = rot * coord;
  
  FragColor = texture(ourTexture, vec2(.5 * texmult * coord.x + .5, .5 * texmult * coord.y + .5)) * vec4(color,1.0);
}
