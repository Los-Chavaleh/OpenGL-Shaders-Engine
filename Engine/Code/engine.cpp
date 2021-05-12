//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <iostream>

#include "assimp_model_loading.h"

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;

    GLuint vaoHandle = 0;
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;
                    const u32 stride = submesh.vertexBufferLayout.stride;
                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }

            assert(attributeWasLinked);
        }

        glBindVertexArray(0);
    }

    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

void Init(App* app)
{

    // RETRIEVE INFO FROM GPU
    app->oglInfo.version = glGetString(GL_VERSION);
    app->oglInfo.renderer = glGetString(GL_RENDERER);
    app->oglInfo.vendor = glGetString(GL_VENDOR);
    app->oglInfo.shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (int i = 0; i < num_extensions; ++i)
    {
        app->oglInfo.extensions = glGetStringi(GL_EXTENSIONS, GLuint(i));
    }

    // PROGRAM INITIALIZATION
    app->model = LoadModel(app, "Patrick/Patrick.obj");
    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "SHOW_TEXTURED_MESH");
    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    texturedMeshProgram.vertexInputLayout.attributes.push_back({ 0,3 });
    texturedMeshProgram.vertexInputLayout.attributes.push_back({ 2,2 });
    app->mode = Mode_Model;
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::End();
    if (app->oglInfo.show)
    {
        ImGui::Begin("Info");
        ImGui::Text("VERSION: %s", app->oglInfo.version);
        ImGui::Text("RENDERER: %s", app->oglInfo.renderer);
        ImGui::Text("VENDOR: %s", app->oglInfo.vendor);
        ImGui::Text("SHADING LANGUAGE VERSION: %s", app->oglInfo.shadingLanguageVersion);
        ImGui::Text("EXTENSIONS: %s", app->oglInfo.extensions);
        ImGui::End();
    }
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here
    if (app->input.keys[K_0] == ButtonState::BUTTON_PRESS)
        app->oglInfo.show = !app->oglInfo.show;
}

void Render(App* app)
{
    switch (app->mode)
    {
        case Mode_TexturedQuad:
            {
                // TODO: Draw your textured quad here!
        // - clear the framebuffer
            glClearColor(0.2f, 0.2f, 0.2f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // - set the viewport
            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            // - set the blending state
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // - bind the texture into unit 0
            glUniform1i(app->programUniformTexture, 0);
            glActiveTexture(GL_TEXTURE0);
            GLuint textureHandle = app->textures[app->diceTexIdx].handle;
            glBindTexture(GL_TEXTURE_2D, textureHandle);

            // - bind the program
            //   (...and make its texture sample from unit 0)
            const Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
            glUseProgram(programTexturedGeometry.handle);

            // - bind the vao
            glBindVertexArray(app->vao);

            // - glDrawElements() !!!
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
            }
            break;
        case Mode_Model:
        {
            Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);

            Model& model = app->models[app->model];
            Mesh& mesh = app->meshes[model.meshIdx];
            for (u32 i = 0; i < mesh.submeshes.size(); ++i)
            {
                GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                glBindVertexArray(vao);

                u32 submeshMaterialIdx = model.materialIdx[i];
                Material& submeshMaterial = app->materials[submeshMaterialIdx];

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                glUniform1i(app->texturedMeshProgram_uTexture, 0);

                const Submesh& submesh = mesh.submeshes[i];
                glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
            }

            glBindVertexArray(0);

            glUseProgram(0);
        }
        break;

        default:;
    }
}
