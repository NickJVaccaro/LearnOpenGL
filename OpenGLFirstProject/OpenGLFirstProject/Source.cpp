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

bool blinn = false;
bool blinnKeyPressed = false;

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

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
    {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        blinnKeyPressed = false;
    }
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
void drawScene(Shader mainShader, Shader transShader, Model bagpag, glm::mat4 viewMatrix, vector<glm::vec3> transObjs, int transVAO, int transTexture, int skyboxVAO, int skyboxTexture);
unsigned int loadCubemap(vector<std::string> faces);
void drawSkybox(Shader shader, int vao, int texture, glm::mat4 view);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    // Capture mouse & set up mouse event callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Set up our shaders
    Shader shader("./shader.vert", "./shader.frag");

    // Define our common vars
    glm::mat4 view;
    glm::mat4 projection;

    stbi_set_flip_vertically_on_load(true);
    Model bagpag("./backpack/backpack.obj");
    stbi_set_flip_vertically_on_load(false);

    shader.use();
    shader.setVec3("dirLight.direction", -0.2, -1.0, -0.2);
    shader.setVec3("dirLight.ambient", 0.05, 0.05, 0.05);
    shader.setVec3("dirLight.diffuse", 0.4, 0.4, 0.4);
    shader.setVec3("dirLight.specular", 1.0, 1.0, 1.0);
    shader.setFloat("material.shininess", 64);

    shader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("spotLight.diffuse", 0.5f, 0.5f, 0.5f);
    shader.setVec3("spotLight.specular", 1.0, 1.0, 1.0);

    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5)));
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);

    // RENDER LOOP:
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);
        
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        setupShader(shader);
        shader.setVec3("lightColor", glm::vec3(1.0, 1.0, 1.0));
        shader.setVec3("spotLight.position", camera.Position);
        shader.setVec3("spotLight.direction", camera.Front);

        shader.setBool("blinn", blinn);
        shader.setMat4("model", glm::mat4(1.0));

        drawModel(bagpag, shader, glm::vec3(0, -3, -5), 1);

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void drawSkybox(Shader shader, int vao, int texture, glm::mat4 view)
{
    glDepthMask(GL_FALSE);
    shader.use();
    setupShader(shader);
    shader.setMat4("view", view);
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
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
    shader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom), 1200.0f / 900.0f, 0.1f, 100.0f));
    shader.setVec3("viewPos", camera.Position);
}

void drawScene(Shader mainShader, Shader transShader, Model bagpag, glm::mat4 viewMatrix, vector<glm::vec3> transObjs, int transVAO, int transTexture, int skyboxVAO, int skyboxTexture)
{
    // First, draw the bagpags
    mainShader.use();
    mainShader.setMat4("view", viewMatrix);
    mainShader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom), 1200.0f / 900.0f, 0.1f, 100.0f));
    mainShader.setVec3("viewPos", camera.Position);
    mainShader.setVec3("lightColor", glm::vec3(1.0, 1.0, 1.0));
    mainShader.setVec3("spotLight.position", camera.Position);
    mainShader.setVec3("spotLight.direction", camera.Front);

    glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should pass the stencil test
    glStencilMask(0xFF); // enable writing to the stencil buffer
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    //glDrawArrays(GL_TRIANGLES, 0, 36);

    drawModel(bagpag, mainShader, glm::vec3(0.0, 0.0, 0.0), 0.5);
    drawModel(bagpag, mainShader, glm::vec3(3.0, 0.0, -5.0), 0.5);

    // Then, draw transparencies
    glFrontFace(GL_CW);
    std::map<float, glm::vec3> sorted;
    for (unsigned int i = 0; i < transObjs.size(); i++)
    {
        float distance = glm::length(camera.Position - transObjs[i]);
        sorted[distance] = transObjs[i];
    }

    transShader.use();
    transShader.setMat4("view", viewMatrix);
    transShader.setMat4("projection", glm::perspective(glm::radians(camera.Zoom), 1200.0f / 900.0f, 0.1f, 100.0f));
    glBindVertexArray(transVAO);
    glBindTexture(GL_TEXTURE_2D, transTexture);

    for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        glm::mat4 model = glm::mat4(1.0);
        model = glm::translate(model, it->second);
        transShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) 
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else 
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}