#version 330 core
//相机移动太远会出问题
//add sky box
uniform sampler3D perlworl;
uniform sampler3D worl;
uniform sampler2D curl;
uniform sampler2D weather;

uniform int check;
uniform mat4 MVPM; 
uniform float aspect;
uniform float time;
uniform vec2 resolution;
uniform float downscale;
//总体云层密度
uniform float cloud_density;
//颜色
uniform vec3 color_style;
//变化速度
uniform float speed;
uniform vec3 cameraPos ;

in vec3 vSunDirection;

in vec3 vSunColor;

out vec4 color;


const float g_radius = 200000.0; //ground radius
const float sky_b_radius = 201000.0;//bottom of cloud layer
const float sky_t_radius = 201500.0;//top of cloud layer

const float cwhiteScale = 1.2;


vec3 U2Tone(const vec3 x) {
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;

   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}



const vec3 RANDOM_VECTORS[6] = vec3[6]
(
	vec3( 0.38051305f,  0.92453449f, -0.02111345f),
	vec3(-0.50625799f, -0.03590792f, -0.86163418f),
	vec3(-0.32509218f, -0.94557439f,  0.01428793f),
	vec3( 0.09026238f, -0.27376545f,  0.95755165f),
	vec3( 0.28128598f,  0.42443639f, -0.86065785f),
	vec3(-0.16852403f,  0.14748697f,  0.97460106f)
	);

//点在云层中的位置 fractional value for sample position in the cloud layer
float GetHeightFractionForPoint(float inPosition)
{ // get global fractional position in cloud zone
	float height_fraction = (inPosition -  sky_b_radius) / (sky_t_radius - sky_b_radius); 
	return clamp(height_fraction, 0.0, 1.0);
}

vec4 mixGradients(const float cloudType){

	const vec4 STRATUS_GRADIENT = vec4(0.02f, 0.05f, 0.09f, 0.11f);
	const vec4 STRATOCUMULUS_GRADIENT = vec4(0.02f, 0.2f, 0.48f, 0.625f);
	const vec4 CUMULUS_GRADIENT = vec4(0.01f, 0.0625f, 0.78f, 1.0f); // these fractions would need to be altered if cumulonimbus are added to the same pass
	float stratus = 1.0f - clamp(cloudType * 2.0f, 0.0, 1.0);
	float stratocumulus = 1.0f - abs(cloudType - 0.5f) * 2.0f;
	float cumulus = clamp(cloudType - 0.5f, 0.0, 1.0) * 2.0f;
	return STRATUS_GRADIENT * stratus + STRATOCUMULUS_GRADIENT * stratocumulus + CUMULUS_GRADIENT * cumulus;
}

float densityHeightGradient(const float heightFrac, const float cloudType) {
	vec4 cloudGradient = mixGradients(cloudType);
	return smoothstep(cloudGradient.x, cloudGradient.y, heightFrac) - smoothstep(cloudGradient.z, cloudGradient.w, heightFrac);
}

float intersectSphere(const vec3 pos, const vec3 dir, const float r) {
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, pos);
    float c = dot(pos, pos) - (r * r);
		float d = sqrt((b*b) - 4.0*a*c);
		float p = -b - d;
		float p2 = -b + d;
    return max(p, p2)/(2.0*a);
}

//  maps a value from one range to another. 
float remap(const float originalValue, const float originalMin, const float originalMax, const float newMin, const float newMax)
{
	return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}
//云层密度计算
float density(vec3 p, vec3 weather,const bool hq,const float LOD) {
	p.x += time*speed;
	float height_fraction = GetHeightFractionForPoint(length(p));
	vec4 n = textureLod(perlworl, p*0.0003, LOD);
	float fbm = n.g*0.625+n.b*0.25+n.a*0.125;
	float g = densityHeightGradient(height_fraction, cloud_density);
	float base_cloud = remap(n.r, -(1.0-fbm), 1.0, 0.0, 1.0);
	float cloud_coverage = smoothstep(0.6, 1.3, weather.x);
	base_cloud = remap(base_cloud*g, 1.0-cloud_coverage, 1.0, 0.0, 1.0); 
	base_cloud *= cloud_coverage;
	if (hq) {
		vec2 whisp = texture(curl, p.xy*0.0003).xy;
		p.xy += whisp*400.0*(1.0-height_fraction);
		vec3 hn = texture(worl, p*0.004, LOD-2.0).xyz;
		float hfbm = hn.r*0.625+hn.g*0.25+hn.b*0.125;
		hfbm = mix(hfbm, 1.0-hfbm, clamp(height_fraction*3.0, 0.0, 1.0));
		base_cloud = remap(base_cloud, hfbm*0.2, 1.0, 0.0, 1.0);
	}
	return clamp(base_cloud, 0.0, 1.0);
}

#define Ambient  vec3(.7,.7,.7)    // 基础散射

vec4 march(const vec3 pos, const vec3 end, vec3 dir, const int depth) {
	float T = 1.0;
	float alpha = 0.0;//初始透明度
	vec3 p = pos;		//世界坐标
	float ss = length(dir);
	const float t_dist = sky_t_radius-sky_b_radius;
	float lss = t_dist/float(depth);
	vec3 ldir = vSunDirection*ss;
	vec3 L = vec3(0.0);
	int count=0;
	float t = 1.0;

	for (int i=0;i<depth;i++) {
		p += dir;
		float height_fraction = GetHeightFractionForPoint(length(p));
		const float weather_scale = 0.00008;
		vec3 weather_sample = texture(weather, p.xz*weather_scale).xyz;
		t = density(p, weather_sample, true, 0.0);
		const float ldt = 0.5;
		float dt = exp(-ldt*t*ss);
		T *= dt;		
		vec3 lp = p;
		const float ld = 0.5;
		float lt = 1.0;
		float ncd = 0.0;
		float cd = 0.0;
		if (t>0.0) { //密度不为0时才计算光照
			for (int j=0;j<6;j++) {
				lp += (ldir+(RANDOM_VECTORS[j]*float(j+1))*lss);
				vec3 lweather = texture(weather, lp.xz*weather_scale).xyz;
				lt = density(lp, lweather, false, float(j));
				cd += lt;
				ncd += (lt * (1.0-(cd*(1.0/(lss*6.0)))));
			}
			lp += ldir*12.0;
			vec3 lweather = texture(weather, lp.xz*weather_scale).xyz;
			lt = density(lp, lweather, false, 5.0);//LOD?
			cd += lt;
			ncd += (lt * (1.0-(cd*(1.0/(lss*18.0)))));

		float beers = max(exp(-ld*ncd*lss), exp(-ld*0.25*ncd*lss)*0.7);
		float powshug = 1.0-exp(-ld*ncd*lss*2.0);

		vec3 ambient = 4.0*Ambient*mix(0.15, 1.0, height_fraction);
		vec3 sunC = pow(vSunColor, vec3(0.75));
		L += (ambient+sunC*beers*powshug*2.0)*(t)*T*ss;	
		
		alpha += (1.0-dt)*(1.0-alpha);
		}
		
	}
	L = max(L,vec3(0.4,0.4,0.4)) * color_style;//vec3(0.8,0.5,0.8)
	return vec4(L, alpha);
}
void main()
{
	vec2 shift = vec2(floor(float(check)/downscale), mod(float(check), downscale));	
	
	vec2 uv = (gl_FragCoord.xy*downscale+shift.yx)/(resolution);
	uv = uv-vec2(0.5);
	uv *= 2.0;
	uv.x *= aspect;
	vec4 uvdir = (vec4(uv.xy, 1.0, 1.0));
	vec4 worldPos = (inverse((MVPM))*uvdir);
	//vec3 dir = normalize(worldPos.xyz/worldPos.w);
	vec3 dir = normalize(worldPos.xyz/worldPos.w);
	vec4 col = vec4(0.0);
	if (dir.y>0.0) {
	
		vec3 camPos = cameraPos+vec3(0.0, g_radius, 0.0);
		vec3 start = camPos+dir*intersectSphere(camPos, dir, sky_b_radius);//计算云层底部向量
		vec3 end =min( camPos+dir*intersectSphere(camPos, dir, sky_t_radius),start+dir*500);//计算云层顶部向量
		const float t_dist = sky_t_radius-sky_b_radius;
		float shelldist = (length(end-start));
		
		float steps = (mix(96.0, 54.0, dot(dir, vec3(0.0, 1.0, 0.0))));
		float dmod = smoothstep(0.0, 1.0, (shelldist/t_dist)/14.0);
		float s_dist = mix(t_dist, t_dist*4.0, dmod)/(steps);
		vec3 raystep = dir*s_dist;
		vec4 volume;//云层
		//ray marching 获得云量和颜色
		volume = march(start, end, raystep, int(steps));//好吧 降采样
		volume.xyz = U2Tone(volume.xyz)*cwhiteScale;//云量控制在一定的范围
		volume.xyz = sqrt(volume.xyz);
		volume.a=min(volume.a,0.95);
		vec3 background = vec3(0.6,0.6,0.6);
		//mix bgcolor
		
		col = vec4(background*(1.0-volume.a)+volume.xyz*volume.a,volume.a);
		if (volume.a>1.0) {
			col = vec4(1.0, 0.0, 0.0, 1.0);
		}
	} else {
		col = vec4(vec3(0.0), 0.0);
	}
	
	
	color = col;
}
