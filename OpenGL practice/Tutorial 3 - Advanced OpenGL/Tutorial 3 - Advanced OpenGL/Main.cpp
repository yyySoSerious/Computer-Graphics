#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"

//Window dimensions
unsigned int WINDOW_WIDTH = 800;
unsigned int WINDOW_HEIGHT = 600;
const unsigned int FRAMEBUFFER_WIDTH = 800;
const unsigned int FRAMEBUFFER_HEIGHT = 600;

//Change in  time between current frame and last frame
float deltaTime = 0.0f;
//camera
Camera newCamera(glm::vec3(0.0f, 0.0f, 3.0f));

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

unsigned int textureFromFile(const char* textureFile);
void processInput(GLFWwindow* window, Shader& shader, Camera& camera);
void drawTwoContainers(GLuint cubeVAO, const Shader& shader, float scale);
void createFBO(unsigned int& framebuffer, unsigned int& texColorBuffer);
void createMSAA_FBO();

int main(int argc, char* argv[]) {
    //initialize glfw and set context options
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Create window and its associated context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //sets the frambuffer resize callbaclk for the specified window
    glfwSetCursorPosCallback(window, mouse_callback); //registers the mouse_callback function for mouse events
    glfwSetScrollCallback(window, scroll_callback); // registers the scroll_callack function for mouse scroll events

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    //--------------------------------------------------------------------

    //load shaders
    Shader shader = Shader("depthTesting.vert", "depthTesting.frag");
    Shader shaderSingleColor = Shader("depthTesting.vert", "shaderSingleColor.frag");
    Shader screenShader = Shader("screenShader.vert", "KernelEffectsShader.frag");
    //--------------------------------------------------------------

    float cubeVertices[] = {
        // back face CW
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-right
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-left    
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left              
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-right                
        // front face CW
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right  
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right      
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left        
        // left face CW
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left       
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
        // right face CW
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right      
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right          
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
        // bottom face  CW      
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left        
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
        // top face CW
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right                 
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // bottom-left  
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f  // top-left              
    };

    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f, //bottom-right
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f, //bottom-left
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f, //top-left

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f, //bottom-right
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f, //top-left
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f //top-right
    };

    //grass data
    float grassQuad[] = {
        //position                  texCoords
        -0.5f,  0.5f,   0.0f,       0.0f, 1.0f, //top-left
        -0.5f, -0.5f,   0.0f,       0.0f, 0.0f, //bottom-left
         0.5f,  0.5f,   0.0f,       1.0f, 1.0f,  //top-right
        0.5f,  -0.5f,   0.0f,       1.0f, 0.0f  //bottom-right
    };

    int grassIndices[] = {
        0, 1, 2,
        2, 1, 3
    };

    //quad for the screen
    float screenQuadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f, //top-left
         1.0f, -1.0f,  1.0f, 0.0f, //bottom-right
        -1.0f, -1.0f,  0.0f, 0.0f, //bottom-left

        -1.0f,  1.0f,  0.0f, 1.0f, //top-left
         1.0f,  1.0f,  1.0f, 1.0f,  //top-right
         1.0f, -1.0f,  1.0f, 0.0f //bottom-right
    };

    std::vector<glm::vec3> vegetation;
    vegetation.push_back(glm::vec3(-1.0f, 0.0f, -0.48f));
    vegetation.push_back(glm::vec3(2.0f, 0.0f, 0.51f));
    vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));
    //--------------------------------------------------------

    //create fbo
    unsigned int framebuffer, texColorBuffer;
    createFBO(framebuffer, texColorBuffer); //put the newly created frame buffer and color attachment in the args

    //screen quad VAO
    unsigned int screenQuadVAO, screenQuadVBO;
    glGenVertexArrays(1, &screenQuadVAO);
    glGenBuffers(1, &screenQuadVBO);

    glBindVertexArray(screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), &screenQuadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    //------------------------------------------------------------------------------

    //cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    //----------------------------------------

    //plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    //---------------------------------

    //vegetation vao
    unsigned int vegetationVAO, vegetationVBO, vegetationEBO;
    glGenVertexArrays(1, &vegetationVAO);
    glGenBuffers(1, &vegetationVBO);
    glGenBuffers(1, &vegetationEBO);

    glBindVertexArray(vegetationVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassQuad), grassQuad, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vegetationEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grassIndices), grassIndices, GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //unbinds ebo
    //--------------------------------------------------------

    //load textures
    unsigned int cubeTexture = textureFromFile("../../Textures/container.jpg"); //textureFromFile("../../Textures/iron_texture.jpg");
    unsigned int floorTexture = textureFromFile("../../Textures/marble.jpg");
    unsigned int grassTexture = textureFromFile("../../Textures/grass.png");
    unsigned int windowTexture = textureFromFile("../../Textures/blending_transparent_window.png");
    //-----------------------------------------------

    //main render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f; //The time of last frame
        float currentFrame = glfwGetTime(); //current time
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, shader, newCamera);

        //first pass
        glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        shader.activateShader();

        //draws floor
        glStencilMask(0x00); //disable writing to the stencil buffer, prevents floor from affecting the rendering of the cube border
        glBindVertexArray(planeVAO);
        shader.setUniformMatrix4("model", glm::mat4(1.0f));


        glActiveTexture(GL_TEXTURE0); //sets texture unit as the current active texture unit
        glBindTexture(GL_TEXTURE_2D, floorTexture);//binds texture objects to the current active texture unit in texture target
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //--------------------------------------------------

        //draw cubes
        glStencilFunc(GL_ALWAYS, 1, 0xFF); // all cube fragments  passes the stencil test
        glStencilMask(0xFF); //enable writing to the stencil buffer
        glBindTexture(GL_TEXTURE_2D, cubeTexture); //binds texture objects to the current active texture unit in texture target
        drawTwoContainers(cubeVAO, shader, 1);
        //----------------------------------------------------

        //draw cube outlines
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF); //only border fragments passes stencil test
        //glStencilMask(0x00);//disable writing to the stencil buffer
        glDisable(GL_DEPTH_TEST);//disable depth test, prevents it from being overwritten by the floor
        shaderSingleColor.activateShader();
        drawTwoContainers(cubeVAO, shaderSingleColor, 1.1f);
        //-----------------------------------------------------------

        //glStencilMask(0xFF); //enable writing to stencil buffer
        //glStencilFunc(GL_ALWAYS, 1, 0xFF); //reset back to all fragment passing the shader

        //draw windows using contents in vegetation VAO
        shader.activateShader();
        glDisable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glBindTexture(GL_TEXTURE_2D, windowTexture);
        glBindVertexArray(vegetationVAO);
        std::sort(vegetation.begin(), vegetation.end(), [](const glm::vec3& element1, const glm::vec3& element2)->bool {
            return glm::length2(newCamera.getEye() - element1) > glm::length2(newCamera.getEye() - element2);
            });
        for (unsigned int i = 0; i < vegetation.size(); ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, vegetation[i]);
            shader.setUniformMatrix4("model", model);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        glEnable(GL_CULL_FACE);
        //--------------------------------------------------------------------

        //-----------------------------------------------------------------------------------

        //second pass
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //back to default
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.activateShader();
        glBindVertexArray(screenQuadVAO);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //----------------------------------------------

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents(); //poll IO events(keys pressed/released, mouse moved etc.)
    }

    //clean up resources
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &vegetationVAO);
    glDeleteVertexArrays(1, &screenQuadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &vegetationVBO);
    glDeleteBuffers(1, &vegetationEBO);
    glDeleteBuffers(1, &screenQuadVBO);

    glfwTerminate();
    //-----------------------------------------------

    return 0;
}

//This function generates a texture object, sets its texture parameters and loads the texture image for the object.
//It returns the initalized texture object.
//Note: it expects the image to be RGB or RGBA format
unsigned int textureFromFile(const char* textureFile) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID); //binds texture to target, GL_TEXTURE_2D

    //load image from file
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, 0);

    //create texture image for the bound texture object and generate mipmaps from the now attached texture image
    if (data) {
        GLenum format = GL_RGB;
        switch (nrChannels) {
        case 1:
            format = GL_RED;
            break;
        case 4:
            format = GL_RGBA;
            break;
        }

        glBindTexture(GL_TEXTURE_2D, textureID); //binds texture to target, GL_TEXTURE_2D
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        //sets texture wrapping and filtering options for the currently bound texture object
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        //When using transparent textures you don't want to repeat
       // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
       // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //---------------------------------------------------------------------

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "ERROR: Cannot load texture file: " << textureFile << std::endl;
    }

    //free texture data
    stbi_image_free(data);
    return textureID;
}

//This function generates a cubemap object, sets its texture parameters and loads the cubemap faces  for the object.
//It returns the initalized cubemap object.
//Note: it expects the image to be RGB or RGBA format
unsigned int cubeMapFromfile(const std::vector<const char*> &faces){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); ++i) {
        //load texture face
        unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);

        //create texture image for each face of the cubemap
        if (data) {
            GLenum format = GL_RGB;
            switch (nrChannels) {
            case 1:
                format = GL_RED;
                break;
            case 4:
                format = GL_RGBA;
                break;
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        }
        else {
            std::cout << "ERROR: Cannot load cubemap texture file: " << faces[i] << std::endl;
        }
        //--------------------------------------------------------------------------------------------------------------------

        //set cubemap texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        //------------------------------------------------------------------------------

        //free texture data
        stbi_image_free(data);

        return textureID;
    }
}

/* This function processes input events
*   Parameters:
*       window: Current window
*       shader: Current shader
*       camera: Current camera
* */
void processInput(GLFWwindow* window, Shader& shader, Camera& camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //vary how much one texture is visible over the other
    static float factor = 0.2f;
    if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) && factor > 0.0f)
        factor -= 0.0005f;
    else if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) && factor < 1.0f)
        factor += 0.0005f;
    glUniform1f(glGetUniformLocation(shader.getProgramId(), "factor"), factor);

    //handle camera movement events
    const float cameraSpeed = 2.5f;
    const float distanceMoved = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveForward(distanceMoved);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveBackward(distanceMoved);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveLeft(distanceMoved);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight(distanceMoved);
}

/** This callback is called when the frambuffer of the specifed window resizes
*   Parameters:
*       window: Current window
*       width: Width after the window resizes
*       height: Height after the window resizes
* */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //glViewport(0, 0, width, height);
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
}

/*  This call back is called when there is mouse movement on the screen
*   Parameters:
*       window: Current window
*       xPos, yPos: Current mouse position
* */
void mouse_callback(GLFWwindow* window, double mouseX, double mouseY) {
    static double lastMouseX = mouseX;
    static double lastMouseY = mouseY;

    double xOffset = mouseX - lastMouseX;
    double yOffset = lastMouseY - mouseY;
    lastMouseX = mouseX;
    lastMouseY = mouseY;

    newCamera.processMouseMovement(xOffset, yOffset);
}

/*  This call back is called when the mouse is scrolled
*   Parameters:
*       window: Current window
*       xOffset: The scoll offset along the x-axis
*       yOffset: The scroll offset along the y-axis
* */
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    newCamera.processMouseScroll(yOffset);
}

void drawTwoContainers(GLuint cubeVAO, const Shader& shader, float scale) {
    //setting up projection matrix and passing it to the shader
    glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    shader.setUniformMatrix4("projection", projection);
    //---------------------------------------------------------------

    //setting up view matrix and passing it to the shader
    glm::mat4 view = newCamera.getViewMatrix();
    shader.setUniformMatrix4("view", view);
    //-----------------------------------------------------------------
    //draws cubes
    glBindVertexArray(cubeVAO);

    //cube 1
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    shader.setUniformMatrix4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //cube 2
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    shader.setUniformMatrix4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //---------------------------------------------------

    glBindVertexArray(0);
}

void createFBO(unsigned int& framebuffer, unsigned int& texColorBuffer) {
    //setup framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //generate texture as color attachement
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);//unnbind color buffer from target
    //---------------------------------------

    //attach the newly-generated color buffer to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    //create renderbuffer object and allocate space for it
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);  //unbind rbo
    //--------------------------------------------------------

    //attach the renderbuffer object to the currently-bound object
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    //check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    //----------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbined framebuffer
    //---------------------------------------------------------------------------------------------
}

void createMSAA_FBO() {

}


