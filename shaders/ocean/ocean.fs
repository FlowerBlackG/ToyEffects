#version 330

in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 colour;

struct DirectionalLight 
{
	vec3 colour;
	float ambientIntensity;
	vec3 direction;
	float diffuseIntensity;
};

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform sampler2D theTexture;
uniform DirectionalLight directionalLight;
uniform Material material;

uniform vec3 eyePosition;

void main()
{
	//环境光照
	vec4 ambientColour = vec4(directionalLight.colour, 1.0f) * directionalLight.ambientIntensity;
	//漫反射光照，我们对norm和lightDir向量进行点乘，计算光源对当前片段实际的漫反射影响。结果值再乘以光的颜色，得到漫反射分量。
	//两个向量之间的角度越大，漫反射分量就会越小
	float diffuseFactor = max(dot(normalize(Normal), normalize(directionalLight.direction)), 0.0f);
	vec4 diffuseColour = vec4(directionalLight.colour, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;
	
	vec4 specularColour = vec4(0, 0, 0, 0);
	
	if(diffuseFactor > 0.0f)
	{
		vec3 fragToEye = normalize(eyePosition - FragPos);
		//求反射向量
		vec3 reflectedVertex = normalize(reflect(directionalLight.direction, normalize(Normal)));
		//镜面高光影响因子
		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularColour = vec4(directionalLight.colour * material.specularIntensity * specularFactor, 1.0f);
		}
	}

	colour = texture(theTexture, TexCoord) * (ambientColour + diffuseColour + specularColour);
}