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

  angle = (4 * M_PI/n) * triangle(phase + n * (angle + M_PI) / (4 * M_PI)) -  n * (4 * M_PI/n)* phase;

  return vec2(dist * cos(angle)/2 + .5, dist * sin(angle)/2 + .5);
}

vec2 affineSymmtex(float hrepeat, float vrepeat, vec2 pos)
{
  return vec2(triangle(-hrepeat * (pos.x + 1.0)/2 ), 
	      2 * triangle(vrepeat * (pos.y + 1.0)/4 + .5) + 1.0);
}

int modi(int x, int y) {
    return x - y * (x / y);
}
 
int and(int a, int b) {
    int result = 0;
    int n = 1;
    const int BIT_COUNT = 32;
 
    for(int i = 0; i < BIT_COUNT; i++) {
        if ((modi(a, 2) == 1) && (modi(b, 2) == 1)) {
            result += n;
        }
 
        a >>= 1;
        b >>= 1;
        n <<= 1;
 
        if (!(a > 0 && b > 0))
            break;
    }
    return result;
}
 

vec4 vibrance(vec4 inCol, float vibrance) //r,g,b 0.0 to 1.0,  vibrance 1.0 no change, 0.0 image B&W.
{
	vec4 outCol;
    if (vibrance <= 1.0)
    {
        float avg = dot(inCol.rgb, vec3(0.3, 0.6, 0.1));
        outCol.rgb = mix(vec3(avg), inCol.rgb, vibrance); 
    }
    else // vibrance > 1.0
    {
        float hue_a, a, f, p1, p2, p3, i, h, s, v, amt, _max, _min, dlt;
        float br1, br2, br3, br4, br5, br2_or_br1, br3_or_br1, br4_or_br1, br5_or_br1;
        int use;
 
        _min = min(min(inCol.r, inCol.g), inCol.b);
        _max = max(max(inCol.r, inCol.g), inCol.b);
        dlt = _max - _min + 0.00001 /*Hack to fix divide zero infinities*/;
        h = 0.0;
        v = _max;
 
	br1 = step(_max, 0.0);
        s = (dlt / _max) * (1.0 - br1);
        h = -1.0 * br1;
 
	br2 = 1.0 - step(_max - inCol.r, 0.0); 
        br2_or_br1 = max(br2, br1);
        h = ((inCol.g - inCol.b) / dlt) * (1.0 - br2_or_br1) + (h*br2_or_br1);
 
	br3 = 1.0 - step(_max - inCol.g, 0.0); 
        
        br3_or_br1 = max(br3, br1);
        h = (2.0 + (inCol.b - inCol.r) / dlt) * (1.0 - br3_or_br1) + (h*br3_or_br1);
 
        br4 = 1.0 - br2*br3;
        br4_or_br1 = max(br4, br1);
        h = (4.0 + (inCol.r - inCol.g) / dlt) * (1.0 - br4_or_br1) + (h*br4_or_br1);
 
        h = h*(1.0 - br1);
 
        hue_a = abs(h); // between h of -1 and 1 are skin tones
        a = dlt;      // Reducing enhancements on small rgb differences
 
        // Reduce the enhancements on skin tones.    
        a = step(1.0, hue_a) * a * (hue_a * 0.67 + 0.33) + step(hue_a, 1.0) * a;                                    
        a *= (vibrance - 1.0);
        s = (1.0 - a) * s + a * pow(s, 0.25);
 
        i = floor(h);
        f = h - i;
 
        p1 = v * (1.0 - s);
        p2 = v * (1.0 - (s * f));
        p3 = v * (1.0 - (s * (1.0 - f)));
 
        inCol.rgb = vec3(0.0); 
        i += 6.0;
        //use = 1 << ((int)i % 6);
        use = int(pow(2.0,mod(i,6.0)));
        a = float(and(use , 1)); // i == 0;
        use >>= 1;
        inCol.rgb += a * vec3(v, p3, p1);
 
        a = float(and(use , 1)); // i == 1;
        use >>= 1;
        inCol.rgb += a * vec3(p2, v, p1); 
 
        a = float( and(use,1)); // i == 2;
        use >>= 1;
        inCol.rgb += a * vec3(p1, v, p3);
 
        a = float(and(use, 1)); // i == 3;
        use >>= 1;
        inCol.rgb += a * vec3(p1, p2, v);
 
        a = float(and(use, 1)); // i == 4;
        use >>= 1;
        inCol.rgb += a * vec3(p3, p1, v);
 
        a = float(and(use, 1)); // i == 5;
        use >>= 1;
        inCol.rgb += a * vec3(v, p1, p2);
 
        outCol = inCol;
    }
    return outCol;
}

void brightnessAdjust( inout vec4 color, in float b) {
    color.rgb += b;
}
 
vec4 shiftHue(in vec3 col, in float Shift)
{
    vec3 P = vec3(0.55735) * dot(vec3(0.55735), col);
    vec3 U = col - P;
    vec3 V = cross(vec3(0.55735), U);    
    col = U * cos(Shift * 6.2832) + V * sin(Shift * 6.2832) + P;
    return vec4(col, 1.0);
}

void main()
{  
  float mx = TexCoord.x * 2 - 1.0;
  float my = TexCoord.y * 2 - 1.0;
  //vec2 pos = affineSymmtex(1,1,vec2(mx,my));
  //vec2 posT = vec2(pos.x + 1.0, pos.y + 1.0);
  float dist = max(abs(mx), abs(my)); //pow(mx, 2.0) + pow(my, 2.0);

  // apply a distortion
  vec2 disp = vec2(0.0, 0.0);
  vec2 disp2 = vec2(0.0, 0.0);
  vec2 disp3 = vec2(0.0, 0.0);
  //disp += .0005 * cos(1.5 + 3 * time + 40 * dist) * (vec2(valr.y - .5, valr.z - .5));	

  float calcAmp = 0.001 * ampDistort * (1.0 + sin(wobbleAngDistort) * wobbleDistort * my * my + cos(wobbleAngDistort) * wobbleDistort * mx * mx);

  // calculate distortion
  for(int k=0; k < 10; k++)
  {
    vec4 valdist = texture(distanceMap, TexCoord + disp);
    vec4 flow    = texture(shepardsMap, TexCoord + disp);
    vec4 flow2   = texture(shepardsMap, TexCoord + disp2);
    vec4 flow3   = texture(shepardsMap, TexCoord + disp3);

    float phase  = freqDistort * time + edgePhaseDistort * valdist.z + distxPhaseDistort * my * my + distyPhaseDistort * mx * mx;
    disp        += calcAmp * cos(phase) * vec2(flow.y - .5, flow.x - .5);
    disp2       += calcAmp * cos(phase + 2 * M_PI/3) * vec2(flow2.y - .5, flow2.x - .5);
    disp3       += calcAmp * cos(phase + 4 * M_PI/3) * vec2(flow3.y - .5, flow3.x - .5);
  }  

  //vec4 valr = texture(distanceMap, posT + disp);
  vec4 valr2 = texture(distanceMap, TexCoord + disp);

  //float i = pow(valr2.z + 2, 2.0);
  //float j = pow(valr.z + 2, 2.0);

  float i = pow(valr2.z + 2, 2.0);
  float phase = .5 + clamp(valr2.x,0,1) * ampSat * sin(freqSat * time + (30/wlenSat) * i + valr2.z);
  
  vec4 mix1 = mix(texture(textureMap, TexCoord + disp2),
         texture(textureMap, TexCoord + disp2), 
         0.5 + ampSat * sin(freqSat * time + (5/wlenSat) * dist));	
  vec4 mix2 = mix(texture(textureMap, TexCoord + disp3),
         texture(textureMap, TexCoord + disp3), 
         0.5 + ampSat * sin(1.5 * freqSat * time + M_PI/2 + (5/wlenSat) * dist));	

  vec4 frag = mix(mix(texture(textureMap, TexCoord + disp), mix1, phase), mix2, 0.15 + 0.8 * phase);

  vec3 hsv = rgb2hsv(frag.xyz);

  // new hsv values
  float hue = ampHue * triangle(freqHue * time + (5/wlenHue) * (valr2.z - 0.1 * dist));
  //float sat = hsv.y + offSat;
  float val = offVal + ampVal * triangle(freqVal * time + .5 + (5/wlenVal) * (valr2.z + 0.1 * dist));
  
  
  //  float phase = .5 + clamp(2.0 * valr2.x,0,1) * ampHue * sin(freqHue * time + (30/wlenVal) * i);
  //float phase2 = .5 + ampHue * sin(freqHue * time + (30/wlenVal) * i +  M_PI/2);
  //fragn = mix(mix(fragn, diln, clamp(.5 - phase2, 0.0, 1.0)), eron, clamp(phase2-.5,0.0,1.0));
  //frag = mix(mix(frag, dil, clamp(.5 - phase2, 0.0, 1.0)), ero, clamp(phase2-.5,0.0,1.0));
  
  vec4 vibrant = vibrance(frag, 1.0 + exp(10.0 * offSat));
  vec4 hues = shiftHue(vibrant.xyz, hue);
  brightnessAdjust(hues, val);

  FragColor = hues;  //hsv2rgb(vec3(hue, sat, val)),1.0);//mix(fragn, frag, 2.0 * phase);   
}
