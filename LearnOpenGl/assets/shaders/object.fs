struct Material {
	vec4 diffuse;
	vec4 specular;
	
	float shininess;
};

uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;

uniform sampler2D dirLightBuffer;
uniform samplerCube pointLightBuffers[MAX_POINT_LIGHTS];
uniform sampler2D spotLightBuffers[MAX_SPOT_LIGHTS];

out vec4 FragColor;

in VS_OUT {
	vec3 FragPos;
	vec2 TexCoord;

	TangentLights tanLights;
} fs_in;

uniform Material material;
uniform bool noTex;
uniform bool noNormalMap;
uniform bool skipNormalMap;

uniform bool useBlinn;
uniform bool useGamma;
uniform float gamma;

float calcPointLightShadow(int idx, vec3 norm, vec3 viewVec, vec3 lightDir);
vec4 calcPointLight(int idx, vec3 norm, vec3 viewVec, vec3 viewDir, vec4 diffMap, vec4 specMap);
vec4 calcDirLight(vec3 norm, vec3 viewVec, vec3 viewDir, vec4 diffMap, vec4 specMap);
vec4 calcSpotLight(int idx, vec3 norm, vec3 viewVec, vec3 viewDir, vec4 diffMap, vec4 specMap);

#define NUM_SAMPLES 20

vec3 sampleOffsetDirections[NUM_SAMPLES] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  0, 0)
); 

void main(){
    
	vec3 norm = normalize(fs_in.tanLights.Normal);
	
	if(!noNormalMap && !skipNormalMap){
		norm = texture(normal0, fs_in.TexCoord).rgb;
		norm = normalize(norm * 2.0 - 1.0); // map to [-1,1]
	}
	
	vec3 viewVec = fs_in.tanLights.viewPos - fs_in.tanLights.FragPos;
	vec3 viewDir = normalize(viewVec);


	vec4 diffMap;
	vec4 specMap;

	if(noTex){
		diffMap = material.diffuse;
		specMap = material.specular;
	}
	else{
		diffMap = texture(diffuse0, fs_in.TexCoord);
		specMap = texture(specular0, fs_in.TexCoord);
	}

	// placeholder
	vec4 results = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// directional
	results = calcDirLight(norm, viewVec, viewDir, diffMap, specMap);

	// point light
	for (int i = 0; i < noPointLights; i++){
		results += calcPointLight(i, norm, viewVec, viewDir, diffMap, specMap);
	}

	// spot light
	for (int i = 0; i < noSpotLights; i++){
		results += calcSpotLight(i, norm, viewVec, viewDir, diffMap, specMap);
	}

	// gamma correction
	if(useGamma){
		results.rgb = pow(results.rgb, vec3(1.0 / gamma));
	}
	
	// depth testing
	float near = 0.1;
	float far = 100.0f;
	float z = gl_FragCoord.z * 2.0 - 1.0; //  transform to fs_in.Normalize Device coordinates [0, 1] => [1, -1]
	float linearDepth = (2.0 * near * far) / (z * (far - near) - (far + near)); // taking the inverse of projection matrix (perspective)
	float factor = (near + linearDepth) / (near - far); // convert from back to [0, 1]

	results.rgb *= 1 - factor;

	FragColor = results;
}

float calcDirLightShadow(vec3 norm, vec3 viewVec, vec3 lightDir){
	vec4 fragPosLightSpace = dirLight.lightSpaceMatrix * vec4(fs_in.FragPos, 1.0);

	// perspective divide (transforming coordinates fs_in.Normalize Device coordinates)
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // [depth relative to the light] => [-1, 1]

	// NDC to depth range 
	projCoords = projCoords * 0.5 + 0.5; // [-1, 1] => [0, 1]
	
	// if too far from light do not return shadow
	if(projCoords.z > 1.0){
		return 0.0;
	}

	// get depth of fragment
	float currentDepth = projCoords.z;

	// calculate bias
	float maxBias = 0.05;
	float minBias = 0.005;

	float bias = max(maxBias * (1.0 - dot(norm, lightDir)), minBias);

	// PCF
	float shadowSum = 0.0;
	vec2 texelSize = 1.0 / textureSize(dirLightBuffer, 0);
	float viewDist = length(viewVec);
	float diskRadius = (1.0 + (viewDist / dirLight.farPlane)) / 30.0;
	for(int y = -1; y <= 1; y++){
		for(int x = -1; x <=1; x++){
			float pcfDepth = texture(dirLightBuffer, projCoords.xy + vec2(x, y) * texelSize * diskRadius).r;
			shadowSum += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	
	// return the avarage
	return shadowSum / 9.0;
}

vec4 calcDirLight(vec3 norm, vec3 viewVec, vec3 viewDir, vec4 diffMap, vec4 specMap){
	// ambient
	vec4 ambient = dirLight.ambient * diffMap;

	// diffuse
	vec3 lightDir = normalize(-fs_in.tanLights.dirLightDirection);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = dirLight.diffuse * (diff * diffMap);

	// specular
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
	if(diff > 0){
		// if diff is <= 0 object is "behing" light

		float dotProd = 0.0;
		if(useBlinn){
			// caluclate using Blinn Phong model
			vec3 halfwayDir = normalize(lightDir + viewDir);
			dotProd = dot(norm, halfwayDir);
		}
		else{
			// normal Phong model
			vec3 reflectDir = reflect(-lightDir, norm);
			dotProd = dot(viewDir, reflectDir);
		}
		float spec = pow(max(dotProd, 0.0), material.shininess * 128);
		specular = dirLight.specular * (spec * specMap);
	}

	float shadow = calcDirLightShadow(norm, viewVec, lightDir);
	//float shadow = 0;	

	return vec4(ambient + (1.0 - shadow) * (diffuse + specular));

}

float calcPointLightShadow(int idx, vec3 norm, vec3 viewVec, vec3 lightDir){
	// get vector from the light to the fragment
	vec3 lightToFrag = fs_in.FragPos - pointLights[idx].position;

	// get current depth
	float currentDepth = length(lightToFrag);

	// calculate bias
	float minBias = 0.005;
	float maxBias = 0.05;
	float bias = max(maxBias * (1.0 - dot(norm, lightDir)), minBias);

	// PCF
	float shadow = 0.0;
	float viewDist = length(viewVec);
	float diskRadius = (1.0 + (viewDist / pointLights[idx].farPlane)) / 30.0;
	
	for (int i = 0; i < NUM_SAMPLES; i++) {
		float pcfDepth = texture(pointLightBuffers[idx], 
			lightToFrag + sampleOffsetDirections[i] * diskRadius).r;

		pcfDepth *= pointLights[idx].farPlane;

		if (currentDepth - bias > pcfDepth) {
			shadow += 1.0;
		}
		
	}

	shadow /= NUM_SAMPLES;

	return shadow;
	
}

vec4 calcPointLight(int idx, vec3 norm, vec3 viewVec, vec3 viewDir, vec4 diffMap, vec4 specMap){
	// ambient
	vec4 ambient = pointLights[idx].ambient * diffMap;

	// diffuse
	vec3 lightDir = normalize(fs_in.tanLights.pointLightPositions[idx] - fs_in.tanLights.FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec4 diffuse = pointLights[idx].diffuse * (diff * diffMap);


	// specular
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
	if(diff > 0){
		// if diff is <= 0 object is "behing" light
		float dotProd = 0.0;
		if(useBlinn){
			// caluclate using Blinn Phong model
			vec3 halfwayDir = normalize(lightDir + viewDir);
			dotProd = dot(norm, halfwayDir);
		}
		else{
			// normal Phong model
			vec3 reflectDir = reflect(-lightDir, norm);
			dotProd = dot(viewDir, reflectDir);
		}
		float spec = pow(max(dotProd, 0.0), material.shininess * 128);
		specular = pointLights[idx].specular * (spec * specMap);
	}

	
	// calculate intansity
	float dist = length(pointLights[idx].position - fs_in.FragPos);
	float attenuation = 1.0 / (pointLights[idx].k0 + (pointLights[idx].k1 * dist) + (pointLights[idx].k2 * (dist * dist)));

	// apply attenuation
	ambient *= attenuation;
	specular *= attenuation;
	diffuse *= attenuation;

	float shadow = calcPointLightShadow(idx, norm, viewVec, lightDir);
	//float shadow = 0;
	return vec4(ambient + (1 - shadow) * (diffuse + specular));
}

float calcSpotLightShadow(int idx, vec3 norm, vec3 viewVec, vec3 lightDir){
	vec4 fragPosLightSpace = spotLights[idx].lightSpaceMatrix * vec4(fs_in.FragPos, 1.0);
	
	// perspective devide (transorm to NDC)
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w; // [near, far] => [-1, 1]
	
	// NDC to Depth range
	projCoords = projCoords * 0.5 + 0.5; // [-1, 1] => [0, 1]
	
	// if too far from light do not return shadow
	if(projCoords.z > 1.0){
		return 0.0;
	}

	// get depth of the fragment
	float currentDepth = projCoords.z;

	// calculate bias
	float maxBias = 0.05;
	float minBias = 0.005;

	float bias = max(maxBias * (1.0 - dot(norm, lightDir)), minBias);

	// PCF
	float shadowSum = 0.0;
	vec2 texelSize = 1.0 / textureSize(spotLightBuffers[idx], 0);
	float viewDist = length(viewVec);
	float diskRadius = (1.0 + (viewDist / spotLights[idx].farPlane)) / 30.0;
	for(int y = -1; y <= 1; y++){
		for(int x = -1; x <=1; x++){
			float pcfDepth = texture(spotLightBuffers[idx], projCoords.xy + vec2(x, y) * texelSize * diskRadius).r;
			pcfDepth  *= spotLights[idx].farPlane;
			shadowSum += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	
	// return the avarage
	return shadowSum / 9.0;
}

vec4 calcSpotLight(int idx, vec3 norm, vec3 viewVec, vec3 viewDir, vec4 diffMap, vec4 specMap){

	vec3 lightDir = normalize(fs_in.tanLights.spotLightPositions[idx] - fs_in.tanLights.FragPos);
	float theta = dot(lightDir, normalize(-fs_in.tanLights.spotLightDirections[idx]));

	// ambient
	vec4 ambient = spotLights[idx].ambient * diffMap;

	if(theta > spotLights[idx].outerCutOff){ // > because using cosines and not degrees
		// if in cutoff, render light

		// diffuse
		float diff = max(dot(norm, lightDir), 0.0);
		vec4 diffuse = spotLights[idx].diffuse * (diff * diffMap);

		// specular
		vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
		if(diff > 0){
			// if diff is <= 0 object is "behing" light

			float dotProd = 0.0;
			if(useBlinn){
				// caluclate using Blinn Phong model
				vec3 halfwayDir = normalize(lightDir + viewDir);
				dotProd = dot(norm, halfwayDir);
			}
			else{
				// normal Phong model
				vec3 reflectDir = reflect(-lightDir, norm);
				dotProd = dot(viewDir, reflectDir);
			}
			float spec = pow(max(dotProd, 0.0), material.shininess * 128);
			specular = spotLights[idx].specular * (spec * specMap);
		}

		float intensity = clamp((theta - spotLights[idx].outerCutOff) / (spotLights[idx].cutOff - spotLights[idx].outerCutOff), 0.0, 1.0);
		
		diffuse *= intensity;
		specular *= intensity;

		float dist = length(spotLights[idx].position - fs_in.FragPos);
		float attenuation = 1.0 / (spotLights[idx].k0 + spotLights[idx].k1 * dist + spotLights[idx].k2 * (dist * dist));
	
		// apply attenuation
		ambient *= attenuation;
		specular *= attenuation;
		diffuse *= attenuation;

		float shadow = calcSpotLightShadow(idx, norm, viewVec, lightDir);
		//float shadow = 0.0;		

		return vec4(ambient + (1.0 - shadow) * (diffuse + specular));
	}
	else{ // render just ambient
		return ambient;
	}
}



