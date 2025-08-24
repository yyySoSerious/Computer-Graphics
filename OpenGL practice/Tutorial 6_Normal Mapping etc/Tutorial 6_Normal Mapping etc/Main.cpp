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
#include <ctime>
#include <random>
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"

//Window dimensions
unsigned int WINDOW_WIDTH = 800;
unsigned int WINDOW_HEIGHT = 600;
const unsigned int FRAMEBUFFER_WIDTH = 800;
const unsigned int FRAMEBUFFER_HEIGHT = 600;
const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

//Change in  time between current frame and last frame
float deltaTime = 0.0f;
//camera
Camera newCamera(glm::vec3(0.0f, 0.0f, 3.0f));
bool GAMMA_ENABLED = false; //gamma option
bool NORMAL_MAPPING = false; //normal mapping option
bool PARALLAX_MAPPING = false; //parallax mapping option
bool BLOOM_ENABLED = false; //bloom option
bool AO_ENABLED = false; //ambient occlusion option

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void processInput(GLFWwindow* window, Camera& camera);
void drawTwoContainers(GLuint cubeVAO, const Shader& shader, const glm::mat4& projection, const glm::mat4& view, float scale);
void drawCube(GLuint cubeVAO, const Shader& shader, const glm::vec3 translationVec = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), glm::mat4 rotationMatrix = glm::mat4(1.0));
void createFBO(unsigned int& framebuffer, unsigned int* colorBuffer, unsigned int numColorBuffers, GLint colorBuffer_internalFormat, bool hasDepthbuffer = true);
void createFBO(unsigned int& framebuffer, unsigned int* colorBuffer, unsigned int numColorBuffers, GLint* colorBuffer_internalFormat, bool hasDepthBuffer = true);
void createFBO(unsigned int& framebuffer, unsigned int& texColorBuffer, GLint colorBuffer_internalFormat, bool hasDepthbuffer = true);
void createDepthMapFBO(unsigned int& depthMapFBO, unsigned int& depthMap);
void createDepthCubeMapFBO(unsigned int& depthCubeMapFBO, unsigned int& depthCubeMap);
void createMSAA_FBO(unsigned int& multisampledFBO, unsigned int& msaa_texColorBuffer, unsigned int numSamples);
void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);
void setObjectTangentsandBitangents(std::vector<glm::vec3>& tangentsAndBitangents, const float* objData,
    const unsigned int objDataSize, const unsigned int vertexStride, const unsigned int uvOffset); 
void renderRandomScene(unsigned int cubeVAO, unsigned int planeVAO, unsigned int lightObjectVAO, unsigned int uboMatrices);
void renderTunnelScene(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices);
void renderSceneWithBloomEffect(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices);
void renderSceneWithDeferredShading(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices);
void renderSceneWithSSAO(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices);
void createSphere(unsigned int xSegments, unsigned int ySegments, unsigned int& sphereVAO, unsigned int& indicesSize);
void drawSphere(unsigned int xSegs = 64, unsigned int ySegs = 64);
void PBR_directLighting(unsigned int uboMatrices);
void renderEquirectangularMap_withPBR(unsigned int cubeVAO, unsigned int uboMatrices);

int main(int argc, char* argv[]) {
    const unsigned int NUM_SAMPLES = 4;

    //initialize glfw and set context options
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    //glfwWindowHint(GLFW_SAMPLES, NUM_SAMPLES); //increasing number of samples per pixel to reduce aliasing

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
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //configure global opengl state 
    //---------------------------------------------------------------------
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(messageCallback, "Learn OpenGL application");
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    //glEnable(GL_MULTISAMPLE); //enable multisampling anti-aliasing(MSAA)
    //--------------------------------------------------------------------
    //---------------------------------------------------------------------

    //cube data
    float cubeVertices[] = {
        //position              normal                  texCoord
        // back face CW
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      1.0f, 0.0f, // bottom-right
         0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      0.0f, 0.0f,// bottom-left    
         0.5f,  0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      0.0f, 1.0f,// top-left              
         0.5f,  0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      0.0f, 1.0f, // top-left
        -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      1.0f, 1.0f, // top-right
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,      1.0f, 0.0f,// bottom-right                
        // front face CW
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 0.0f,// bottom-left
         0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       1.0f, 1.0f,// top-right  
         0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       1.0f, 0.0f,// bottom-right      
         0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       1.0f, 1.0f,// top-right
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 0.0f,// bottom-left
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 1.0f,// top-left        
        // left face CW
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,     1.0f, 1.0f,// top-right
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,     0.0f, 0.0f,// bottom-left
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,     0.0f, 1.0f,// top-left       
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,     0.0f, 0.0f,// bottom-left
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,     1.0f, 1.0f,// top-right
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,     1.0f, 0.0f,// bottom-right
        // right face CW
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,     0.0f, 1.0f,// top-left
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,     1.0f, 1.0f,// top-right      
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,     1.0f, 0.0f,// bottom-right          
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,     1.0f, 0.0f,// bottom-right
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,     0.0f, 0.0f,// bottom-left
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,     0.0f, 1.0f,// top-left
        // bottom face  CW      
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     1.0f, 1.0f,// top-right
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,     0.0f, 0.0f,// bottom-left
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     0.0f, 1.0f,// top-left        
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,     0.0f, 0.0f,// bottom-left
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     1.0f, 1.0f,// top-right
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,     1.0f, 0.0f,// bottom-right
        // top face CW
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f, 1.0f,// top-left
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     1.0f, 1.0f,// top-right
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,     1.0f, 0.0f,// bottom-right                 
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,     1.0f, 0.0f,// bottom-right
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,     0.0f, 0.0f,// bottom-left  
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f, 1.0f// top-left              
    };
    std::vector<glm::vec3> cubeTangentsAndBitangents;
    unsigned int cubeVerticesLength = sizeof(cubeVertices) / sizeof(float);
    unsigned int vertexStride = 8; //stride to next vertex
    unsigned int uvOffset = 6; //offset to tex Coords from beginning of a vertex
    setObjectTangentsandBitangents(cubeTangentsAndBitangents, cubeVertices, cubeVerticesLength, vertexStride, uvOffset);
    //--------------------------------------------------------------------------------------------------------------------------

    //plane data
    float planeVertices[] = {
        //positions             normal                  texCoords
         5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,       2.0f, 0.0f,//bottom-right
        -5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,       0.0f, 0.0f,//bottom-left
        -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,       0.0f, 2.0f,//top-left

         5.0f, -0.5f,  5.0f,    0.0f, 1.0f, 0.0f,       2.0f, 0.0f,//bottom-right
        -5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,       0.0f, 2.0f,//top-left
         5.0f, -0.5f, -5.0f,    0.0f, 1.0f, 0.0f,       2.0f, 2.0f//top-right
    };
    std::vector<glm::vec3> planeTangentsAndBitangents;
    unsigned int planeVerticesLength = sizeof(planeVertices) / sizeof(float);
    setObjectTangentsandBitangents(planeTangentsAndBitangents, planeVertices, planeVerticesLength, vertexStride, uvOffset);
    //----------------------------------------------------------------------------------------------------------------------

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
    unsigned int cubeVAO, cubeVBO, cubeTB_VBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeTB_VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    //position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    //normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    //uv attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, cubeTB_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * cubeTangentsAndBitangents.size(), cubeTangentsAndBitangents.data(), GL_STATIC_DRAW);

    //tangent attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    //bitangent attribute
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    //----------------------------------------------------------------------------------------------------
    
    //sphere VAO

    //plane VAO
    unsigned int planeVAO, planeVBO, planeTB_VBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glGenBuffers(1, &planeTB_VBO);

    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    //position atribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    //normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    //uv attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));


    glBindBuffer(GL_ARRAY_BUFFER, planeTB_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)* planeTangentsAndBitangents.size(), planeTangentsAndBitangents.data(), GL_STATIC_DRAW);

    //tangent attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    //bitangent attribute
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    //-------------------------------------------------------------------------------------------

    //light VAO
    unsigned int lightObjectVAO;
    glGenVertexArrays(1, &lightObjectVAO);
    glBindVertexArray(lightObjectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); //VBO already contains the data so no need add data with glBufferData()

    //link text coords attribute in the vertex data to the shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //------------------------------------------------------------------------------------------------

    //create ubo buffer
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); ///alllocates uniform block data
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //----------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------

    //main render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f; //The time of last frame
        float currentFrame = glfwGetTime(); //current time
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, newCamera);
        //renderRandomScene(cubeVAO, planeVAO, lightObjectVAO, uboMatrices);
        //renderTunnelScene(cubeVAO, lightObjectVAO, screenQuadVAO, uboMatrices);
        //renderSceneWithBloomEffect(cubeVAO, lightObjectVAO, screenQuadVAO, uboMatrices);
        //renderSceneWithDeferredShading(cubeVAO, lightObjectVAO, screenQuadVAO, uboMatrices);
        //renderSceneWithSSAO(cubeVAO, lightObjectVAO, screenQuadVAO, uboMatrices);
        //PBR_directLighting(uboMatrices);
        renderEquirectangularMap_withPBR(cubeVAO, uboMatrices);

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents(); //poll IO events(keys pressed/released, mouse moved etc.)
    }

    //clean up resources
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &screenQuadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeTB_VBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &planeTB_VBO);
    glDeleteBuffers(1, &screenQuadVBO);

    glfwTerminate();
    //-----------------------------------------------

    return 0;
}

// you should destroy vertex array and vertex buffer objects
void createSphere(unsigned int xSegments, unsigned int ySegments, unsigned int& sphereVAO, unsigned int& indicesSize) {
    unsigned int sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    std::vector<float> vertexData;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359;
    //for each point on the longitude, find the equivalent point on the latitude
    for (unsigned int y = 0; y <= ySegments; ++y) {
        //The first and last vertices have same position and normal, but different tex coords
        for (unsigned int x = 0; x <= xSegments; ++x) {
            float ySegment = (float)y / ySegments;
            float xSegment = (float)x / xSegments;
            float xPos = std::sin(ySegment * PI) * std::cos(xSegment * 2.0f * PI);  //sin(phi) * cos(theta)
            float yPos = std::cos(ySegment * PI); //cos(phi)
            float zPos = std::sin(ySegment * PI) * std::sin(xSegment * 2.0f * PI); //sin(phi) * sin(theta)

            //vertex's position
            vertexData.push_back(xPos);
            vertexData.push_back(yPos);
            vertexData.push_back(zPos);

            //vertex's texture coordinates
            vertexData.push_back(xSegment);
            vertexData.push_back(ySegment); //v-coordinate is flipped

            //vertex's normal
            vertexData.push_back(xPos);
            vertexData.push_back(yPos);
            vertexData.push_back(zPos);
        }
    }

    /*for (unsigned int y = 0; y < ySegments; ++y) {
        for (unsigned int x = 0; x < xSegments; ++x) {
            unsigned int currVertex = y * (xSegments + 1) + x;
            unsigned int belowCurrVertex = (y + 1) * (xSegments + 1) + x;

            //triangle 1
            indices.push_back(currVertex);
            indices.push_back(belowCurrVertex);
            indices.push_back(currVertex + 1);

            //triangle 2
            indices.push_back(belowCurrVertex);
            indices.push_back(belowCurrVertex + 1);
            indices.push_back(currVertex + 1);
        }
    }*/

    //or

    for (unsigned int y = 0; y < ySegments; ++y) {
        for (unsigned int x = 0; x <= xSegments; ++x) {
            indices.push_back(y * (xSegments + 1) + x);
            indices.push_back((y + 1) * (xSegments + 1) + x);
        }
    }
    indicesSize = indices.size();

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    unsigned int stride = 8 * sizeof(float);

    //position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    //texture coordinates attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    //normal attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
}

void drawSphere(unsigned int xSegs, unsigned int ySegs) {
    static bool initialized = false;
    static unsigned int xSegments = xSegs;
    static unsigned int ySegments = ySegs;
    static unsigned int sphereVAO = 0;
    static unsigned int indicesSize = 0;
    if (!initialized || xSegments != xSegs || ySegments != ySegs) {
        createSphere(xSegments = xSegs, ySegments = ySegs, sphereVAO, indicesSize);
        initialized = true;
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indicesSize, GL_UNSIGNED_INT, 0);
}
/* This function processes input events
*   Parameters:
*       window: Current window
*       shader: Current shader
*       camera: Current camera
* */
void processInput(GLFWwindow* window, Camera& camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    //vary how much one texture is visible over the other
   /* static float factor = 0.2f;
    if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) && factor > 0.0f)
        factor -= 0.0005f;
    else if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) && factor < 1.0f)
        factor += 0.0005f;
    glUniform1f(glGetUniformLocation(shader.getProgramId(), "factor"), factor);*/

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

/*  Callback to process key events
*   Parameters:
*       window:	    The window that received the event.
*       key:	    The keyboard key that was pressed or released.
*       scancode:	The system - specific scancode of the key.
*       action: 	GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.Future releases may add more actions.
*       mods:   	Bit field describing which modifier keys were held down.
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    static bool captured = true;
    //capture mouse option
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        if (captured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            captured = false;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            captured = true;
        }
    }

    //gamma option
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        if (GAMMA_ENABLED) {
            GAMMA_ENABLED = false;
        }
        else {
            GAMMA_ENABLED = true;
        }
    }

    //normal mapping option
    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        if (NORMAL_MAPPING) {
            NORMAL_MAPPING = false;
        }
        else {
            NORMAL_MAPPING = true;
        }
    }

    //parallax mapping option
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        if (PARALLAX_MAPPING) {
            PARALLAX_MAPPING = false;
        }
        else {
            PARALLAX_MAPPING = true;
        }
    }
    
    //bloom option
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        if (BLOOM_ENABLED) {
            BLOOM_ENABLED = false;
        }
        else {
            BLOOM_ENABLED = true;
        }
    }
    //Ambient occlusion option
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        if (AO_ENABLED) {
            AO_ENABLED = false;
        }
        else {
            AO_ENABLED = true;
        }
    }
}
float lerp(float a, float b, float f) {
    return a * (1 - f) + b * f;
}
void renderSceneWithSSAO(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices) {
    static bool initialized = false;
    static glm::mat4 identityMatrix = glm::mat4(1.0);
    //------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------INITIALIZATION-------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //load model into object
    static Model modelObject = Model("../../Models/backpack/backpack.obj");

    //initialize light parameters
    //--------------------------------------------------------------------------------------------------------
    static std::vector<PointLight> pointLights = {
            PointLight(glm::vec3(0.0f,  0.5f, 1.5f), glm::vec3(0.2f), glm::vec3(1.0f, 1.0f, 1.0f)),
            PointLight(glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(0.2f), glm::vec3(2.0f, 0.0f, 0.0f)),
            PointLight(glm::vec3(3.0f, 0.5f, 1.0f), glm::vec3(0.2f), glm::vec3(0.0f, 0.0f, 3.0f)),
            PointLight(glm::vec3(-0.8f, 2.4f, -1.0f), glm::vec3(0.2f), glm::vec3(0.0f, 1.0f, 0.0f)),
    };
    //--------------------------------------------------------------------------------------------------------

    //initialize shaders
    //--------------------------------------------------------------------------------------------------------
    static Shader SSAOGeometryPassShader = Shader("Shaders/SSAOGeometryPass.vert", "Shaders/SSAOGeometryPass.frag");
    static Shader SSAOShader = Shader("Shaders/SSAO.vert", "Shaders/SSAO.frag");
    static Shader SSAOBlurShader = Shader("Shaders/SSAO.vert", "Shaders/SSAOBlur.frag");
    static Shader SSAOLightingPassShader= Shader("Shaders/deferredMultipleLightingPass.vert", "Shaders/SSAOLightingPass.frag");
    static Shader ScreenShader = Shader("Shaders/screenShader.vert", "Shaders/screenShader.frag");
    //--------------------------------------------------------------------------------------------------------

    //load textures
    //--------------------------------------------------------------------------------------------------------
    static unsigned int cubeTexture = textureFromFile("../../Textures/container2.png", false);
    static unsigned int cubeTextureGammaCorrected = textureFromFile("../../Textures/container2.png", true);
    static unsigned int floorTexture = textureFromFile("../../Textures/wood.png", false);
    static unsigned int floorTextureGammaCorrected = textureFromFile("../../Textures/wood.png", true);
    static unsigned int cubeTexture_normal = textureFromFile("../../Textures/toy_box_normal.png", false);
    static unsigned int cubeTexture_depth = textureFromFile("../../Textures/toy_box_disp.png", false);
    //--------------------------------------------------------------------------------------------------------

    static unsigned int gBuffer, gPosition, gNormal, gAlbedoSpec;
    static unsigned int ssaoFBO, ssaoColorBuffer, noiseTexture;
    static unsigned int ssaoBlurFBO, ssaoColorBufferBlur;
    static std::vector<glm::vec3> ssaoKernel;
    static float kernelRadius = 0.5;
    static int noiseRadius = 4;
    static glm::vec2 noiseScale(FRAMEBUFFER_WIDTH / noiseRadius, FRAMEBUFFER_HEIGHT / noiseRadius);
    if (!initialized) {
        //setup G-buffer
        //---------------------------------------------------------------------------------------------------------
        unsigned int colorBuffers[3];
        GLint internalFormat[3] = { GL_RGBA16F, GL_RGBA16F, GL_RGBA };
        createFBO(gBuffer, colorBuffers, 3, internalFormat);
        gPosition = colorBuffers[0];
        gNormal = colorBuffers[1];
        gAlbedoSpec = colorBuffers[2];
        //---------------------------------------------------------------------------------------------------------
        //---------------------------------------------------------------------------------------------------------

        //setup SSAO framebuffer
        //---------------------------------------------------------------------------------------------------------
        createFBO(ssaoFBO, ssaoColorBuffer, GL_RED, false);
        //---------------------------------------------------------------------------------------------------------
        //---------------------------------------------------------------------------------------------------------


        //setup kernel
        //---------------------------------------------------------------------------------------------------------
        int kernelSize = 64;
        std::default_random_engine engine(static_cast<unsigned int> (time(0)));
        std::uniform_real_distribution<float> randomFloat(0.0f, 1.0f);
        for(unsigned int i = 0; i < kernelSize; ++i) {
            glm::vec3 sample(
                randomFloat(engine) * 2.0f - 1.0f,
                randomFloat(engine) * 2.0f - 1.0f,
                randomFloat(engine)
            );
            sample = glm::normalize(sample); //place all samples on the unit hemisphere
            float scale = (float) i / kernelSize;
            scale = lerp(0.1f, 1.0f, scale * scale); 
            sample *= scale;//the distance increases as the number of samples increases
            ssaoKernel.push_back(sample);
        }
        //---------------------------------------------------------------------------------------------------------
        //---------------------------------------------------------------------------------------------------------

        //setup noise texture : random rotation vectors
        //---------------------------------------------------------------------------------------------------------
        unsigned int noiseSize = noiseRadius * noiseRadius;
        std::vector<glm::vec3> ssaoNoise;
        for (unsigned int i = 0; i < noiseSize; ++i){
            glm::vec3 noise(
                randomFloat(engine) * 2.0f - 1.0f,
                randomFloat(engine) * 2.0f - 1.0f,
                0.0f
            ); //setting this to zero because we are rotating around the z-axis
            ssaoNoise.push_back(noise);
        }
        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, noiseRadius, noiseRadius, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
        //---------------------------------------------------------------------------------------------------------
        //---------------------------------------------------------------------------------------------------------
        
        //setup ssao blur fbo
        //--------------------------------------------------------------------------------------------------------
        createFBO(ssaoBlurFBO, ssaoColorBufferBlur, GL_RED, false);
        //--------------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------

        //bind ubo and shaders to a binding location
        //--------------------------------------------------------------------------------------------------------
        //bind uniform buffer object to binding point(loc) 0
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        //get relevant block indices
        unsigned int SSAOGeometryPassShader_uniformBlockIndex = glGetUniformBlockIndex(SSAOGeometryPassShader.getProgramId(), "Matrices");
        
        //link each shader's uniform block indices to uniform binding point(loc) 0
        glUniformBlockBinding(SSAOGeometryPassShader.getProgramId(), SSAOGeometryPassShader_uniformBlockIndex, 0);
        //--------------------------------------------------------------------------------------------------------

        initialized = true;
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------

    //First pass: Geometry Pass
    //---------------------------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //inserting projection and view matrices into ubo
    //--------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

    //glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

    glm::mat4 view = newCamera.getViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        //Draw point lights
        //---------------------------------------------------------------------------------------------------------------
       // lightSourceDeferredGeometryPassShader.activateShader();
       // lightSourceDeferredGeometryPassShader.setUniformVec3("lightColor", pointLights[i].diffuse);
      //  drawCube(lightObjectVAO, lightSourceDeferredGeometryPassShader, pointLights[i].position, glm::vec3(0.2, 0.2, 0.2));
        //---------------------------------------------------------------------------------------------------------------
    }

    //activate shader and pass uniforms to it
    //---------------------------------------------------------------------------------------------------------------
    SSAOGeometryPassShader.activateShader();
    SSAOGeometryPassShader.setUniformVec3("cameraPos", newCamera.getEye());
    SSAOGeometryPassShader.setUniformInt("gamma", GAMMA_ENABLED);
    SSAOGeometryPassShader.setUniformInt("normal_mapping", NORMAL_MAPPING);
    SSAOGeometryPassShader.setUniformInt("parallax_mapping", PARALLAX_MAPPING);
    SSAOGeometryPassShader.setUniformFloat("height_scale", 0.2);

    //sets material properties
    SSAOGeometryPassShader.setUniformInt("material.diffuseMap", 0);
    SSAOGeometryPassShader.setUniformInt("material.specularMap", 0); //use the floor texture as a specular map
    SSAOGeometryPassShader.setUniformInt("material.emissionMap", 2);
    SSAOGeometryPassShader.setUniformFloat("material.shininess", 64.0f);
    SSAOGeometryPassShader.setUniformInt("material.heightMap", 1);
    SSAOGeometryPassShader.setUniformInt("depthMap", 3);
    //---------------------------------------------------------------------------------------------------------------

    //draw scene
    //---------------------------------------------------------------------------------------------------------------
    modelObject.draw(SSAOGeometryPassShader);
    //draw cube as floor
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? floorTextureGammaCorrected : floorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_depth);
    drawCube(cubeVAO, SSAOGeometryPassShader, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(12.5f, 0.5f, 12.5f));

    //draw other cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? cubeTextureGammaCorrected : cubeTexture);
    drawCube(cubeVAO, SSAOGeometryPassShader, glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.5f));
    drawCube(cubeVAO, SSAOGeometryPassShader, glm::vec3(2.0f, 0.0f, 1.0f), glm::vec3(0.5f));
    glm::mat4 rotationMatrix = glm::rotate(identityMatrix, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, SSAOGeometryPassShader, glm::vec3(-1.0f, -1.0f, 2.0f), glm::vec3(1.0f), rotationMatrix);
    rotationMatrix = glm::rotate(identityMatrix, glm::radians(23.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, SSAOGeometryPassShader, glm::vec3(0.0f, 2.7f, 4.0f), glm::vec3(1.25f), rotationMatrix);
    rotationMatrix = glm::rotate(identityMatrix, glm::radians(124.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, SSAOGeometryPassShader, glm::vec3(-2.0f, 1.0f, -3.0f), glm::vec3(1.0f), rotationMatrix);
    drawCube(cubeVAO, SSAOGeometryPassShader, glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(0.5f));
    //---------------------------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    //SSAO pass: calculate ambient occlusion for each fragment
    //---------------------------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SSAOShader.activateShader();
    SSAOShader.setUniformMatrix4("projection", projection);
    SSAOShader.setUniformMatrix4("view", view);

    //set uniform sampler textures in shader
    SSAOShader.setUniformInt("gPosition", 0);
    SSAOShader.setUniformInt("gNormal", 1);
    SSAOShader.setUniformInt("noiseTexture", 2);

    //set remaining uniforms
    SSAOShader.setUniformFloat("kernelRadius", kernelRadius);
    SSAOShader.setUniformVec2("noiseScale", noiseScale);
    SSAOShader.setUniformArrayOfVec3("ssaoKernel", ssaoKernel);

    //draw screen quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    //SSAO blur pass
    //---------------------------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO); 
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SSAOBlurShader.activateShader();

    //draw screen quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    //Lighting pass: render to screen
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SSAOLightingPassShader.activateShader();

    //send light data
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        float constant = pointLights[i].constant;
        float linear = pointLights[i].linear;
        float quadratic = pointLights[i].quadratic;
        float lightMax = std::fmaxf(std::fmaxf(pointLights[i].diffuse.r, pointLights[i].diffuse.g), pointLights[i].diffuse.b);
        float radius = (-linear + std::sqrtf(linear * linear - 4.0 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
            / (2 * quadratic);
        SSAOLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].radius", radius);
        SSAOLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].constant", pointLights[i].constant);
        SSAOLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].linear", pointLights[i].linear);
        SSAOLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].quadratic", pointLights[i].quadratic);
        SSAOLightingPassShader.setUniformVec3("lightPos[" + std::to_string(i) + "]", 
            glm::vec3(view * glm::vec4(pointLights[i].position, 1.0)));
        SSAOLightingPassShader.setUniformVec3("lights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
        SSAOLightingPassShader.setUniformVec3("lights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
        SSAOLightingPassShader.setUniformVec3("lights[" + std::to_string(i) + "].specular", pointLights[i].specular);
    }

    //send remaining uniforms
    SSAOLightingPassShader.setUniformVec3("cameraPos", newCamera.getEye());
    SSAOLightingPassShader.setUniformFloat("shininess", 64.0f);
    SSAOLightingPassShader.setUniformInt("gamma", GAMMA_ENABLED);
    SSAOLightingPassShader.setUniformInt("ao", AO_ENABLED);
    SSAOLightingPassShader.setUniformInt("numLights", pointLights.size());
    SSAOLightingPassShader.setUniformInt("gPosition", 0);
    SSAOLightingPassShader.setUniformInt("gNormal", 1); //use the floor texture as a specular map
    SSAOLightingPassShader.setUniformInt("gAlbedoSpec", 2); 
    SSAOLightingPassShader.setUniformInt("ssao", 3);
    //draw screen quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------
}

void renderSceneWithDeferredShading(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices) {
    static bool initialized = false;
    static glm::mat4 identityMatrix = glm::mat4(1.0);
    //------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------INITIALIZATION-------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------

    //initialize light parameters
    //--------------------------------------------------------------------------------------------------------
    static std::vector<PointLight> pointLights = {
            PointLight(glm::vec3(0.0f,  0.5f, 1.5f), glm::vec3(0.2f), glm::vec3(5.0f, 5.0f, 5.0f)),
            PointLight(glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(0.2f), glm::vec3(10.0f, 0.0f, 0.0f)),
            PointLight(glm::vec3(3.0f, 0.5f, 1.0f), glm::vec3(0.2f), glm::vec3(0.0f, 0.0f, 15.0f)),
            PointLight(glm::vec3(-0.8f, 2.4f, -1.0f), glm::vec3(0.2f), glm::vec3(0.0f, 5.0f, 0.0f)),
    };
    //--------------------------------------------------------------------------------------------------------

    //initialize shaders
    //--------------------------------------------------------------------------------------------------------
    static Shader deferredGeometryPassShader = Shader("Shaders/deferredGeometryPass.vert", "Shaders/deferredGeometryPass.frag");
    static Shader lightSourceDeferredGeometryPassShader = Shader("Shaders/lightSourceDeferredGeometryPass.vert", "Shaders/lightSourceDeferredGeometryPass.frag");
    static Shader screenShader = Shader("Shaders/screenShader.vert", "Shaders/screenShader.frag");
    static Shader lightSourceShader = Shader("Shaders/lightSourceShader.vert", "Shaders/lightSourceShader.frag");
    static Shader deferredMultipleLightingPassShader = Shader("Shaders/deferredMultipleLightingPass.vert", "Shaders/deferredMultipleLightingPass.frag");
    //--------------------------------------------------------------------------------------------------------

    //load textures
    //--------------------------------------------------------------------------------------------------------
    static unsigned int cubeTexture = textureFromFile("../../Textures/container2.png", false);
    static unsigned int cubeTextureGammaCorrected = textureFromFile("../../Textures/container2.png", true);
    static unsigned int floorTexture = textureFromFile("../../Textures/wood.png", false);
    static unsigned int floorTextureGammaCorrected = textureFromFile("../../Textures/wood.png", true);
    static unsigned int cubeTexture_normal = textureFromFile("../../Textures/toy_box_normal.png", false);
    static unsigned int cubeTexture_depth = textureFromFile("../../Textures/toy_box_disp.png", false);
    //--------------------------------------------------------------------------------------------------------

    static unsigned int gBuffer, gPosition, gNormal, gAlbedoSpec;
    if (!initialized) {
        //initialize light range to 7
        //---------------------------------------------------------------------------------------------------------
        for (unsigned int i = 0; i < pointLights.size(); ++i) {
            pointLights[i].linear = 0.7;
            pointLights[i].quadratic = 1.8;
        }
        //---------------------------------------------------------------------------------------------------------

        //setup G-buffer
        //---------------------------------------------------------------------------------------------------------
        unsigned int colorBuffers[3];
        GLint internalFormat[3] = { GL_RGBA16F, GL_RGBA16F, GL_RGBA };
        createFBO(gBuffer, colorBuffers, 3, internalFormat);
        gPosition = colorBuffers[0];
        gNormal = colorBuffers[1];
        gAlbedoSpec = colorBuffers[2];
        //---------------------------------------------------------------------------------------------------------
        //---------------------------------------------------------------------------------------------------------

        //bind ubo and shaders to a binding location
        //--------------------------------------------------------------------------------------------------------
        //bind uniform buffer object to binding point(loc) 0
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        //get relevant block indices
        unsigned int lightSourceDeferredGeometryPassShader_uniformBlockIndex = glGetUniformBlockIndex(lightSourceDeferredGeometryPassShader.getProgramId(), "Matrices");
        unsigned int deferredGeometryPassShader_uniformBlockIndex = glGetUniformBlockIndex(deferredGeometryPassShader.getProgramId(), "Matrices");
        unsigned int lightSourceShader_uniformBlockIndex = glGetUniformBlockIndex(lightSourceShader.getProgramId(), "Matrices");

        //link each shader's uniform block indices to uniform binding point(loc) 0
        glUniformBlockBinding(lightSourceDeferredGeometryPassShader.getProgramId(), lightSourceDeferredGeometryPassShader_uniformBlockIndex, 0);
        glUniformBlockBinding(deferredGeometryPassShader.getProgramId(), deferredGeometryPassShader_uniformBlockIndex, 0);
        glUniformBlockBinding(lightSourceShader.getProgramId(), lightSourceShader_uniformBlockIndex, 0);
        //--------------------------------------------------------------------------------------------------------

        initialized = true;
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------

    //First pass: Geometry Pass
    //---------------------------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //inserting projection and view matrices into ubo
    //--------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

    //glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

    glm::mat4 view = newCamera.getViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        //Draw point lights
        //---------------------------------------------------------------------------------------------------------------
       // lightSourceDeferredGeometryPassShader.activateShader();
       // lightSourceDeferredGeometryPassShader.setUniformVec3("lightColor", pointLights[i].diffuse);
      //  drawCube(lightObjectVAO, lightSourceDeferredGeometryPassShader, pointLights[i].position, glm::vec3(0.2, 0.2, 0.2));
        //---------------------------------------------------------------------------------------------------------------

        //send pointLight uniform values for drawing objects
        //---------------------------------------------------------------------------------------------------------------
        deferredGeometryPassShader.activateShader();
        deferredGeometryPassShader.setUniformVec3("lightPos[" + std::to_string(i) + "]", pointLights[i].position);
        deferredGeometryPassShader.setUniformVec3("lights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
        deferredGeometryPassShader.setUniformVec3("lights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
        deferredGeometryPassShader.setUniformVec3("lights[" + std::to_string(i) + "].specular", pointLights[i].specular);
        //---------------------------------------------------------------------------------------------------------------
    }

    //activate shader and pass uniforms to it
    //---------------------------------------------------------------------------------------------------------------
    deferredGeometryPassShader.activateShader();
    deferredGeometryPassShader.setUniformVec3("cameraPos", newCamera.getEye());
    deferredGeometryPassShader.setUniformInt("gamma", GAMMA_ENABLED);
    deferredGeometryPassShader.setUniformInt("normal_mapping", NORMAL_MAPPING);
    deferredGeometryPassShader.setUniformInt("parallax_mapping", PARALLAX_MAPPING);
    deferredGeometryPassShader.setUniformFloat("height_scale", 0.2);
    deferredGeometryPassShader.setUniformInt("numLights", pointLights.size());

    //sets material properties
    deferredGeometryPassShader.setUniformInt("material.diffuseMap", 0);
    deferredGeometryPassShader.setUniformInt("material.specularMap", 0); //use the floor texture as a specular map
    deferredGeometryPassShader.setUniformInt("material.emissionMap", 2);
    deferredGeometryPassShader.setUniformFloat("material.shininess", 64.0f);
    deferredGeometryPassShader.setUniformInt("material.heightMap", 1);
    deferredGeometryPassShader.setUniformInt("depthMap", 3);
    //---------------------------------------------------------------------------------------------------------------

    //draw scene
    //---------------------------------------------------------------------------------------------------------------
    //draw cube as floor
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? floorTextureGammaCorrected : floorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_depth);
    drawCube(cubeVAO, deferredGeometryPassShader, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(12.5f, 0.5f, 12.5f));

    //draw other cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? cubeTextureGammaCorrected : cubeTexture);
    drawCube(cubeVAO, deferredGeometryPassShader, glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.5f));
    drawCube(cubeVAO, deferredGeometryPassShader, glm::vec3(2.0f, 0.0f, 1.0f), glm::vec3(0.5f));
    glm::mat4 rotationMatrix = glm::rotate(identityMatrix, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, deferredGeometryPassShader, glm::vec3(-1.0f, -1.0f, 2.0f), glm::vec3(1.0f), rotationMatrix);
    rotationMatrix = glm::rotate(identityMatrix, glm::radians(23.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, deferredGeometryPassShader, glm::vec3(0.0f, 2.7f, 4.0f), glm::vec3(1.25f), rotationMatrix);
    rotationMatrix = glm::rotate(identityMatrix, glm::radians(124.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, deferredGeometryPassShader, glm::vec3(-2.0f, 1.0f, -3.0f), glm::vec3(1.0f), rotationMatrix);
    drawCube(cubeVAO, deferredGeometryPassShader, glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(0.5f));
    //---------------------------------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    //Lighting pass: render to screen
    //---------------------------------------------------------------------------------------------------------------
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    deferredMultipleLightingPassShader.activateShader();

    //send light data
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        float constant = pointLights[i].constant;
        float linear = pointLights[i].linear;
        float quadratic = pointLights[i].quadratic;
        float lightMax = std::fmaxf(std::fmaxf(pointLights[i].diffuse.r, pointLights[i].diffuse.g), pointLights[i].diffuse.b);
        float radius = (-linear + std::sqrtf(linear * linear - 4.0 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
            / (2 * quadratic);
        deferredMultipleLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].radius", radius);
        deferredMultipleLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].constant", pointLights[i].constant);
        deferredMultipleLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].linear", pointLights[i].linear);
        deferredMultipleLightingPassShader.setUniformFloat("lights[" + std::to_string(i) + "].quadratic", pointLights[i].quadratic);
        deferredMultipleLightingPassShader.setUniformVec3("lightPos[" + std::to_string(i) + "]", pointLights[i].position);
        deferredMultipleLightingPassShader.setUniformVec3("lights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
        deferredMultipleLightingPassShader.setUniformVec3("lights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
        deferredMultipleLightingPassShader.setUniformVec3("lights[" + std::to_string(i) + "].specular", pointLights[i].specular);
    }

    //send remaining uniforms
    deferredMultipleLightingPassShader.setUniformVec3("cameraPos", newCamera.getEye());
    deferredMultipleLightingPassShader.setUniformFloat("shininess", 64.0f);
    deferredMultipleLightingPassShader.setUniformInt("gamma", GAMMA_ENABLED);
    deferredMultipleLightingPassShader.setUniformInt("numLights", pointLights.size());
    deferredMultipleLightingPassShader.setUniformInt("gPosition", 0);
    deferredMultipleLightingPassShader.setUniformInt("gNormal", 1); //use the floor texture as a specular map
    deferredMultipleLightingPassShader.setUniformInt("gAlbedoSpec", 2);
    //draw screen quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glBindVertexArray(screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        //Draw point lights
        //---------------------------------------------------------------------------------------------------------------
        lightSourceShader.activateShader();
        lightSourceShader.setUniformVec3("lightColor", pointLights[i].diffuse);
        drawCube(lightObjectVAO, lightSourceShader, pointLights[i].position, glm::vec3(0.2, 0.2, 0.2));
    }
    //glEnable(GL_DEPTH_TEST);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------
}

void renderSceneWithBloomEffect(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices) {
    static bool initialized = false;
    static glm::mat4 identityMatrix = glm::mat4(1.0);
    //------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------INITIALIZATION-------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------

    //initialize light parameters
    //--------------------------------------------------------------------------------------------------------
    static std::vector<PointLight> pointLights = {
            PointLight(glm::vec3(0.0f,  0.5f, 1.5f), glm::vec3(0.0f), glm::vec3(5.0f, 5.0f, 5.0f)),
            PointLight(glm::vec3(-4.0f, 0.5f, -3.0f), glm::vec3(0.0f), glm::vec3(10.0f, 0.0f, 0.0f)),
            PointLight(glm::vec3(3.0f, 0.5f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 15.0f)),
            PointLight(glm::vec3(-0.8f, 2.4f, -1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 5.0f, 0.0f)),
    };
    //--------------------------------------------------------------------------------------------------------

    //initialize shaders
    //--------------------------------------------------------------------------------------------------------
    static Shader multipleLightsMRTShader = Shader("Shaders/multipleLightsMRT.vert", "Shaders/multipleLightsMRT.frag");
    static Shader lightSourceMRTShader = Shader("Shaders/lightSourceMRT.vert", "Shaders/lightSourceMRT.frag");
    static Shader blurShader = Shader("Shaders/blur.vert", "Shaders/blur.frag");
    static Shader bloomShader = Shader("Shaders/bloom.vert", "Shaders/bloom.frag");
    //--------------------------------------------------------------------------------------------------------

    //load textures
    //--------------------------------------------------------------------------------------------------------
    static unsigned int cubeTexture = textureFromFile("../../Textures/container2.png", false);
    static unsigned int cubeTextureGammaCorrected = textureFromFile("../../Textures/container2.png", true);
    static unsigned int floorTexture = textureFromFile("../../Textures/wood.png", false);
    static unsigned int floorTextureGammaCorrected = textureFromFile("../../Textures/wood.png", true);
    static unsigned int cubeTexture_normal = textureFromFile("../../Textures/toy_box_normal.png", false);
    static unsigned int cubeTexture_depth = textureFromFile("../../Textures/toy_box_disp.png", false);
    //--------------------------------------------------------------------------------------------------------

    static unsigned int hdrFBO, hdr_colorBuffers[2];
    static unsigned int pingpongFBO[2], pingpongBuffers[2];

    if (!initialized) {
        //set up floating point framebuffer to render scene to
        createFBO(hdrFBO, hdr_colorBuffers, 2, GL_RGBA16F);
        //---------------------------------------------------------------------------------------------------------

        //creatae two basic framebuffers, each with only a color buffer texture
        createFBO(pingpongFBO[0], pingpongBuffers[0], GL_RGBA16F, false);
        createFBO(pingpongFBO[1], pingpongBuffers[1], GL_RGBA16F, false);
        //--------------------------------------------------------------------------------------------------------

        //bind ubo and shaders to a binding location
        //--------------------------------------------------------------------------------------------------------
        //bind uniform buffer object to binding point(loc) 0
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        //get relevant block indices
        unsigned int lightSourceMRTShader_uniformBlockIndex = glGetUniformBlockIndex(lightSourceMRTShader.getProgramId(), "Matrices");
        unsigned int multipleLightsMRTShader_uniformBlockIndex = glGetUniformBlockIndex(multipleLightsMRTShader.getProgramId(), "Matrices");

        //link each shader's uniform block indices to uniform binding point(loc) 0
        glUniformBlockBinding(lightSourceMRTShader.getProgramId(), lightSourceMRTShader_uniformBlockIndex, 0);
        glUniformBlockBinding(multipleLightsMRTShader.getProgramId(), multipleLightsMRTShader_uniformBlockIndex, 0);
        //--------------------------------------------------------------------------------------------------------

        initialized = true;
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------

    //First pass
    //---------------------------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //inserting projection and view matrices into ubo
    //--------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

    //glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

    glm::mat4 view = newCamera.getViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        //Draw point lights
        //---------------------------------------------------------------------------------------------------------------
        lightSourceMRTShader.activateShader();
        lightSourceMRTShader.setUniformVec3("lightColor", pointLights[i].diffuse);
        drawCube(lightObjectVAO, lightSourceMRTShader, pointLights[i].position, glm::vec3(0.2, 0.2, 0.2));
        //---------------------------------------------------------------------------------------------------------------

        //send pointLight uniform values for drawing tunnel cube
        //---------------------------------------------------------------------------------------------------------------
        multipleLightsMRTShader.activateShader();
        multipleLightsMRTShader.setUniformVec3("lightPos[" + std::to_string(i) + "]", pointLights[i].position);
        multipleLightsMRTShader.setUniformVec3("lights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
        multipleLightsMRTShader.setUniformVec3("lights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
        multipleLightsMRTShader.setUniformVec3("lights[" + std::to_string(i) + "].specular", pointLights[i].specular);
        //---------------------------------------------------------------------------------------------------------------
    }

    //activate shader and pass uniforms to it
    //---------------------------------------------------------------------------------------------------------------
    multipleLightsMRTShader.activateShader();
    multipleLightsMRTShader.setUniformVec3("cameraPos", newCamera.getEye());
    multipleLightsMRTShader.setUniformInt("gamma", GAMMA_ENABLED);
    multipleLightsMRTShader.setUniformInt("normal_mapping", NORMAL_MAPPING);
    multipleLightsMRTShader.setUniformInt("parallax_mapping", PARALLAX_MAPPING);
    multipleLightsMRTShader.setUniformFloat("height_scale", 0.2);
    multipleLightsMRTShader.setUniformInt("numLights", pointLights.size());

    //sets material properties
    multipleLightsMRTShader.setUniformInt("material.diffuseMap", 0);
    multipleLightsMRTShader.setUniformInt("material.specularMap", 0); //use the floor texture as a specular map
    multipleLightsMRTShader.setUniformInt("material.emissionMap", 2);
    multipleLightsMRTShader.setUniformFloat("material.shininess", 64.0f);
    multipleLightsMRTShader.setUniformInt("material.heightMap", 1);
    multipleLightsMRTShader.setUniformInt("depthMap", 3);
    //---------------------------------------------------------------------------------------------------------------

    //draw scene
    //---------------------------------------------------------------------------------------------------------------
    //draw cube as floor
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? floorTextureGammaCorrected : floorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_depth);
    drawCube(cubeVAO, multipleLightsMRTShader, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(12.5f, 0.5f, 12.5f));
    
    //draw other cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? cubeTextureGammaCorrected : cubeTexture);
    drawCube(cubeVAO, multipleLightsMRTShader, glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.5f));
    drawCube(cubeVAO, multipleLightsMRTShader, glm::vec3(2.0f, 0.0f, 1.0f), glm::vec3(0.5f));
    glm::mat4 rotationMatrix = glm::rotate(identityMatrix, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, multipleLightsMRTShader, glm::vec3(-1.0f, -1.0f, 2.0f), glm::vec3(1.0f), rotationMatrix);
    rotationMatrix = glm::rotate(identityMatrix, glm::radians(23.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, multipleLightsMRTShader, glm::vec3(0.0f, 2.7f, 4.0f), glm::vec3(1.25f), rotationMatrix);
    rotationMatrix = glm::rotate(identityMatrix, glm::radians(124.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    drawCube(cubeVAO, multipleLightsMRTShader, glm::vec3(-2.0f, 1.0f, -3.0f), glm::vec3(1.0f), rotationMatrix);
    drawCube(cubeVAO, multipleLightsMRTShader, glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(0.5f));
    //---------------------------------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    //second pass: blur bright fragments with two-pass Gaussian Blur
    //---------------------------------------------------------------------------------------------------------------
    blurShader.activateShader();
    bool horizontal = true, first_iteration = true;
    int amount = 10;
    for (unsigned int i = 0; i < amount; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        blurShader.setUniformInt("horizontal", horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? hdr_colorBuffers[1] : pingpongBuffers[!horizontal]);
        glBindVertexArray(screenQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    //third pass: blend scene's HDR texture and  blurred brightness texture together to achieve bloom effect
    //---------------------------------------------------------------------------------------------------------------
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    bloomShader.activateShader();
    bloomShader.setUniformFloat("exposure", 0.1);
    bloomShader.setUniformInt("bloom", BLOOM_ENABLED);
    bloomShader.setUniformInt("scene", 0);
    bloomShader.setUniformInt("blur", 1);
    glBindVertexArray(screenQuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------
}

void renderRandomScene(unsigned int cubeVAO, unsigned int planeVAO, unsigned int lightObjectVAO, unsigned int uboMatrices) {
    static bool initialized = false;
    static glm::mat4 identityMatrix = glm::mat4(1.0);
    //------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------INITIALIZATION-------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //load model into object
    //static Model modelObject = Model("../../Models/backpack/backpack.obj");
    //--------------------------------------------------------------------------------------------------------

    static PointLight pointLight = PointLight(glm::vec3(0.4f, 2.5f, 0.7f), glm::vec3(0.2f), glm::vec3(0.5f));// glm::vec3(-2.0f + 2, 2.3f - 1 - 1, -1.0f + 5));// glm::vec3(-1.0f, 0.3f, -0.3f));

    //initialize shaders
    static Shader lightSourceShader = Shader("Shaders/lightSourceShader.vert", "Shaders/lightSourceShader.frag");
    static Shader defaultLightingShader = Shader("Shaders/defaultLighting.vert", "Shaders/defaultLighting.frag");
    //--------------------------------------------------------------------------------------------------------

    //load textures
    static unsigned int cubeTexture = textureFromFile("../../Textures/wood.png", false); //textureFromFile("iron_texture.jpg");
    static unsigned int floorTexture = textureFromFile("../../Textures/marble.jpg", false);
    static unsigned int floorTextureGammaCorrected = textureFromFile("../../Textures/marble.jpg", true);
    static unsigned int cubeTextureGammaCorrected = textureFromFile("../../Textures/wood.png", true);
    static unsigned int cubeTexture_normal = textureFromFile("../../Textures/toy_box_normal.png", false);
    static unsigned int cubeTexture_depth = textureFromFile("../../Textures/toy_box_disp.png", false);
    static unsigned int singleCubeTexture = textureFromFile("../../Textures/bricks2.jpg", false);
    static unsigned int singleCubeTextureGammaCorrected = textureFromFile("../../Textures/bricks2.jpg", true);
    static unsigned int singleCubeTexture_normal = textureFromFile("../../Textures/bricks2_normal.jpg", false);
    static unsigned int singleCubeTexture_depth = textureFromFile("../../Textures/bricks2_disp.jpg", false);
    //--------------------------------------------------------------------------------------------------------

    if (!initialized) {
        //bind ubo and shaders to a binding location
        //--------------------------------------------------------------------------------------------------------
        //bind uniform buffer object to binding point(loc) 0
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        //get relevant block indices
        unsigned int lightSourceShader_uniformBlockIndex = glGetUniformBlockIndex(lightSourceShader.getProgramId(), "Matrices");
        unsigned int defaultLightingShader_uniformBlockIndex = glGetUniformBlockIndex(defaultLightingShader.getProgramId(), "Matrices");

        //link each shader's uniform block indices to uniform binding point(loc) 0
        glUniformBlockBinding(lightSourceShader.getProgramId(), lightSourceShader_uniformBlockIndex, 0);
        glUniformBlockBinding(defaultLightingShader.getProgramId(), defaultLightingShader_uniformBlockIndex, 0);
        //--------------------------------------------------------------------------------------------------------

        initialized = true;
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------


    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //inserting projection and view matrices into ubo
    //--------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

    //glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

    glm::mat4 view = newCamera.getViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    //activate shader and pass uniforms to it
    //--------------------------------------------------------------------------------------------------------
    defaultLightingShader.activateShader();
    defaultLightingShader.setUniformVec3("cameraPos", newCamera.getEye());
    defaultLightingShader.setUniformInt("gamma", GAMMA_ENABLED);
    defaultLightingShader.setUniformInt("normal_mapping", NORMAL_MAPPING);
    defaultLightingShader.setUniformInt("parallax_mapping", PARALLAX_MAPPING);
    defaultLightingShader.setUniformFloat("height_scale", 0.2);

    //send point light values to shader
    defaultLightingShader.setUniformVec3("lightPos", pointLight.position);
    defaultLightingShader.setUniformVec3("light.ambient", pointLight.ambient);
    defaultLightingShader.setUniformVec3("light.diffuse", pointLight.diffuse);
    defaultLightingShader.setUniformVec3("light.specular", pointLight.specular);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    //draws floor
    //--------------------------------------------------------------------------------------------------------
    glStencilMask(0x00); //disable writing to the stencil buffer, prevents floor from affecting the rendering of the cube border
    glBindVertexArray(planeVAO);
    glm::mat4 rotationMatrix = glm::rotate(identityMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
    defaultLightingShader.setUniformMatrix4("rotationMatrix", rotationMatrix);
    defaultLightingShader.setUniformMatrix4("model", identityMatrix);

    //sets material properties
    defaultLightingShader.setUniformInt("material.diffuseMap", 0);
    defaultLightingShader.setUniformInt("material.specularMap", 0); //use the floor texture as a specular map
    defaultLightingShader.setUniformInt("material.emissionMap", 2);
    defaultLightingShader.setUniformFloat("material.shininess", 64.0f);
    defaultLightingShader.setUniformInt("material.heightMap", 1);
    defaultLightingShader.setUniformInt("depthMap", 3);
    //--------------------------------------------------------------------------------------------------------

    glActiveTexture(GL_TEXTURE0); //sets texture unit as the current active texture unit
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? floorTextureGammaCorrected : floorTexture);//binds texture objects to the current active texture unit in texture target
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_depth);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    //draw cubes
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? cubeTextureGammaCorrected : cubeTexture); //binds texture objects to the current active texture unit in texture target
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_normal); glActiveTexture(GL_TEXTURE3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_depth);  //glBindTexture(GL_TEXTURE_2D, depthMap);
    drawTwoContainers(cubeVAO, defaultLightingShader, projection, view, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? singleCubeTextureGammaCorrected : singleCubeTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, singleCubeTexture_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, singleCubeTexture_depth);
    drawCube(cubeVAO, defaultLightingShader, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1, 1, 1));
    //--------------------------------------------------------------------------------------------------------


    //modelObject.draw(defaultLightingShader);
    //Draw Point light
    lightSourceShader.activateShader();
    drawCube(lightObjectVAO, lightSourceShader, pointLight.position, glm::vec3(0.2, 0.2, 0.2));
    lightSourceShader.setUniformVec3("lightColor", pointLight.diffuse);
    //--------------------------------------------------------------------------------------------------------
}
void renderTunnelScene(unsigned int cubeVAO, unsigned int lightObjectVAO, unsigned int screenQuadVAO, unsigned int uboMatrices) {
    static bool initialized = false;
    static glm::mat4 identityMatrix = glm::mat4(1.0);
    //------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------INITIALIZATION-------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------

    //initialize light parameters
    //--------------------------------------------------------------------------------------------------------
    static std::vector<PointLight> pointLights = {
            PointLight(glm::vec3(0.0f,  0.0f, 49.5f), glm::vec3(0.0f), glm::vec3(200.0f, 200.0f, 200.0f)),
            PointLight(glm::vec3(-1.4f, -1.9f, -9.0f + 17), glm::vec3(0.0f), glm::vec3(0.1f, 0.0f, 0.0f)),
            PointLight(glm::vec3(0.0f, -1.8f, -4.0f + 17), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 0.2f)),
            PointLight(glm::vec3(0.8f, -1.7f, -6.0f + 17), glm::vec3(0.0f), glm::vec3(0.0f, 0.1f, 0.0f)),
            PointLight(glm::vec3(0.0f,  0.0f, 12.5f), glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
            PointLight(glm::vec3(-0.4f, -0.9f, 9.0f + 15), glm::vec3(0.0f), glm::vec3(0.01f, 0.0f, 0.0f)),
            PointLight(glm::vec3(0.0f, -0.8f, 4.0f + 15), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 0.02f)),
            PointLight(glm::vec3(0.8f, -0.7f, 6.0f + 15), glm::vec3(0.0f), glm::vec3(0.0f, 0.01f, 0.0f)),
    };
    //--------------------------------------------------------------------------------------------------------

    //initialize shaders
    //--------------------------------------------------------------------------------------------------------
    static Shader multipleLightsShader = Shader("Shaders/multipleLights.vert", "Shaders/multipleLights.frag");
    static Shader lightSourceShader = Shader("Shaders/lightSourceShader.vert", "Shaders/lightSourceShader.frag");
    static Shader hdrShader = Shader("Shaders/hdrShader.vert", "Shaders/hdrShader.frag");
    //--------------------------------------------------------------------------------------------------------

    //load textures
    //--------------------------------------------------------------------------------------------------------
    static unsigned int cubeTexture = textureFromFile("../../Textures/wood.png", false);
    static unsigned int cubeTextureGammaCorrected = textureFromFile("../../Textures/wood.png", true);
    static unsigned int cubeTexture_normal = textureFromFile("../../Textures/toy_box_normal.png", false);
    static unsigned int cubeTexture_depth = textureFromFile("../../Textures/toy_box_disp.png", false);
    //--------------------------------------------------------------------------------------------------------

    static unsigned int hdrFBO, hdr_screenTexture;

    if (!initialized) {
        //bind ubo and shaders to a binding location
        //--------------------------------------------------------------------------------------------------------
        //bind uniform buffer object to binding point(loc) 0
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        //get relevant block indices
        unsigned int lightSourceShader_uniformBlockIndex = glGetUniformBlockIndex(lightSourceShader.getProgramId(), "Matrices");
        unsigned int multipleLightsShader_uniformBlockIndex = glGetUniformBlockIndex(multipleLightsShader.getProgramId(), "Matrices");

        //link each shader's uniform block indices to uniform binding point(loc) 0
        glUniformBlockBinding(lightSourceShader.getProgramId(), lightSourceShader_uniformBlockIndex, 0);
        glUniformBlockBinding(multipleLightsShader.getProgramId(), multipleLightsShader_uniformBlockIndex, 0);
        //--------------------------------------------------------------------------------------------------------

        //setup hdr framebuffer
        createFBO(hdrFBO, hdr_screenTexture, GL_RGBA16F);
        //--------------------------------------------------------------------------------------------------------

        initialized = true;
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------

    //First pass
    //---------------------------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //inserting projection and view matrices into ubo
    //--------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

    //glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

    glm::mat4 view = newCamera.getViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        //Draw point lights
        //---------------------------------------------------------------------------------------------------------------
        lightSourceShader.activateShader();
        lightSourceShader.setUniformVec3("lightColor", pointLights[i].diffuse);
        drawCube(lightObjectVAO, lightSourceShader, pointLights[i].position, glm::vec3(0.2, 0.2, 0.2));
        //---------------------------------------------------------------------------------------------------------------

        //send pointLight uniform values for drawing tunnel cube
        //---------------------------------------------------------------------------------------------------------------
        multipleLightsShader.activateShader();
        multipleLightsShader.setUniformVec3("lightPos[" + std::to_string(i) + "]", pointLights[i].position);
        multipleLightsShader.setUniformVec3("lights[" + std::to_string(i) + "].ambient", pointLights[i].ambient);
        multipleLightsShader.setUniformVec3("lights[" + std::to_string(i) + "].diffuse", pointLights[i].diffuse);
        multipleLightsShader.setUniformVec3("lights[" + std::to_string(i) + "].specular", pointLights[i].specular);
        //---------------------------------------------------------------------------------------------------------------
    }

    //activate shader and pass uniforms to it
    //---------------------------------------------------------------------------------------------------------------
    multipleLightsShader.activateShader();
    multipleLightsShader.setUniformVec3("cameraPos", newCamera.getEye());
    multipleLightsShader.setUniformInt("gamma", GAMMA_ENABLED);
    multipleLightsShader.setUniformInt("normal_mapping", NORMAL_MAPPING);
    multipleLightsShader.setUniformInt("parallax_mapping", PARALLAX_MAPPING);
    multipleLightsShader.setUniformInt("inverse_normals", true);
    multipleLightsShader.setUniformFloat("height_scale", 0.2);
    multipleLightsShader.setUniformInt("numLights", pointLights.size());

    //sets material properties
    multipleLightsShader.setUniformInt("material.diffuseMap", 0);
    multipleLightsShader.setUniformInt("material.specularMap", 0); //use the floor texture as a specular map
    multipleLightsShader.setUniformInt("material.emissionMap", 2);
    multipleLightsShader.setUniformFloat("material.shininess", 64.0f);
    multipleLightsShader.setUniformInt("material.heightMap", 1);
    multipleLightsShader.setUniformInt("depthMap", 3);
    //---------------------------------------------------------------------------------------------------------------

    //draw tunnel
    //---------------------------------------------------------------------------------------------------------------
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? cubeTextureGammaCorrected : cubeTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cubeTexture_depth);
    drawCube(cubeVAO, multipleLightsShader, glm::vec3(0.0f, 0.0f, 25.0f), glm::vec3(2.5f, 2.5f, 27.5f));
    //---------------------------------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    //second pass
    //---------------------------------------------------------------------------------------------------------------
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    hdrShader.activateShader();
    hdrShader.setUniformFloat("exposure", 0.1);
    glBindVertexArray(screenQuadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------
}

void drawTwoContainers(GLuint cubeVAO, const Shader& shader, const glm::mat4& projection, const glm::mat4& view, float scale) {
    //passing view and projection matrices to the shader
   // shader.setUniformMatrix4("projection", projection);
    //shader.setUniformMatrix4("view", view);
    //-----------------------------------------------------------------
    //draws cubes
    glBindVertexArray(cubeVAO);

    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, glm::vec3(scale, scale, scale));

    glm::mat4 rotationMatrix(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(30.0f), glm::vec3(1.0f, 0.3f, 0.5f));

    glm::mat4 translationMatrix(1.0f);

    //cube 1
    translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -1.0f));
    glm::mat4 model = translationMatrix * rotationMatrix * scaleMatrix;
    shader.setUniformMatrix4("model", model);
    shader.setUniformMatrix4("rotationMatrix", rotationMatrix);
    shader.setUniformBool("useSkybox", true); //ensures reflected skybox is used as texture
    glDrawArrays(GL_TRIANGLES, 0, 36);

    //cube 2
    translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(30.0f), glm::vec3(1.0f, 0.3f, 0.5f));
    model = translationMatrix * rotationMatrix * scaleMatrix;
    shader.setUniformMatrix4("model", model);
    shader.setUniformMatrix4("rotationMatrix", rotationMatrix);
    shader.setUniformBool("useSkybox", false);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //---------------------------------------------------

    glBindVertexArray(0);
}

void drawCube(GLuint cubeVAO, const Shader& shader, const glm::vec3 translationVec, glm::vec3 scale, glm::mat4 rotationMatrix) {
    glBindVertexArray(cubeVAO);

    glm::mat4 identityMatrix = glm::mat4(1.0);
    glm::mat4 scaleMatrix = glm::scale(identityMatrix, scale);
    glm::mat4 translationMatrix = glm::translate(identityMatrix, translationVec);
    //glm::mat4 rotationMatrix = glm::rotate(identityMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
    glm::mat4 model = translationMatrix * rotationMatrix * scaleMatrix;
    shader.setUniformMatrix4("model", model);
    shader.setUniformMatrix4("rotationMatrix", rotationMatrix);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void createFBO(unsigned int& framebuffer, unsigned int *colorBuffer, unsigned int numColorBuffers, GLint colorBuffer_internalFormat, bool hasDepthBuffer) {
    std::vector<unsigned int> attachments;

    //setup framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //generate texture as color attachement
    glGenTextures(numColorBuffers, colorBuffer);
    for (unsigned int i = 0; i < numColorBuffers; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, colorBuffer_internalFormat, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);//unnbind color buffer from target

        //attach the newly-generated color buffer to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, colorBuffer[i], 0);

        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    //Tell OpenGL we're rendering to multiple colorbuffers
    if (numColorBuffers > 1){
        glDrawBuffers(numColorBuffers, attachments.data());
    }

    if (hasDepthBuffer) {
        //create renderbuffer object and allocate space for it
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);  //unbind rbo
        //--------------------------------------------------------

        //attach the renderbuffer object to the currently-bound object
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }
    //check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    //----------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbined framebuffer
    //---------------------------------------------------------------------------------------------
}
void createFBO(unsigned int& framebuffer, unsigned int* colorBuffer, unsigned int numColorBuffers, GLint* colorBuffer_internalFormat, bool hasDepthBuffer) {
    std::vector<unsigned int> attachments;

    //setup framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //generate texture as color attachement
    glGenTextures(numColorBuffers, colorBuffer);
    for (unsigned int i = 0; i < numColorBuffers; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, colorBuffer_internalFormat[i], FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);//unnbind color buffer from target

        //attach the newly-generated color buffer to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], 0);

        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }

    //Tell OpenGL we're rendering to multiple colorbuffers
    if (numColorBuffers > 1) {
        glDrawBuffers(numColorBuffers, attachments.data());
    }

    if (hasDepthBuffer) {
        //create renderbuffer object and allocate space for it
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);  //unbind rbo
        //--------------------------------------------------------

        //attach the renderbuffer object to the currently-bound object
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }
    //check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    //----------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbined framebuffer
    //---------------------------------------------------------------------------------------------
}

void createFBO(unsigned int& framebuffer, unsigned int& texColorBuffer, GLint colorBuffer_internalFormat, bool hasDepthbuffer) {
    createFBO(framebuffer, &texColorBuffer, 1, colorBuffer_internalFormat, hasDepthbuffer);
}

void createDepthMapFBO(unsigned int& depthMapFBO, unsigned int& depthMap) {
    //setup framebuffer
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

    //generate texture as color attachement
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

    glBindTexture(GL_TEXTURE_2D, 0);//unbind color buffer from target
    //---------------------------------------

    //attach the newly-generated color buffer to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    //To explicitly tel OpenGL, we are not rendering any color data and it is complete without color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    //------------------------------------------------------------------------------------------------

    //check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    //----------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbined framebuffer
    //---------------------------------------------------------------------------------------------
}

void createDepthCubeMapFBO(unsigned int& depthCubeMapFBO, unsigned int& depthCubeMap) {
    //setup framebuffer
    glGenFramebuffers(1, &depthCubeMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);

    //generate cubemap faces textures as color attachment
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);//unbind cubemap from target
    //--------------------------------------------------------------

    //attach the newly-generated cubemap color buffer to currently bound framebuffer object
    /** Note: Not using glFramebufferTexture2D because it requires a textarget to be a face of the cubemap (such as,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X) but we are trying to attahch it all at once **/
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);

    //To explicitly tell OpenGL, we are not rendering any color data and it is complete without color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    //------------------------------------------------------------------------------------------------

    //check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    //----------------------------------------------------------------------------------------

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbined framebuffer
    //---------------------------------------------------------------------------------------------
}

void createMSAA_FBO(unsigned int& multisampledFBO, unsigned int& msaa_texColorBuffer, unsigned int numSamples) {
    //setup MSAA framebuffer
    glGenFramebuffers(1, &multisampledFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);

    //generate multisampled texture as color attachement
    glGenTextures(1, &msaa_texColorBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa_texColorBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGB, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, GL_TRUE);
    //NOT NEED TO SET TEX PARAMETERS FOR MAGNIFYING AND MINIMIZING TEXTURE BECAUSE OPENGL CHOOSES GL_NEAREST NO MATTER WHAT
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);//unnbind color buffer from target

    //attach the newly-generated color buffer to currently bound MSAA framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa_texColorBuffer, 0);//create renderbuffer object and allocate space for it

    //create multisampled renderbuffer object and allocate space for it
    unsigned int msaa_rbo;
    glGenRenderbuffers(1, &msaa_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, msaa_rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_DEPTH24_STENCIL8, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);  //unbind rbo

    //attach the renderbuffer object to the currently-bound object
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaa_rbo);

    //check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbined framebuffer
    //-----------------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------------
}

void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam) {

    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
    // Suppress some useless warnings
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        exit(-1);
    default:
        return;
    }
}

/*  This function calculates the tangent and bitangent of each vertex in the provided object data and inserts them in 
    the provided vector container
*   Parameters:
*       tangentsAndBitangents:  Vector container that will be filled with the tangents and bitangents of the vertices in an 
                                orderly fashion. Each vertex takes up 6 elements in the vector
*
*       objData:                A pointer to the object's data of type float
*       objDataSize:            The size of the object's data
*       vertexStride:           Stride to the next vertex
*       uvOffset:               Offset from the beginning of a vertex to the uv coordinates of a vertex
*   NOTE:
*           This function expects an object triangle data, that is the data is filled with triangles
* */
void setObjectTangentsandBitangents(std::vector<glm::vec3>& tangentsAndBitangents, const float *objData, 
    const unsigned int objDataSize, const unsigned int vertexStride, const unsigned int uvOffset) {
    unsigned int triangleStride = vertexStride * 3; //stride to next triangle
    for (unsigned int i = 0; i < objDataSize; i += triangleStride) {
        glm::vec3 tangent, bitangent;

        glm::vec3 vertex1 = glm::vec3(objData[i], objData[i + 1], objData[i + 2]);
        glm::vec2 uv1 = glm::vec2(objData[i + uvOffset], objData[i + uvOffset + 1]);

        glm::vec3 vertex2 = glm::vec3(objData[i + vertexStride], objData[i + vertexStride + 1],
            objData[i + vertexStride + 2]);
        glm::vec2 uv2 = glm::vec2(objData[i + vertexStride + uvOffset], objData[i + vertexStride + uvOffset + 1]);

        glm::vec3 vertex3 = glm::vec3(objData[i + (2 * vertexStride)], objData[i + (2 * vertexStride) + 1],
            objData[i + (2 * vertexStride) + 2]);
        glm::vec2 uv3 = glm::vec2(objData[i + (2 * vertexStride) + uvOffset],
            objData[i + (2 * vertexStride) + uvOffset + 1]);


        glm::vec3 edge1 = vertex2 - vertex1;
        glm::vec3 edge2 = vertex3 - vertex1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);


        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        //vertex 1 tangent and bitangent
        tangentsAndBitangents.push_back(tangent);
        tangentsAndBitangents.push_back(bitangent);

        //vertex 2 tangent and bitangent
        tangentsAndBitangents.push_back(tangent);
        tangentsAndBitangents.push_back(bitangent);

        //vertex 3 tangent and bitangent
        tangentsAndBitangents.push_back(tangent);
        tangentsAndBitangents.push_back(bitangent);
    }
}

void PBR_directLighting(unsigned int uboMatrices) {
    static bool initialized = false;
    //------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------INITIALIZATION-------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    static glm::mat4 identityMatrix = glm::mat4(1.0);
    static int nRows = 7;
    static int nCols = 7;
    static float spacing = 2.5;

    //initialize light parameters
    //--------------------------------------------------------------------------------------------------------
    /*static std::vector<PointLight> pointLights = {
            PointLight(glm::vec3(-10.0f,  10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f)),
            PointLight(glm::vec3(10.0f,  10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f)),
            PointLight(glm::vec3(-10.0f,  -10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f)),
            PointLight(glm::vec3(10.0f,  -10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f))
    };*/ 
    static std::vector<PointLight> pointLights = {
            PointLight(glm::vec3(0.0f,  0.0f, 10.0f), glm::vec3(0.0f), glm::vec3(150.0f, 150.0f, 150.0f))
    };
    //--------------------------------------------------------------------------------------------------------

    //initialize shaders
    //--------------------------------------------------------------------------------------------------------
    static Shader shader = Shader("Shaders/PBR_directLighting.vert", "Shaders/PBR_directLighting.frag");
    //--------------------------------------------------------------------------------------------------------

    //load textures
    //--------------------------------------------------------------------------------------------------------
    static unsigned int albedoMap = textureFromFile("../../Textures/rustediron/rustediron_basecolor.png", true); //convert to lineasr space
    static unsigned int normalMap = textureFromFile("../../Textures/rustediron/rustediron_normal.png", false);
    static unsigned int metallicMap = textureFromFile("../../Textures/rustediron/rustediron_metallic.png", false);
    static unsigned int roughnessMap = textureFromFile("../../Textures/rustediron/rustediron_roughness.png", false);
    static unsigned int aoMap = textureFromFile("../../Textures/rustediron/my_ao.png", false);
    //--------------------------------------------------------------------------------------------------------

    if (!initialized) {
        //bind ubo and shaders to a binding location
        //--------------------------------------------------------------------------------------------------------
        //bind uniform buffer object to binding point(loc) 0
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        //get relevant block indices
        unsigned int shader_uniformBlockIndex = glGetUniformBlockIndex(shader.getProgramId(), "Matrices");

        //link each shader's uniform block indices to uniform binding point(loc) 0
        glUniformBlockBinding(shader.getProgramId(), shader_uniformBlockIndex, 0);
        //--------------------------------------------------------------------------------------------------------

        //bind texture maps to their respective texture unit and uniform sampler
        shader.activateShader();
        shader.setUniformVec3("albedo", 0.5f, 0.0f, 0.0f);
        shader.setUniformFloat("ao", 1.0f);

        shader.setUniformInt("albedoMap", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoMap);

        shader.setUniformInt("normalMap", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);

        shader.setUniformInt("metallicMap", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicMap);

        shader.setUniformInt("roughnessMap", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessMap);

        shader.setUniformInt("aoMap", 4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, aoMap);
        //--------------------------------------------------------------------------------------------------------

        initialized = true;
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //inserting projection and view matrices into ubo
    //--------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

    glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

    glm::mat4 view = newCamera.getViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //--------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------

    shader.activateShader();
    shader.setUniformVec3("cameraPos", newCamera.getEye());
    glm::mat4 model = identityMatrix;

    //draw point lights. It looks a bit off as we use the same shader, but it'll make their positions obvious
    //and keeps the codeprint small.
    //---------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        //pointLights[i].position = pointLights[i].position + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        shader.setUniformVec3("lightPositions[" + std::to_string(i) + "]", pointLights[i].position);
        shader.setUniformVec3("lightColors[" + std::to_string(i) + "]", pointLights[i].diffuse);

        model = identityMatrix;
        model = glm::translate(model, pointLights[i].position);
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setUniformMatrix4("model", model);
        drawSphere(64, 64);
    }

    //Render row*column number of spheres with varying metallic/roughneass values scaled by rows and columns respectively
    //As the row number increases, the surface of the sphere becomes more metallic
    //As the col number increases, the surface of the sphere becomes rougher
    //---------------------------------------------------------------------------------------------------------------
    for (int row = 0; row < nRows; ++row) {
        shader.setUniformFloat("metallic", (float)row / nRows);
        for (int col = 0; col < nCols; ++col) {
            //the sphere's surface is clamped to 0.05 -1.0, as perfectly smooth surfaces (roughness of 0.0) tend to
            //look a bit off on direct lighting
            shader.setUniformFloat("roughness", glm::clamp((float)col / nCols, 0.05f, 1.0f));

            model = identityMatrix;
            model = glm::translate(model, glm::vec3(
                (col - (nCols / 2)) * spacing,
                (row - (nRows / 2)) * spacing,
                0.0f
            ));
            shader.setUniformMatrix4("model", model);
            drawSphere(64, 64);
        }
    }
}

void renderEquirectangularMap_withPBR(unsigned int cubeVAO, unsigned int uboMatrices) {
    static bool initialized = false;
    //------------------------------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------INITIALIZATION-------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    static glm::mat4 identityMatrix = glm::mat4(1.0);
    static int nRows = 7;
    static int nCols = 7;
    static float spacing = 2.5;

    //initialize light parameters
    //--------------------------------------------------------------------------------------------------------
    static std::vector<PointLight> pointLights = {
            PointLight(glm::vec3(-10.0f,  10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f)),
            PointLight(glm::vec3(10.0f,  10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f)),
            PointLight(glm::vec3(-10.0f,  -10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f)),
            PointLight(glm::vec3(10.0f,  -10.0f, 10.0f), glm::vec3(0.0f), glm::vec3(300.0f, 300.0f, 300.0f))
    };
    //--------------------------------------------------------------------------------------------------------

    //initialize shaders
    //--------------------------------------------------------------------------------------------------------
    static Shader shader = Shader("Shaders/PBR_indirectLighting.vert", "Shaders/PBR_indirectLighting.frag");
    static Shader equirectangularToCubemapShader = Shader("Shaders/equirectangularToCubemap.vert", "Shaders/equirectangularToCubemap.frag");
    static Shader SkyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
    static Shader irradianceShader = Shader("Shaders/equirectangularToCubemap.vert", "Shaders/irradiance.frag");
    //--------------------------------------------------------------------------------------------------------

    //load textures
    //--------------------------------------------------------------------------------------------------------
    static unsigned int albedoMap = textureFromFile("../../Textures/rustediron/rustediron_basecolor.png", true); //convert to lineasr space
    static unsigned int normalMap = textureFromFile("../../Textures/rustediron/rustediron_normal.png", false);
    static unsigned int metallicMap = textureFromFile("../../Textures/rustediron/rustediron_metallic.png", false);
    static unsigned int roughnessMap = textureFromFile("../../Textures/rustediron/rustediron_roughness.png", false);
    static unsigned int aoMap = textureFromFile("../../Textures/rustediron/my_ao.png", false);
    static unsigned int hdrTexture = textureFromFile_f("../../Textures/newport_loft.hdr");
    //--------------------------------------------------------------------------------------------------------

    static unsigned int captureFBO, envCubemap, irradianceMap;
    if (!initialized) {
        //--------------------------------------------------------------------------------------------------------
        //setup environment cubemap texture
        //--------------------------------------------------------------------------------------------------------
        glGenTextures(1, &envCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        for (unsigned int i = 0; i < 6; ++i) {
            //note that we store each face with 16 bit floating point values
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //--------------------------------------------------------------------------------------------------------
        //setup capture fbo object
        //--------------------------------------------------------------------------------------------------------
        unsigned int captureRBO;
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
        //--------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------
        //setup capture projection and capture views
        //--------------------------------------------------------------------------------------------------------
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
        //--------------------------------------------------------------------------------------------------------

        //convert HDR Equirectangular environment map to cubemap equivalent
        //-------------------------------------------------------------------------------------------------------
        equirectangularToCubemapShader.activateShader();
        equirectangularToCubemapShader.setUniformInt("equirectangularMap", 0);
        equirectangularToCubemapShader.setUniformMatrix4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        glViewport(0, 0, 512, 512); //configured to capture dimensions
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i) {
            equirectangularToCubemapShader.setUniformMatrix4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawCube(cubeVAO, equirectangularToCubemapShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //-------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------
        //setup irradiance cubemap texture with a low resolution of 32x32, which will be attached to the capture FBO
        //--------------------------------------------------------------------------------------------------------
        glGenTextures(1, &irradianceMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        for (unsigned int i = 0; i < 6; ++i) {
            //note that we store each face with 16 bit floating point values
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //--------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------
        //re-scaling the capture framebuffer to the new resolution
        //--------------------------------------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
        //--------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------
        //generate the irradiance map as a cubemap by convoluting the environment's lighting
        //--------------------------------------------------------------------------------------------------------
        irradianceShader.activateShader();
        irradianceShader.setUniformInt("environmentMap", 0);
        irradianceShader.setUniformMatrix4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        glViewport(0, 0, 32, 32); //since we are using a capture dimension of 32x32
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i) {
            irradianceShader.setUniformMatrix4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawCube(cubeVAO, irradianceShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //--------------------------------------------------------------------------------------------------------

        //--------------------------------------------------------------------------------------------------------
        //bind ubo and shaders to a binding location
        //--------------------------------------------------------------------------------------------------------
        //bind uniform buffer object to binding point(loc) 0
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        //get relevant block indices
        unsigned int shader_uniformBlockIndex = glGetUniformBlockIndex(shader.getProgramId(), "Matrices");
        unsigned int SkyboxShader_uniformBlockIndex = glGetUniformBlockIndex(SkyboxShader.getProgramId(), "Matrices");

        //link each shader's uniform block indices to uniform binding point(loc) 0
        glUniformBlockBinding(shader.getProgramId(), shader_uniformBlockIndex, 0);
        glUniformBlockBinding(SkyboxShader.getProgramId(), SkyboxShader_uniformBlockIndex, 0);
        //--------------------------------------------------------------------------------------------------------

        //bind texture maps to their respective texture unit and uniform sampler
        SkyboxShader.activateShader();
        SkyboxShader.setUniformBool("isHDR", 1);


        shader.activateShader();
        shader.setUniformVec3("albedo", 0.5f, 0.0f, 0.0f);
        shader.setUniformFloat("ao", 1.0f);

        shader.setUniformInt("albedoMap", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoMap);

        shader.setUniformInt("normalMap", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);

        shader.setUniformInt("metallicMap", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicMap);

        shader.setUniformInt("roughnessMap", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessMap);

        shader.setUniformInt("aoMap", 4);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, aoMap);
        //--------------------------------------------------------------------------------------------------------

        initialized = true;
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------------------------------------------------------



    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //inserting projection and view matrices into ubo
    //---------------------------------------------------------------------------------------------------------------
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

    glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

    glm::mat4 view = newCamera.getViewMatrix();
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //---------------------------------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------

    shader.activateShader();
    shader.setUniformVec3("cameraPos", newCamera.getEye());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glm::mat4 model = identityMatrix;

    //draw point lights. It looks a bit off as we use the same shader, but it'll make their positions obvious
    //and keeps the codeprint small.
    //---------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < pointLights.size(); ++i) {
        //pointLights[i].position = pointLights[i].position + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
        shader.setUniformVec3("lightPositions[" + std::to_string(i) + "]", pointLights[i].position);
        shader.setUniformVec3("lightColors[" + std::to_string(i) + "]", pointLights[i].diffuse);

        model = identityMatrix;
        model = glm::translate(model, pointLights[i].position);
        model = glm::scale(model, glm::vec3(0.5f));
        shader.setUniformMatrix4("model", model);
        drawSphere(64, 64); 
    }

    //Render row*column number of spheres with varying metallic/roughneass values scaled by rows and columns respectively
    //As the row number increases, the surface of the sphere becomes more metallic
    //As the col number increases, the surface of the sphere becomes rougher
    //---------------------------------------------------------------------------------------------------------------
    for (int row = 0; row < nRows; ++row) {
        shader.setUniformFloat("metallic", (float)row / nRows);
        for (int col = 0; col < nCols; ++col) {
            //the sphere's surface is clamped to 0.05 -1.0, as perfectly smooth surfaces (roughness of 0.0) tend to
            //look a bit off on direct lighting
            shader.setUniformFloat("roughness", glm::clamp((float)col / nCols, 0.05f, 1.0f));

            model = identityMatrix;
            model = glm::translate(model, glm::vec3(
                (col - (nCols / 2)) * spacing,
                (row - (nRows / 2)) * spacing,
                0.0f
            ));
            shader.setUniformMatrix4("model", model);
            drawSphere(64, 64);
        }
    }

    //draw skybox
    //---------------------------------------------------------------------------------------------------------------
    SkyboxShader.activateShader();
    glDepthFunc(GL_LEQUAL);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    drawCube(cubeVAO, SkyboxShader);
    glDepthFunc(GL_LESS);
    //---------------------------------------------------------------------------------------------------------------
}

/*  This function statistically approximate the relative surface area of microfacetas exactly aligned to the 
*   halfway vector h.
*
*   Parameters:
*       n:      Surface's normal
*       h:      The halfway vector to measure againt the surface's microfacets   
*       alpha:  Measure of the surface's roughness
* */
float distributionGGX(glm::vec3 n, glm::vec3 h, float alpha) {
    float alphaSquared = alpha * alpha;
    float n_dot_h = glm::max(glm::dot(n, n), 0.0f);
    float n_dot_hSquared = n_dot_h * n_dot_h;

    float denom = n_dot_hSquared * (alphaSquared - 1.0f) + 1.0f;
    denom = std::_Pi * denom* denom;

    return alphaSquared / denom;
}

/*  This function statistically approximate the relative surface area where its micro surface details overshadow
*   each other, causing light rays to be occluded. Rougher surfaces have a higher probability of overshadowing 
*   microfacets.
*
*   Parameters:
*       n_dot_v:    The dot product between the surface normal and the view direction
*       k:          Measure of the surface's roughness
* */
float geometrySchlickGGX(float n_dot_v, float k) {
    float num = n_dot_v;
    float denom = n_dot_v * (1.0f - k) + k;

    return num / denom;
}

/*  This function statistically approximate the relative surface area where its micro surface details overshadow
*   each other, causing light rays to be occluded. Rougher surfaces have a higher probability of overshadowing 
*   microfacets. It takes into account of both the view direction (geometry obstruction) and the light direction 
*   vector(geometry shadowing).
*
*   Parameters:
*       n:  Surface's normal
*       v:  view direction
*       l:  light direction
*       k:  Measure of the surface's roughness
* */
float geometrySmith(glm::vec3 n, glm::vec3 v, glm::vec3 l, float k) {
    float n_dot_v = glm::max(dot(n, l), 0.0f);
    float n_dot_l = glm::max(dot(n, l), 0.0f);
    float ggx1 = geometrySchlickGGX(n_dot_v, k); //calculates geometry masking for the view direction
    float ggx2 = geometrySchlickGGX(n_dot_l, k); //calculats geometry shadowing for the light direction

    return ggx1 * ggx2;
}

/*  This function returns the fraction of light reflected from a surface 
*
*   Parameters:
*       cosTheta:  the dot product between the halfway vector and the surface's normal
*       F0: base reflectivity; the characteristic speculaar reflectancae of the material; the surface's response
*           when looking straight at the surface.
* */
glm::vec3 fresnelSchlick(float cosTheta, glm::vec3 F0) {
    return F0 + (glm::vec3(1.0f) - F0) * pow(1.0f - cosTheta, 5.0f);
}