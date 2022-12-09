#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

#define PCF_SQUARESAMPLE 1
#define PCF_POISSONSAMPLE 2
#define PCSS_POISSONSAMPLE 3

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform int shadowType;//见上面的宏定义

uniform vec3 lightPos;
uniform float lightWidth;//其实是平行光。。。
uniform vec3 viewPos;


//泊松圆盘随机采样
#define NUM_RINGS 5 //环数
#define NUM_SAMPLES 50  //采样数
#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

highp float rand_1to1(highp float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}

highp float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples( const in vec2 randomSeed ) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = rand_2to1( randomSeed ) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}


float PCFShadow_poissonDisk(vec4 fragPosLightSpace,vec3 normal,vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
     if(projCoords.z>1.0)
        return 0.0;
    //泊松圆盘采样
    poissonDiskSamples(projCoords.xy);
    //shadow map大小
    float texSize=textureSize(shadowMap,0).x;
    //滤波的步长
    float filterStride = 5.0;
    // 滤波窗口的范围
    float filterRange = 1.0 / texSize * filterStride;
    // 当前深度
    float currentDepth = projCoords.z;

    //add bias to solve shadow acne(与采样方法也有关)
    float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.01); 

    float shadow =0.0;
    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        vec2 sampleCoord = poissonDisk[i] * filterRange + projCoords.xy;

        float pcfDepth = texture(shadowMap, sampleCoord).r; 

        shadow += (currentDepth-bias > pcfDepth ) ? 1.0 : 0.0;
    }
    shadow/=float(NUM_SAMPLES);

    
    return shadow;

}

//找到遮挡的深度【传入的是经透视除法的坐标！】
float getBlockDepth(vec3 projCoords,vec3 normal,vec3 lightDir)
{
    int blockCnt=0;
    float totalDepth= 0.0;
    // 当前深度
    float currentDepth = projCoords.z;


    // 滤波窗口的范围
    float filterStride= 50.0;
    

    //shadow map大小
    float texSize=textureSize(shadowMap,0).x;
    // 滤波窗口的范围
    float filterRange = 1.0 / texSize * filterStride;

    //泊松圆盘采样
    poissonDiskSamples(projCoords.xy);

    //add bias to solve shadow acne(与采样方法也有关)
    float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.01); 
    
    for(int i=0;i<NUM_SAMPLES;i++)
    {
        vec2 sampleCoord = poissonDisk[i] * filterRange + projCoords.xy;
        float shadowDepth=texture(shadowMap,sampleCoord).r;
        if(currentDepth-bias >shadowDepth)
        {
            blockCnt++;
            totalDepth+=shadowDepth;
        }
    }

    if(blockCnt==0 )
        return 1.0;
    else
        return totalDepth/float(blockCnt);

}

float PCSSShadow_poissonDisk(vec4 fragPosLightSpace,vec3 normal,vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z>1.0)
        return 0.0;

    float blockDepth=getBlockDepth(projCoords,normal,lightDir);
    float recDepth=projCoords.z;

    //半影区大小
    float penumWidth=lightWidth*(recDepth-blockDepth)/blockDepth;


    //下边就和pcf基本一样，除了采样在求遮挡时已做和多乘半影
    //shadow map大小
    float texSize=textureSize(shadowMap,0).x;
    //滤波的步长
    float filterStride = 20.0;
    // 滤波窗口的范围
    float filterRange = 1.0 / texSize * filterStride;
    // 当前深度
    float currentDepth = projCoords.z;

    //add bias to solve shadow acne(与采样方法也有关)
    float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.01); 

    float shadow =0.0;
    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        //注意这里多出来乘半影范围
        vec2 sampleCoord = penumWidth* poissonDisk[i] * filterRange + projCoords.xy;
        float pcfDepth = texture(shadowMap, sampleCoord).r; 

        shadow += (currentDepth-bias > pcfDepth ) ? 1.0 : 0.0;
    }
    shadow/=float(NUM_SAMPLES);

    
    return shadow;


}


float ShadowCalculation(vec4 fragPosLightSpace,vec3 normal,vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z>1.0)
        return 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    //float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;


    //add bias to solve shadow acne
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 

    // check whether current frag pos is in shadow （不在光源视锥内的只是照不到）

    // PCF 
    float shadow =0.0;
    vec2 texelSize=1.0/textureSize(shadowMap,0);
    for(int x=-2 ;x<=2;x++)
    {
        for(int y=-2 ;y<=2;y++)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            //shadow += (currentDepth-bias > pcfDepth && projCoords.z<=1.0) ? 1.0 : 0.0;
            shadow += (currentDepth-bias > pcfDepth ) ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;//float(NUM_SAMPLES);


    //float shadow = (currentDepth-bias > closestDepth && projCoords.z<=1.0) ? 1.0 : 0.0;

    return shadow;
}






void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(1.0);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow=0.0f;
    if(shadowType == PCF_SQUARESAMPLE)
        shadow = ShadowCalculation(fs_in.FragPosLightSpace,normal,lightDir);                      
    else if(shadowType == PCF_POISSONSAMPLE)
        shadow =PCFShadow_poissonDisk(fs_in.FragPosLightSpace,normal,lightDir);   
    else if(shadowType == PCSS_POISSONSAMPLE)
        shadow =PCSSShadow_poissonDisk(fs_in.FragPosLightSpace,normal,lightDir);   

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}