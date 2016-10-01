#version 120
#extension GL_EXT_gpu_shader4 : enable

/*varying vec3 VertexPosition;*/
varying vec3 Position;
varying vec2 Wrap;
/*varying vec2 Normal;*/

uniform sampler2D Texture;
uniform vec3 Brightness;
uniform vec4 MaterialDiffuse;

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (M_PI * 2.0)

bool floatcmp(float a, float b, float e)
{
    return abs(a-b) < e;
}

bool colorcmp(vec4 a, vec4 b, float t)
{
    return floatcmp(a.r,b.r,t) &&
        floatcmp(a.g,b.g,t) &&
        floatcmp(a.b,b.b,t);
}

vec4 grayscale(vec4 c)
{
    float v = (c.r + c.g + c.b) / 3.0;
    return vec4(v,v,v, c.a);
}

float avg(vec3 c)
{
    return (c.r + c.g + c.b) / 3.0;
}

vec4 lookup(float c)
{    
    if(c < 1.0/7.0)
        return vec4(0.05, 0.33, 0.05, 1.0);
    else if(c < 3.0/7.0)
        return vec4(0.05, 0.55, 0.05, 1.0);
    else if(c < 5.0/7.0)
        return vec4(0.5, 0.66, 0.05, 1.0);
    return vec4(0.5, 1.0, 0.05, 1.0);
}

vec4 cel(float c)
{
    if(int(c * 14.0) % 2 == 1)
    {
        vec4 darker = lookup(c - 1.0/7.0);
        vec4 lighter = lookup(c + 1.0/7.0);
        if(int((Position.x/2.0+0.5)*160) % 2 == 0 && int((Position.y/2.0+0.5)*144) % 2 == 0)
            return (int(c * 7.0) % 2 == 1) ? darker : lighter;
        else
            return (int(c * 7.0) % 2 == 1) ? lighter : darker;
    }
    
    if(int(c * 7.0) % 2 == 1)
    {
        if(int((Position.x/2.0+0.5)*160) % 2 == int((Position.y/2.0+0.5)*144) % 2)
            return lookup(c - 1.0/7.0);
        else
            return lookup(c + 1.0/7.0);
    }
    return lookup(c);
}

void main()
{
    vec4 color = texture2D(Texture, Wrap);
    float e = 0.1; // threshold
    if(floatcmp(color.r, 1.0, e) &&
        floatcmp(color.g, 0.0, e) &&
        floatcmp(color.b, 1.0, e))
    {
        discard;
    }
    if(floatcmp(color.a, 0.0, e)) {
        discard;
    }
    
    float c = (color.r + color.g + color.b)/3.0;
    gl_FragColor = cel(c);
}

