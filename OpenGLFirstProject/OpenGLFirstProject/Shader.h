#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // Include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // The program ID
    unsigned int ID;

    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    {
        // 1. Retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;

        // Ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            // Open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            
            // Read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();

            // Close file handlers
            vShaderFile.close();
            fShaderFile.close();

            // Convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

            if (geometryPath != nullptr)
            {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR:SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        }

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. Compile shaders
        unsigned int vertex, fragment;
        int success;
        char infoLog[512];

        // Vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        // Print compile errors if any
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR:SHADER:VERTEX:COMPILATION_FAILED\n" << vertexPath << "\n" << infoLog << std::endl;
        }

        // Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        // Print compile errors if any
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) 
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR:SHADER:FRAGMENT:COMPILATION_FAILED\n" << fragmentPath << "\n" << infoLog << std::endl;
        }

        // Geometry Shader (if applicable)
        unsigned int geometry;
        if (geometryPath != nullptr)
        {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(geometry, 512, NULL, infoLog);
                std::cout << "ERROR:SHADER:GEOMETRY:COMPILATION_FAILED\n" << geometryPath << "\n" << infoLog << std::endl;
            }
        }

        // Shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if(geometryPath != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        // Print linking errors if any
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "ERROR:SHADER:PROGRAM:LINKING_FAILED\n" << infoLog << std::endl;
        }

        // Delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);
    }

    // Use/Activate the shader
    void use()
    {
        glUseProgram(ID);
    }

    // Utility uniform functions:
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setMat3(const std::string& name, glm::mat3 value) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
    void setMat4(const std::string& name, glm::mat4 value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }
    void setVec2(const std::string& name, glm::vec2 value) const
    {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setVec3(const std::string& name, glm::vec3 value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }
    void setVec3(const std::string& name, float xVal, float yVal, float zVal) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(glm::vec3(xVal, yVal, zVal)));
    }
};

#endif