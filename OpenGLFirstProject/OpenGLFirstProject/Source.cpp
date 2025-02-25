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
void drawScene(Shader mainShader, Shader transShader, Model bagpag, glm::mat4 viewMatrix, vector<glm::vec3> transObjs, int transVAO, int transTexture, int skyboxVAO, int skyboxTexture);
unsigned int loadCubemap(vector<std::string> faces);
void drawSkybox(Shader shader, int vao, int texture, glm::mat4 view);

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
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in NDC
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    float mirrorVertices[] = { // again, in NDC
        -0.4, 0.9,  0.0, 1.0,
        -0.4, 0.5,  0.0, 0.0,
         0.4, 0.5,  1.0, 0.0,

        -0.4, 0.9,  0.0, 1.0,
         0.4, 0.5,  1.0, 0.0,
         0.4, 0.9,  1.0, 1.0
    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    float pointsVertices[] = {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    // Set up our shaders
    Shader ourShader("./shader.vert", "./shader.frag", "./identityShader.geom");
    Shader normalShader("./normalShader.vert", "./normalShader.frag", "./normalShader.geom");
    /*Shader lightShader("./lightsource.vert", "./lightsource.frag");
    Shader outlineShader("./shader.vert", "./shaderSingleColor.frag");
    Shader transShader("./transparentShader.vert", "./transparentShader.frag");
    Shader screenShader("./quadShader.vert", "./quadShader.frag");
    Shader skyboxShader("./skybox.vert", "./skybox.frag");*/
    Shader pointsShader("./points.vert", "./points.frag", "./points.geom");

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
    ourShader.setVec3("dirLight.ambient", 1.0, 1.0, 1.0);
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

    //// Set up transparent rendering
    //vector<glm::vec3> transparentObjs;
    //transparentObjs.push_back(glm::vec3(-1.5f, -0.5f, -0.48f));
    //transparentObjs.push_back(glm::vec3( 1.5f, -0.5f,  0.51f));
    //transparentObjs.push_back(glm::vec3( 0.0f, -0.5f,  0.7f));
    //transparentObjs.push_back(glm::vec3(-0.3f, -0.5f, -2.3f));
    //transparentObjs.push_back(glm::vec3( 0.5f, -0.5f, -0.6f));

    //unsigned int transparentVAO, transparentVBO;
    //glGenVertexArrays(1, &transparentVAO);
    //glGenBuffers(1, &transparentVBO);
    //glBindVertexArray(transparentVAO);
    //glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(billboardVertices), billboardVertices, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    //glBindVertexArray(0);
    //
    //unsigned int transparentTexture;
    //glGenTextures(1, &transparentTexture);
    //glBindTexture(GL_TEXTURE_2D, transparentTexture);
    //int width, height, nrChannels;
    //unsigned char* data = stbi_load("textures/blending_transparent_window.png", &width, &height, &nrChannels, 0);
    //if (data)
    //{
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //    glGenerateMipmap(GL_TEXTURE_2D);
    //}
    //else
    //{
    //    std::cout << "Failed to load texture grass.png" << std::endl;
    //}
    //stbi_image_free(data);

    //transShader.use();
    //transShader.setInt("texture1", 0);

    //// Set up a framebuffer
    //unsigned int framebuffer;
    //glGenFramebuffers(1, &framebuffer);
    //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //// generate texture
    //unsigned int textureColorbuffer;
    //glGenTextures(1, &textureColorbuffer);
    //glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1200, 900, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glBindTexture(GL_TEXTURE_2D, 0);

    //// attach it to currently bound framebuffer object
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    //// create the renderbuffer object, for depth + stencil buffer
    //unsigned int rbo;
    //glGenRenderbuffers(1, &rbo);
    //glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1200, 900);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    //if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //// set up the VAO to use as drawing the quad to the screen
    //unsigned int quadVAO, quadVBO;
    //glGenVertexArrays(1, &quadVAO);
    //glGenBuffers(1, &quadVBO);
    //glBindVertexArray(quadVAO);
    //glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    //glBindVertexArray(0);

    //unsigned int mirrorVAO, mirrorVBO;
    //glGenVertexArrays(1, &mirrorVAO);
    //glGenBuffers(1, &mirrorVBO);
    //glBindVertexArray(mirrorVAO);
    //glBindBuffer(GL_ARRAY_BUFFER, mirrorVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(mirrorVertices), mirrorVertices, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    //glBindVertexArray(0);

    //// Set up the cubemap/skybox
    //vector<std::string> faces
    //{
    //    "textures/skybox/right.jpg",
    //    "textures/skybox/left.jpg",
    //    "textures/skybox/top.jpg",
    //    "textures/skybox/bottom.jpg",
    //    "textures/skybox/front.jpg",
    //    "textures/skybox/back.jpg"
    //};
    //unsigned int cubemapTexture = loadCubemap(faces);

    //unsigned int skyboxVAO, skyboxVBO;
    //glGenVertexArrays(1, &skyboxVAO);
    //glGenBuffers(1, &skyboxVBO);
    //glBindVertexArray(skyboxVAO);
    //glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Set up the points
    pointsShader.use();
    unsigned int pointsVAO, pointsVBO;
    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointsVertices), pointsVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);
        
        ourShader.use();

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        /*pointsShader.use();
        glBindVertexArray(pointsVAO);
        glDrawArrays(GL_POINTS, 0, 4);*/

        // first pass - bind to our custom framebuffer
        //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        //glClearColor(0.1, 0.1, 0.1, 1.0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_DEPTH_TEST);

        //// draw the skybox FIRST
        //drawSkybox(skyboxShader, skyboxVAO, cubemapTexture, glm::mat4(glm::mat3(camera.GetViewMatrix())));

        // then draw the scene, which will draw into our custom framebuffer
        setupShader(ourShader);
        ourShader.setVec3("lightColor", glm::vec3(1.0, 1.0, 1.0));
        ourShader.setVec3("spotLight.position", camera.Position);
        ourShader.setVec3("spotLight.direction", camera.Front);
        drawModel(bagpag, ourShader, glm::vec3(0.0, 0.0, 0.0), 1.0);
        normalShader.use();
        setupShader(normalShader);
        drawModel(bagpag, normalShader, glm::vec3(0.0, 0.0, 0.0), 1.0);
        //drawScene(ourShader, transShader, bagpag, camera.GetViewMatrix(), transparentObjs, transparentVAO, transparentTexture, skyboxVAO, cubemapTexture);

        //// second pass - swap back to the default framebuffer, then draw the quad over top of the whole screen
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glDisable(GL_DEPTH_TEST);
        //glClear(GL_COLOR_BUFFER_BIT);

        //screenShader.use();
        //glBindVertexArray(quadVAO);
        //glBindTexture(GL_TEXTURE_2D, textureColorbuffer); // this is what we wrote to in our custom framebuffer
        //glDrawArrays(GL_TRIANGLES, 0, 6);

        // third pass - back to the custom buffer, this time draw behind us
        //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //drawSkybox(skyboxShader, skyboxVAO, cubemapTexture, glm::mat4(glm::mat3(camera.GetViewMatrix_Behind())));
        //glEnable(GL_DEPTH_TEST);
        //drawScene(ourShader, transShader, bagpag, camera.GetViewMatrix_Behind(), transparentObjs, transparentVAO, transparentTexture, skyboxVAO, cubemapTexture);

        //// fourth pass - draw the behind-us view as a rear-view mirror type dealie
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glDisable(GL_DEPTH_TEST);

        //screenShader.use();
        //glBindVertexArray(mirrorVAO);
        //glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        //glDrawArrays(GL_TRIANGLES, 0, 6);

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