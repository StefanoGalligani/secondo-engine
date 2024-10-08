#version 330 core

#define MAX_LIGHTS 5

struct SpotLight {
	vec3 pos;
	vec3 color;
	float intensity;
	float range;
	sampler2DShadow shadowTexture;
	sampler2D mapTexture;
	int mapTextureSet;
};
uniform SpotLight spotLights[MAX_LIGHTS];
uniform int nSpotLights;

struct PointLight {
	vec3 pos;
	vec3 worldPos;
	vec3 color;
	float intensity;
	float range;
	samplerCube shadowTexture;
	float far_plane;
};
uniform PointLight pointLights[MAX_LIGHTS];
uniform int nPointLights;

struct DirLight {
	vec3 dir;
	vec3 color;
	float intensity;
	sampler2DShadow shadowTexture;
};
uniform DirLight dirLights[MAX_LIGHTS];
uniform int nDirLights;

struct AmbientLight {
	vec3 color;
	float intensity;
};
uniform AmbientLight ambientLights[MAX_LIGHTS];
uniform int nAmbientLights;

struct Material {
	vec3 Kd;
	vec3 Ks;
	float Ns;
	float d;
	sampler2D dTexture;
	int dTextureSet;
	sampler2D sTexture;
	int sTextureSet;
};  
uniform Material material;

in vec3 worldPosition;
in vec3 viewPosition;
in vec2 textureC;
in vec3 fragNormal;
in vec4 spotLightViewPositions[MAX_LIGHTS];
in vec4 dirLightViewPositions[MAX_LIGHTS];

out vec4 color;

vec3 CalcSpotLightComponent(int i, vec3 albedo, vec3 specular, vec3 camDir) {
	vec3 toLight = spotLights[i].pos - viewPosition;
	vec3 lightDir = normalize(toLight);
	float diffuseComponent = max(0, dot(lightDir, normalize(fragNormal)));
	
	vec3 bounceDir = normalize(lightDir + camDir);
	float specularComponent = pow(max(0, dot(bounceDir, normalize(fragNormal))), material.Ns);

	vec3 p = spotLightViewPositions[i].xyz;
	p.z *= 0.99999;
	p /= spotLightViewPositions[i].w;
	if (p.x > 1 || p.x < 0 || p.y > 1 || p.y < 0 || p.z > 1.0 || p.z < 0.0) {
		diffuseComponent = 0; specularComponent = 0;
	} else {
		float litValue = texture(spotLights[i].shadowTexture, p);
		if (spotLights[i].mapTextureSet == 1) litValue *= texture(spotLights[i].mapTexture, p.xy).x;
		float coefficient = litValue * max(0., (1 - length(toLight) / spotLights[i].range));
		diffuseComponent *= coefficient;
		specularComponent *= coefficient;
	}
	
	// blinn-phong
	vec3 shading = spotLights[i].color * (diffuseComponent * albedo) + specular * specularComponent;
	return spotLights[i].intensity * shading;
}

vec3 CalcPointLightComponent(int i, vec3 albedo, vec3 specular, vec3 camDir) {
	vec3 toLight = pointLights[i].pos - viewPosition;
	vec3 lightDir = normalize(toLight);
	float diffuseComponent = max(0, dot(lightDir, normalize(fragNormal)));
	
	vec3 bounceDir = normalize(lightDir + camDir);
	float specularComponent = pow(max(0, dot(bounceDir, fragNormal)), material.Ns);

	vec3 toLightWorld = pointLights[i].worldPos - worldPosition;
	float sampledDistance = texture(pointLights[i].shadowTexture, -toLightWorld).x;
	sampledDistance *= pointLights[i].far_plane;
	bool inShadow = (length(toLightWorld) - sampledDistance) >= 0.01;
	if (inShadow) {
		diffuseComponent = 0; specularComponent = 0;
	} else {
		float coefficient = max(0., (1 - length(toLightWorld) / pointLights[i].range));
		diffuseComponent *= coefficient;
		specularComponent *= coefficient;
	}

	// blinn-phong
	vec3 shading = pointLights[i].color * (diffuseComponent * albedo) + specular * specularComponent;
	return pointLights[i].intensity * shading;
}

vec3 CalcDirLightComponent(int i, vec3 albedo, vec3 specular, vec3 camDir) {
	vec3 lightDir = normalize(-dirLights[i].dir);
	float diffuseComponent = max(0, dot(lightDir, normalize(fragNormal)));
	
	vec3 bounceDir = normalize(lightDir + camDir);
	float specularComponent = pow(max(0, dot(bounceDir, fragNormal)), material.Ns);

	vec3 p = dirLightViewPositions[i].xyz;
	p.z *= 0.99;
	p /= dirLightViewPositions[i].w;
	if (p.x > 1 || p.x < 0 || p.y > 1 || p.y < 0 || p.z > 1.0 || p.z < 0.0) {
		diffuseComponent = 0; specularComponent = 0;
	} else {
		float litValue = texture(dirLights[i].shadowTexture, p);
		diffuseComponent *= litValue;
		specularComponent *= litValue;
	}
	
	// blinn-phong
	vec3 shading = dirLights[i].color * (diffuseComponent * albedo) + specular * specularComponent;
	return dirLights[i].intensity * shading;
}

vec3 CalcAmbientLightComponent(int i, vec3 albedo) {
	return albedo * ambientLights[i].color * ambientLights[i].intensity;
}

void main() {
	vec3 albedo = (material.dTextureSet == 1) ? texture(material.dTexture, textureC).xyz * material.Kd : material.Kd;
	vec3 specular = (material.sTextureSet == 1) ? texture(material.sTexture, textureC).xyz * material.Ks : material.Ks;
	
	vec3 camDir = -normalize(viewPosition);
	vec3 shading = vec3(0.);
	for(int i=0; i<nSpotLights; i++) {
		shading += CalcSpotLightComponent(i, albedo, specular, camDir);
	}
	for(int i=0; i<nPointLights; i++) {
		shading += CalcPointLightComponent(i, albedo, specular, camDir);
	}
	for(int i=0; i<nDirLights; i++) {
		shading += CalcDirLightComponent(i, albedo, specular, camDir);
	}
	for(int i=0; i<nAmbientLights; i++) {
		shading += CalcAmbientLightComponent(i, albedo);
	}
	
	color = vec4(shading, material.d);
}