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
     	 float 			intensity;
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
out vec3 vPosition;

void main() {
    gl_Position = uWorldViewProjectionMatrix * uWorldMatrix * vec4(aPosition, 1.0);
    vNormals = mat3(transpose(inverse(uWorldMatrix))) * aNormals;
    vTexCoord = aTexCoord;
    vViewDir = uCameraPosition - aPosition;
    vPosition = vec3(uWorldMatrix * vec4(aPosition,1.0));
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

 struct Light{
	 unsigned int 	type; // 0: dir, 1: point
	 vec3	color;
	 vec3	direction;
	 vec3	position;
     float 			intensity;
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
in vec3 vPosition;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormals;
layout(location = 2) out vec4 oAlbedo;

vec3 DirectionalLight(vec3 lightPosition, vec3 color, vec3 normal);
vec3 PointLight(vec3 lightPosition, vec3 color, vec3 normal, vec3 fragPosition, vec3 view_dir, vec2 texCoords);

void main() {
	vec3 lightsColors = vec3(0.0,0.0,0.0);
	for(int i = 0; i < uLightCount; ++i)
	{		if(uLight[i].type == 0) //Directional
			    lightsColors += DirectionalLight(uLight[i].position, uLight[i].color, normalize(vNormals));
            else //PointLight
            {
                lightsColors += PointLight(uLight[i].position, uLight[i].color, vNormals, vPosition,normalize(vViewDir), vTexCoord);
            }
	}
	oColor 		= vec4(lightsColors, 1.0)*texture(uTexture, vTexCoord);
	oNormals 	= vec4(vNormals, 1.0);
    gl_FragDepth = gl_FragCoord.z - 0.2;
    oAlbedo   =   texture(uTexture, vTexCoord);
}

vec3 DirectionalLight(vec3 lightPos, vec3 color, vec3 normal){
    vec3 lightColor = vec3(1.);
    // Ambient
    vec3 ambient = lightColor * 0.15 * color;

    // Diffuse
    vec3 lightDirection = normalize(-lightPos);
    float diffuseIntensity = max(dot(normal, lightDirection),0.0);
    vec3 diffuse = diffuseIntensity * lightColor * color;

    // Specular
    float specularStrength = .85;
    float specularIntensity = pow(max(dot(normal, lightDirection),0.0),32.);
    vec3 specular = specularStrength * specularIntensity * lightColor;
    
    return ambient + diffuse + specular;
}

vec3 PointLight(vec3 lightPos, vec3 color, vec3 normal, vec3 fragPosition, vec3 view_dir, vec2 texCoords)
{
    // Ambient
    vec3 ambient = color;

    // Diffuse
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = ambient * diff;

     // Specular
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(view_dir, reflectDir), 0.0), 0.0) * 0.01;
    vec3 specular = ambient * spec;

    // Range
    float distance = length(lightPos - fragPosition);
    float range = 4/distance;
    
	return (diffuse + specular) * range;
}
#endif
#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef SHOW_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormals;
layout(location=2) in vec2 aTexCoord;

layout(binding = 0, std140) uniform GlobalParms
{
	vec3 			uCameraPosition;
 	int 			uLightCount;
};

layout(binding = 1, std140) uniform LocalParms
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vNormals;
out vec3 vViewDir;
out vec3 vPosition;

void main() {
    gl_Position = uWorldViewProjectionMatrix * uWorldMatrix * vec4(aPosition, 1.0);
    vNormals = mat3(transpose(inverse(uWorldMatrix))) * aNormals;
    vTexCoord = aTexCoord;
    vViewDir = uCameraPosition - aPosition;
    vPosition = vec3(uWorldMatrix * vec4(aPosition,1.0));
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

 struct Light{
	 unsigned int 	type; // 0: dir, 1: point
	 vec3	color;
	 vec3	direction;
	 vec3	position;
     float 			intensity;
};

layout(binding = 0, std140) uniform GlobalParms
{
	vec3 			uCameraPosition;
	int 			uLightCount;
};

in vec2 vTexCoord;
in vec3 vNormals;
in vec3 vViewDir;
in vec3 vPosition;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormals;
layout(location = 2) out vec4 oAlbedo;
layout(location = 3) out vec4 oPosition;
void main() {

	oColor 		= texture(uTexture, vTexCoord);
	oNormals 	= vec4(vNormals, 1.0);
    
    oAlbedo   =   texture(uTexture, vTexCoord);
    oPosition = vec4(vPosition, 1.0);
    gl_FragDepth = gl_FragCoord.z - 0.2;
}

#endif
#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef SHOW_LIGHT

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

struct Light{
	 unsigned int 	type; // 0: dir, 1: point
	 vec3	color;
	 vec3	direction;
	 vec3	position;
     float 	intensity;
};

layout(binding = 0, std140) uniform GlobalParms
{
	vec3 			uCameraPosition;
 	int 			uLightCount;
 	Light			uLight[16];
};

out vec2 vTexCoord;

void main() {

	gl_Position = vec4(aPosition, 1.0);

	vTexCoord = aTexCoord;
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

 struct Light{
	 unsigned int 	type; // 0: dir, 1: point
	 vec3	color;
	 vec3	direction;
	 vec3	position;
     float 	intensity;
};

vec3 DirectionalLight(Light light, vec3 normal, vec3 view_dir, vec2 texCoords){
    vec3 lightColor = vec3(1.);
    // Ambient
    vec3 ambient = lightColor * 0.15 * light.color;

    // Diffuse
    vec3 lightDirection = normalize(-light.position);
    float diffuseIntensity = max(dot(normal, light.direction),0.0);
    vec3 diffuse = diffuseIntensity * lightColor * light.color;

    // Specular
    float specularStrength = 0.01;
    float specularIntensity = pow(max(dot(normal, lightDirection),0.0),0.1);
    vec3 specular = specularStrength * specularIntensity * lightColor * light.intensity;
    
    return ambient + diffuse + specular;
}

vec3 PointLight(Light light, vec3 normal, vec3 frag_pos, vec3 view_dir, vec2 texCoords)
{
    vec3 ambient = light.color;
    // diffuse shadi
    vec3 lightDir = normalize(light.position - frag_pos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = ambient * diff;
     // Specul
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(view_dir, reflectDir), 0.0), 0.0) * 0.01;//    vec3 specular = vec3(0);
    vec3 specular = ambient * spec;
    // attenuati
    float distance = length(light.position - frag_pos);
    float attenuation = 1/distance;      
	return (diffuse + specular) * attenuation * light.intensity;
}

layout(binding = 0, std140) uniform GlobalParms
{
	vec3 			uCameraPosition;
	int 			uLightCount;
	Light			uLight[16];
};

uniform sampler2D uPositionTexture;
uniform sampler2D uNormalsTexture;
uniform sampler2D uAlbedoTexture;

in vec2 vTexCoord;

layout(location = 0) out vec4 oColor;

void main() {
	vec3 fragPos = texture(uPositionTexture, vTexCoord).rgb;
	vec3 norms = texture(uNormalsTexture, vTexCoord).rgb;
	vec3 diffuseCol = texture(uAlbedoTexture, vTexCoord).rgb;

	vec3 viewDir = normalize(uCameraPosition - fragPos);
	vec3 lightsColors = vec3(0.0,0.0,0.0);
	for(int i = 0; i < uLightCount; ++i)
	{		
        if(uLight[i].type == 0) //Directional
        {
			lightsColors += DirectionalLight(uLight[i], norms, normalize(viewDir), vTexCoord);
        }
        else //PointLight
        {
            lightsColors += PointLight(uLight[i], norms, fragPos, viewDir, vTexCoord);
        }
	}
    oColor = vec4(lightsColors + diffuseCol * 0.2, 1.0);
}
#endif
#endif

#ifdef DRAW_LIGHT

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;

uniform mat4 projectionView;
uniform mat4 model;

void main() {
	gl_Position = projectionView * model * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

layout(location = 0) out vec4 oColor;

uniform vec3 lightColor;

void main() {
	oColor = vec4(lightColor, 1.0);
    gl_FragDepth = gl_FragCoord.z - 0.2;
}

#endif
#endif