#version 450 core
out vec4 FragColor;

in vec2 TexCoords;


uniform sampler2D textureDiffuse1;
uniform vec2 screenSize;

int kernelNum = 12;
vec2 g_v2TwelveKernelBase[] =
{
    {0.0,1.0},{0.0,0.0},{1.0,-1.0},
    {0.0,-1.0},{0.0,0.0},{-1.0,1.0},
    {0.0,1.0},{1.0,0.0},{1.0,1.0},
    {0.0,-1.0},{-1.0,0.0},{-1.0,-1.0},
};

void main()
{    
    vec4 v4Original = texture2D(textureDiffuse1, TexCoords);
 
    vec2 v4ScreenSize = screenSize / 5;
    vec3 v3Blurred = vec3(0, 0, 0);
    for(int i = 0; i < kernelNum; i++)
    {
       vec2 v2Offset = vec2(g_v2TwelveKernelBase[i].x / v4ScreenSize.x,
       g_v2TwelveKernelBase[i].y / v4ScreenSize.y);
       vec4 v4Current = texture2D(textureDiffuse1, TexCoords + v2Offset);
       v3Blurred += mix(v4Original.rgb, v4Current.rgb, v4Original.a);
   }
   FragColor = vec4 (v3Blurred / kernelNum , 1.0f);

    
    //FragColor = texture(textureDiffuse1, TexCoords);
}