#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform float frameRatioX;
uniform float frameRatioY;
uniform sampler2D frame;

void main()
{
  vec2 Normcoord;

  if (frameRatioX > frameRatioY) {
    Normcoord = vec2(TexCoord.x, -frameRatioX/frameRatioY * TexCoord.y);

    float bound = 1.0 - frameRatioY/frameRatioX;
    if (TexCoord.y > bound/2 && TexCoord.y < 1.0 - bound/2) {
      FragColor = texture(frame, Normcoord + vec2(0.0, frameRatioX/frameRatioY * bound/2));
    } else {
      FragColor = vec4(0.2, 0.3, 0.4, 1.0);
    }
  } else {
    Normcoord = vec2(frameRatioY/frameRatioX * TexCoord.x, -TexCoord.y);

    float bound = 1.0 - frameRatioX/frameRatioY;
    if (TexCoord.x > bound/2 && TexCoord.x < 1.0 - bound/2) {
      FragColor = texture(frame, Normcoord - vec2(frameRatioY/frameRatioX * bound/2,0.0));
    } else {
      FragColor = vec4(0.2, 0.3, 0.4, 1.0);
    }
  }
  
}
