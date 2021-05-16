///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPos, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main() {
    oColor = texture(uTexture, vTexCoord);
}

#endif
#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef SHOW_TEXTURED_MESH

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormals;
layout(location=2) in vec2 aTexCoord;

struct Light{
	 unsigned int 	type; // 0: dir, 1: point
	 vec3	color;
	 vec3	direction;
	 vec3	position;
};

layout(binding = 0, std140) uniform GlobalParms
{
	vec3 			uCameraPosition;
 	int 			uLightCount;
 	Light			uLight[16];
};

layout(binding = 1, std140) uniform LocalParms
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vNormals;
out vec3 vViewDir;

void main() {
    gl_Position = uWorldViewProjectionMatrix * uWorldMatrix * vec4(aPosition, 1.0);
    vNormals = mat3(transpose(inverse(uWorldMatrix))) * aNormals;
    vTexCoord = aTexCoord;
    vViewDir = uCameraPosition - aPosition;
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////



 struct Light{
	 unsigned int 	type; // 0: dir, 1: point
	 vec3	color;
	 vec3	direction;
	 vec3	position;
};

layout(binding = 0, std140) uniform GlobalParms
{
	vec3 			uCameraPosition;
	int 			uLightCount;
	Light			uLight[16];
};

in vec2 vTexCoord;
in vec3 vNormals;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormals;
layout(location = 2) out vec4 oAlbedo;

vec3 DirectionalLight(vec3 lightPosition, vec3 color, vec3 normal);

void main() {
	vec3 lightsColors = vec3(0.0,0.0,0.0);
	for(int i = 0; i < 1; ++i)
	{			
			lightsColors += DirectionalLight(uLight[i].position, uLight[i].color, normalize(vNormals));
	}
	oColor 		= vec4(lightsColors, 1.0)*texture(uTexture, vTexCoord);
	oNormals 	= vec4(vNormals, 1.0);
    gl_FragDepth = gl_FragCoord.z - 0.2;
    oAlbedo   =   texture(uTexture, vTexCoord);
    //oLight		= vec4(result, 1.0);
}

vec3 DirectionalLight(vec3 lightPos, vec3 color, vec3 normal){
    vec3 lightColor = vec3(1.);
    // Ambient
    vec3 ambient = lightColor * 0.15 * color;

    //Diffuse
    vec3 lightDirection = normalize(-lightPos);
    float diffuseIntensity = max(dot(normal, lightDirection),0.0);
    vec3 diffuse = diffuseIntensity * lightColor * color;

    //Specular
    float specularStrength = .85;
    float specularIntensity = pow(max(dot(normal, lightDirection),0.0),32.);
    vec3 specular = specularStrength * specularIntensity * lightColor;
    
    return ambient + diffuse + specular;
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
