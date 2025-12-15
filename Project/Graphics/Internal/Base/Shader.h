#pragma once

#include "Gl.h"

using namespace std;

unsigned int currentlyActivatedShaderID;

class Shader
{
public:
    unsigned int ID = -1;
    map<string, GLint> uniforms; // optimization.

    Shader(const char* vertexPath, const char* fragmentPath, bool readPath = true) {
        if (vertexPath == NULL || fragmentPath == NULL)
            return;
        const char* vshadertxt = vertexPath;
        const char* fshadertxt = fragmentPath;

        if (readPath) {
            vshadertxt = Utils::ReadFile(vertexPath);
            fshadertxt = Utils::ReadFile(fragmentPath);
        }

        int success = false;
        char infoLog[512];

        GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vshadertxt, NULL);
        glCompileShader(vshader);

        glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vshader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fshadertxt, NULL);
        glCompileShader(fshader);

        glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fshader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vshader);
        glAttachShader(program, fshader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vshader);
        glDeleteShader(fshader);


        ID = program;
        currentlyActivatedShaderID = ID;
    }

    void use() {
        currentlyActivatedShaderID = ID;
        glUseProgram(ID);
    }

    // Uniforms

    GLint uniformLocation(const string& name) {
        if (uniforms.find(name) == uniforms.end())
            uniforms[name] = glGetUniformLocation(ID, name.c_str());

        return uniforms[name];
    }

    inline void preSet(const string& name) {
        if (currentlyActivatedShaderID != ID)
            use();
    }

    void setBool(const string& name, bool value) {
        preSet(name);

        glUniform1i(uniformLocation(name), (int)value);
    }

    void setInt(const string& name, int value) {
        preSet(name);

        glUniform1i(uniformLocation(name), value);
    }

    void setFloat(const string& name, float value) {
        preSet(name);

        glUniform1f(uniformLocation(name), value);
    }

    void setVec2(const string& name, const glm::vec2& value) {
        preSet(name);

        glUniform2fv(uniformLocation(name), 1, &value[0]);
    }

    void setVec2(const string& name, float x, float y) {
        preSet(name);

        glUniform2f(uniformLocation(name), x, y);
    }

    void setVec3(const string& name, const glm::vec3& value) {
        preSet(name);

        glUniform3fv(uniformLocation(name), 1, &value[0]);
    }

    void setVec3(const string& name, float x, float y, float z) {
        preSet(name);

        glUniform3f(uniformLocation(name), x, y, z);
    }

    void setVec4(const string& name, const glm::vec4& value) {
        preSet(name);

        glUniform4fv(uniformLocation(name), 1, &value[0]);
    }

    void setVec4(const string& name, float x, float y, float z, float w) {
        preSet(name);

        glUniform4f(uniformLocation(name), x, y, z, w);
    }

    void setMat2(const string& name, const glm::mat2& mat) {
        preSet(name);

        glUniformMatrix2fv(uniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3(const string& name, const glm::mat3& mat) {
        preSet(name);

        glUniformMatrix3fv(uniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4(const string& name, const glm::mat4& mat) {
        preSet(name);

        glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, &mat[0][0]);
    }
};
