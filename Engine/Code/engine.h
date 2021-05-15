//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>
#include "assimp_model_loading.h"
#include <glm/gtx/quaternion.hpp>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

struct VertexV3V2
{
    glm::vec3 pos;
    glm::vec2 uv;
};

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;
    u8                                 stride;
};

struct Vao
{
    GLuint handle;
    GLuint programHandle;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
    VertexShaderLayout vertexInputLayout;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Model,
    Mode_Count
};

struct OpenGLInfo
{
    bool show = false;
    const GLubyte* version = nullptr;
    const GLubyte* renderer = nullptr;
    const GLubyte* vendor = nullptr;
    const GLubyte* shadingLanguageVersion = nullptr;
    const const unsigned char* extensions = nullptr;
    OpenGLInfo()
    {

    }
};

struct Model
{
    u32 meshIdx;
    std::vector<u32> materialIdx;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32> indices;
    u32              vertexOffset;
    u32              indexOffset;
    std::vector<Vao> vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint               vertexBufferHandle;
    GLuint               indexBufferHandle;
};

struct Material
{
    std::string name;
    vec3        albedo;
    vec3        emissive;
    f32         smoothness;
    u32         albedoTextureIdx;
    u32         emissiveTextureIdx;
    u32         specularTextureIdx;
    u32         normalsTextureIdx;
    u32         bumpTextureIdx;
};

struct Camera {
    //vec3 Position = {0.f,0.f,5.f};
    //glm::quat Rotation = glm::quat(1,0,0,0);
    float pitch = 0.f;
    float yaw = 0.f;
    //float aRoll = 220.f;

    //glm::mat4 Transformation() const { return translate(Position) * glm::toMat4(Rotation); };
    //glm::mat4 View() const { return Transformation(); }

    //void Pitch(float angle) { Rotation = rotate(Rotation, glm::radians(angle), vec3(1, 0, 0)); }
    //void Yaw(float angle) { Rotation = rotate(Rotation, glm::radians(angle), vec3(0, 1, 0)); }
    //void Roll(float angle) { Rotation = rotate(Rotation, glm::radians(angle), vec3(0, 0, 1)); }

    float distanceToOrigin = 1.f;
    float phi{ 90.f }, theta{ 90.f };
	float Phi = glm::radians(phi);
	float Theta = glm::radians(theta);

	glm::vec3 cameraPos = { distanceToOrigin * sin(Phi) * cos(Theta), distanceToOrigin * cos(Phi), distanceToOrigin * sin(Phi) * sin(Theta) };
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

	float fov = 60.f;

    glm::mat4 GetViewMatrix(const vec3& camPos, const vec2& size) {
        // Make sure that: 0 < phi < 3.14

		glm::mat4 view = glm::lookAt(camPos, camPos + cameraFront, cameraUp);

        return glm::perspective(glm::radians(fov), size.x / size.y, 0.1f, 100.f) * view;
    }
};

struct Entity
{
    glm::mat4 matrix = glm::mat4(0.f);
    u32 modelId;

    Entity(const glm::mat4& mat, u32 mdlId) : matrix(mat), modelId(mdlId) {};
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    std::vector<Texture>  textures;
    std::vector<Material>  materials;
    std::vector<Mesh>  meshes;
    std::vector<Model>  models;
    std::vector<Program>  programs;
    std::vector<Entity> entities;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;
    u32 meshProgramIdx;
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
    u32 model;
    // Mode
    Mode mode;
    
    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;
    GLuint texturedMeshProgram_uTexture;
    GLuint texturedMeshProgram_uViewProjection;
    GLuint texturedMeshProgram_uWorldMatrix;
    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;
    GLuint frameBufferController;
    GLuint depthController;
    GLuint colorController;
    GLuint normalsController;
    GLuint albedoController;
    // GPU Info
    OpenGLInfo oglInfo;

    Camera camera;
	bool firstMouse = true;

};

u32 LoadTexture2D(App* app, const char* filepath);

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

