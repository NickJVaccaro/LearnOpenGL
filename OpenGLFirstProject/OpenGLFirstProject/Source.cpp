#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h";

#include <iostream>
#include <map>

bool firstMouse = true;
float lastX = 400, lastY = 300;
float pitch = 0;
float yaw = -90.0f;

float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

glm::vec3 lightPos(1.2f, 2.0f, 2.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.ProcessMouseScroll(yOffset);
}

void drawModel(Model objModel, Shader shader, glm::vec3 position, float scale);
void setupShader(Shader shader);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 900, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 1200, 900);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glEnable(GL_STENCIL_TEST);
    //glStencilFunc(GL_EQUAL, 1, 0xFF);

    // Capture mouse & set up mouse event callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };

    //float billboardVertices[] = {
    //    // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
    //    0.0f,  0.5f,  0.0f,  0.0f,  0.0f, // 1
    //    0.0f, -0.5f,  0.0f,  0.0f,  1.0f, // 2
    //    1.0f, -0.5f,  0.0f,  1.0f,  1.0f, // 3

    //    0.0f,  0.5f,  0.0f,  0.0f,  0.0f, // 1
    //    1.0f, -0.5f,  0.0f,  1.0f,  1.0f, // 2
    //    1.0f,  0.5f,  0.0f,  1.0f,  0.0f  // 3
    //};

    // Exercise 1: Reverse the order from CW to CCW for a cube (but I'm just doing the billboards)
    float billboardVertices[] = {
        // positions
        0.0,  0.5, 0.0, 0.0, 0.0, // 1
        1.0, -0.5, 0.0, 1.0, 1.0, // 3
        0.0, -0.5, 0.0, 0.0, 1.0, // 2

        0.0,  0.5, 0.0, 0.0, 0.0, // 1
        1.0,  0.5, 0.0, 1.0, 0.0, // 3
        1.0, -0.5, 0.0, 1.0, 1.0  // 2
    };

    // Set up our shaders
    Shader ourShader("./shader.vert", "./shader.frag");
    Shader lightShader("./lightsource.vert", "./lightsource.frag");
    Shader outlineShader("./shader.vert", "./shaderSingleColor.frag");
    Shader grassShader("./transparentShader.vert", "./transparentShader.frag");

    // Define our common vars
    glm::mat4 view;
    glm::mat4 projection;

    // Assign anything that doesn't change
    ourShader.use();
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);
    ourShader.setInt("material.emission", 2);
    ourShader.setFloat("material.shininess", 64);

    ourShader.setVec3("dirLight.direction", -0.2, -1.0, -0.2);
    ourShader.setVec3("dirLight.ambient", 0.05, 0.05, 0.05);
    ourShader.setVec3("dirLight.diffuse", 0.4, 0.4, 0.4);
    ourShader.setVec3("dirLight.specular", 1.0, 1.0, 1.0);

    for (unsigned int i = 0; i < 4; i++)
    {
        std::string array = "pointLights[" + std::to_string(i) + "]";
        ourShader.setVec3(array + ".position", pointLightPositions[i]);

        ourShader.setVec3(array +".ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3(array + ".diffuse", 0.5f, 0.5f, 0.5f);
        ourShader.setVec3(array + ".specular", 1.0f, 1.0f, 1.0f);

        ourShader.setFloat(array + ".constant", 1.0f);
        ourShader.setFloat(array + ".linear", 0.09f);
        ourShader.setFloat(array + ".quadratic", 0.032f);
    }
    
    ourShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
    ourShader.setVec3("spotLight.diffuse", 0.5f, 0.5f, 0.5f);
    ourShader.setVec3("spotLight.specular", 1.0, 1.0, 1.0);

    ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5)));
    ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5)));
    ourShader.setFloat("spotLight.constant", 1.0f);
    ourShader.setFloat("spotLight.linear", 0.09f);
    ourShader.setFloat("spotLight.quadratic", 0.032f);

    stbi_set_flip_vertically_on_load(true);
    Model bagpag("./backpack/backpack.obj");
    stbi_set_flip_vertically_on_load(false);

    // Set up grass rendering
    vector<glm::vec3> transparentObjs;
    transparentObjs.push_back(glm::vec3(-1.5f, -0.5f, -0.48f));
    transparentObjs.push_back(glm::vec3( 1.5f, -0.5f,  0.51f));
    transparentObjs.push_back(glm::vec3( 0.0f, -0.5f,  0.7f));
    transparentObjs.push_back(glm::vec3(-0.3f, -0.5f, -2.3f));
    transparentObjs.push_back(glm::vec3( 0.5f, -0.5f, -0.6f));

    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(billboardVertices), billboardVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    
    unsigned int grassTexture;
    glGenTextures(1, &grassTexture);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("textures/blending_transparent_window.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture grass.png" << std::endl;
    }
    stbi_image_free(data);

    grassShader.use();
    grassShader.setInt("texture1", 0);

    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // rendering commands here
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);

        // clear & set background:
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // First, draw the bagpags
        ourShader.use();
        setupShader(ourShader);
        ourShader.setVec3("lightColor", glm::vec3(1.0, 1.0, 1.0));
        ourShader.setVec3("spotLight.position", camera.Position);
        ourShader.setVec3("spotLight.direction", camera.Front);

        glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should pass the stencil test
        glStencilMask(0xFF); // enable writing to the stencil buffer
        drawModel(bagpag, ourShader, glm::vec3(0.0, 0.0, 0.0), 0.5);
        drawModel(bagpag, ourShader, glm::vec3(3.0, 0.0, -5.0), 0.5);

        // Then, Draw transparencies
        glFrontFace(GL_CW);
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < transparentObjs.size(); i++)
        {
            float distance = glm::length(camera.Position - transparentObjs[i]);
            sorted[distance] = transparentObjs[i];
        }

        grassShader.use();
        grassShader.setMat4("view", camera.GetViewMatrix());
        grassShader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom), 1200.0f / 900.0f, 0.1f, 100.0f));
        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, grassTexture);

        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            glm::mat4 model = glm::mat4(1.0);
            model = glm::translate(model, it->second);
            grassShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void drawModel(Model objModel, Shader shader, glm::vec3 position, float scale)
{
    glm::mat4 model = glm::mat4(1.0);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    shader.setMat4("model", model);
    objModel.Draw(shader);
}

void setupShader(Shader shader)
{
    shader.setMat4("view", camera.GetViewMatrix());
    shader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f));
    shader.setVec3("viewPos", camera.Position);
}