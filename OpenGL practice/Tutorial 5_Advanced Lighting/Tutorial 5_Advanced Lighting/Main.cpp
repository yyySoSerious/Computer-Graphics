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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void processInput(GLFWwindow* window, Shader& shader, Camera& camera);
void drawTwoContainers(GLuint cubeVAO, const Shader& shader, const glm::mat4& projection, const glm::mat4& view, float scale);
void drawCube(GLuint cubeVAO, const Shader& shader, const glm::vec3 translationVec, glm::vec3 scale);
void createFBO(unsigned int& framebuffer, unsigned int& texColorBuffer);
void createDepthMapFBO(unsigned int& depthMapFBO, unsigned int& depthMap);
void createDepthCubeMapFBO(unsigned int& depthCubeMapFBO, unsigned int& depthCubeMap);
void createMSAA_FBO(unsigned int& multisampledFBO, unsigned int& msaa_texColorBuffer, unsigned int numSamples);
void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    //glEnable(GL_MULTISAMPLE); //enable multisampling anti-aliasing(MSAA)
    //--------------------------------------------------------------------
    //---------------------------------------------------------------------
    //load shaders
    Shader shader = Shader("Shaders/allPurposeShader.vert", "Shaders/basicShader.frag");
    Shader basicShader = Shader("Shaders/basicShader.vert", "Shaders/basicShader.frag", "Shaders/basicShader.geom");
    Shader screenShader = Shader("Shaders/screenShader.vert", "Shaders/screenShader.frag");
    Shader lightingShader = Shader("Shaders/lightingShader.vert", "Shaders/lightingShader.frag");
    Shader lightSourceShader = Shader("Shaders/lightSourceShader.vert", "Shaders/lightSourceShader.frag");
    Shader basicDepthShader = Shader("Shaders/basicDepthShader.vert", "Shaders/basicDepthShader.frag");
    Shader depthCubeMapShader = Shader("Shaders/depthCubeMapShader.vert", "Shaders/depthCubeMapShader.frag", "Shaders/depthCubeMapShader.geom");
    //----------------------------------------------------------------------------------------------------------

    glm::mat4 identityMatrix = glm::mat4(1.0f);

    float cubeVertices[] = {
        //position            texCoord      normal
        // back face CW
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f, // bottom-right
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f, // bottom-left    
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f, // top-left              
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f, // top-left
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f, // bottom-right                
        // front face CW
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // bottom-left
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f, // top-right  
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f, // bottom-right      
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // bottom-left
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f, // top-left        
        // left face CW
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f,  0.0f,  0.0f, // top-right
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f,  0.0f,  0.0f, // bottom-left
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   -1.0f,  0.0f,  0.0f, // top-left       
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   -1.0f,  0.0f,  0.0f, // bottom-left
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   -1.0f,  0.0f,  0.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   -1.0f,  0.0f,  0.0f, // bottom-right
        // right face CW
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f,  0.0f,  0.0f, // top-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   1.0f,  0.0f,  0.0f, // top-right      
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f, // bottom-right          
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f, // bottom-right
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   1.0f,  0.0f,  0.0f, // bottom-left
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   1.0f,  0.0f,  0.0f, // top-left
        // bottom face  CW      
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f, // top-right
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f,  0.0f, // bottom-left
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,   0.0f, -1.0f,  0.0f, // top-left        
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f,  0.0f, // bottom-left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f, // top-right
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f,  0.0f, // bottom-right
        // top face CW
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f, // top-left
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f,  1.0f,  0.0f, // top-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  1.0f,  0.0f, // bottom-right                 
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,   0.0f,  1.0f,  0.0f, // bottom-right
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,   0.0f,  1.0f,  0.0f, // bottom-left  
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f  // top-left              
    };

    float planeVertices[] = {
        //positions             texCoords     normal
         5.0f, -0.5f,  5.0f,    2.0f, 0.0f,   0.0f, 1.0f, 0.0f, //bottom-right
        -5.0f, -0.5f,  5.0f,    0.0f, 0.0f,   0.0f, 1.0f, 0.0f, //bottom-left
        -5.0f, -0.5f, -5.0f,    0.0f, 2.0f,   0.0f, 1.0f, 0.0f, //top-left

         5.0f, -0.5f,  5.0f,    2.0f, 0.0f,   0.0f, 1.0f, 0.0f, //bottom-right
        -5.0f, -0.5f, -5.0f,    0.0f, 2.0f,   0.0f, 1.0f, 0.0f, //top-left
         5.0f, -0.5f, -5.0f,    2.0f, 2.0f,   0.0f, 1.0f, 0.0f //top-right
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

    //Light-related data
    glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
    };

    glm::vec3 lightColours[] = {
    glm::vec3(0.7f,  0.2f,  0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f,  0.0f, 1.0f),
    glm::vec3(0.2f,  0.5f, 0.8f)
    };

    DirLight dirLight = DirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f, 0.05f, 0.05f));
    PointLight pointLight = PointLight(glm::vec3(-2.0f+2, 2.3f-1, -1.0f));// glm::vec3(-1.0f, 0.3f, -0.3f));
    //-----------------------------------------------------------------------------------------

    //setup MSAA_framebuffer
    unsigned int multisampledFBO, msaa_texColorBuffer;
    createMSAA_FBO(multisampledFBO, msaa_texColorBuffer, NUM_SAMPLES);
    //------------------------------------------------------------------------------------------------------------------

    //setup Intermediate framebuffer
    unsigned int intermediateFBO, screenTexture;
    createFBO(intermediateFBO, screenTexture);
    //-------------------------------------------------------------------------------------------------------------

    //setup depth map framebuffer
    unsigned int depthMapFBO, depthMap;
    createDepthMapFBO(depthMapFBO, depthMap);
    //------------------------------------------------------------------------------------------------------------


    //setup depth cubemap framebuffer
    unsigned int depthCubeMapFBO, depthCubeMap;
    createDepthCubeMapFBO(depthCubeMapFBO, depthCubeMap);
    //------------------------------------------------------------------------------------------------------------

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

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

    //-----------------------------------------------------------------------------------------------
    //setting up uniform block object 
    //-------------------------------------------------------------------------------------------
    //get relevant block indices
    unsigned int Shader_uniformBlockIndex = glGetUniformBlockIndex(shader.getProgramId(), "Matrices");
    unsigned int lightSourceShader_uniformBlockIndex = glGetUniformBlockIndex(lightSourceShader.getProgramId(), "Matrices");
    unsigned int lightingShader_uniformBlockIndex = glGetUniformBlockIndex(lightingShader.getProgramId(), "Matrices");

    //link each shader's uniform block indices to uniform binding point 0
    glUniformBlockBinding(shader.getProgramId(), Shader_uniformBlockIndex, 0); //bind uniform block to binding loc
    glUniformBlockBinding(lightSourceShader.getProgramId(), lightSourceShader_uniformBlockIndex, 0);
    glUniformBlockBinding(lightingShader.getProgramId(), lightingShader_uniformBlockIndex, 0);

    //create ubo buffer
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW); ///alllocates uniform block data
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4)); //bind uniform buffer object to binding point(loc) 0
    //----------------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------------

    //load textures
    unsigned int cubeTexture = textureFromFile("../../Textures/container.jpg", false); //textureFromFile("iron_texture.jpg");
    unsigned int floorTexture = textureFromFile("../../Textures/marble.jpg", false); 
    unsigned int floorTextureGammaCorrected = textureFromFile("../../Textures/marble.jpg", true);
    unsigned int cubeTextureGammaCorrected = textureFromFile("../../Textures/container.jpg", true);


    //--------------------------------------------------------------------------------------------------------

    //main render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f; //The time of last frame
        float currentFrame = glfwGetTime(); //current time
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, shader, newCamera);

        //---------------------------------------------------------
        //render to depth map (directional shadow mapping)
        //---------------------------------------------------------
       // glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); 
       // glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
       // glClear(GL_DEPTH_BUFFER_BIT);

       // basicDepthShader.activateShader();
       // float near_plane = 0.1f, far_plane = 7.5f;
       // glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT,
       //     near_plane, far_plane);
       //// glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
       // glm::mat4 lightView = glm::lookAt(pointLight.position,
       //     glm::vec3(0.0f, 0.0f, 0.0f),
       //     glm::vec3(0.0f, 1.0f, 0.0f));

       // glm::mat4 lightSpaceMatrix = lightProjection * lightView;
       // basicDepthShader.setUniformMatrix4("lightSpaceMatrix", lightSpaceMatrix);

       // //render scene 
       // //draw floor
       // //glCullFace(GL_FRONT); //solves peter-panning issues LIESSSSSSSSSSSSSSSSSSSSSSSS!!!!!!
       // glBindVertexArray(planeVAO);
       // basicDepthShader.setUniformMatrix4("model", identityMatrix);
       // glDrawArrays(GL_TRIANGLES, 0, 6);
       // //draw containers
       // drawTwoContainers(cubeVAO, basicDepthShader, identityMatrix, identityMatrix, 1);
       // drawCube(cubeVAO, basicDepthShader, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1, 1, 1));

       // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //glCullFace(GL_BACK); //return back to default
        //-------------------------------------------------------------------
        //------------------------------------------------------------------------

        //-------------------------------------------------------------------
        //render to depth map(omnidirectional shadow mapping)
        //--------------------------------------------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        depthCubeMapShader.activateShader();
        float near_plane = 0.1f, far_plane = 7.5f, aspect = (float) SHADOW_WIDTH / SHADOW_HEIGHT;
        glm::mat4 shadowProjection = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);

        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProjection *
            glm::lookAt(pointLight.position, pointLight.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProjection *
            glm::lookAt(pointLight.position, pointLight.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProjection *
            glm::lookAt(pointLight.position, pointLight.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(shadowProjection *
            glm::lookAt(pointLight.position, pointLight.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.push_back(shadowProjection *
            glm::lookAt(pointLight.position, pointLight.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProjection *
            glm::lookAt(pointLight.position, pointLight.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

        glm::mat4 lightView = glm::lookAt(pointLight.position,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));

        depthCubeMapShader.setUniformVec3("lightPos", pointLight.position);
        depthCubeMapShader.setUniformFloat("far_plane", far_plane);
        depthCubeMapShader.setUniformArrayOfMatrix4("shadowMatrices", shadowTransforms);

        //render scene 
        //draw floor
        glBindVertexArray(planeVAO);
        depthCubeMapShader.setUniformMatrix4("model", identityMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //draw containers
        drawTwoContainers(cubeVAO, depthCubeMapShader, identityMatrix, identityMatrix, 1);
        drawCube(cubeVAO, depthCubeMapShader, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1, 1, 1));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //-------------------------------------------------------------------
        //------------------------------------------------------------------------

        //-----------------------------------------------------------------------------------
        //debug: render depthMap to screen using quad 
        //----------------------------------------------------------------------------------
        /*glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        screenShader.activateShader();
        screenShader.setUniformFloat("near", near_plane);
        screenShader.setUniformFloat("far", far_plane);
        glBindVertexArray(screenQuadVAO); 
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glDrawArrays(GL_TRIANGLES, 0, 6);*/
        //---------------------------------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------------------------------

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        
        //setting up projection and view matrices
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

        //glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

        glm::mat4 view = newCamera.getViewMatrix();
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        //------------------------------------------------------------------------------------
        
        //activate shader and pass uniforms to it
        lightingShader.activateShader();
        lightingShader.setUniformVec3("cameraPos", newCamera.getEye());
        lightingShader.setUniformInt("gamma", GAMMA_ENABLED);

        //lightingShader.setUniformMatrix4("lightSpaceMatrix", lightSpaceMatrix);


        lightingShader.setUniformFloat("near", near_plane);
        lightingShader.setUniformFloat("far", far_plane);

        //bind depth map to shadow map texture in the shader
        lightingShader.setUniformInt("shadowMap", 1); 
        lightingShader.setUniformInt("shadowCubeMap", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);

        //send point light values to shader
        lightingShader.setUniformVec3("point_light.position", pointLight.position);
        lightingShader.setUniformVec3("point_light.ambient", pointLight.ambient);
        lightingShader.setUniformVec3("point_light.diffuse", pointLight.diffuse);
        lightingShader.setUniformVec3("point_light.specular", pointLight.specular);
        lightingShader.setUniformFloat("point_light.constant", pointLight.constant);
        lightingShader.setUniformFloat("point_light.linear", pointLight.linear);
        lightingShader.setUniformFloat("point_light.quadratic", pointLight.quadratic);
        //----------------------------------------------------------------------------------

        //draws floor
        glStencilMask(0x00); //disable writing to the stencil buffer, prevents floor from affecting the rendering of the cube border
        glBindVertexArray(planeVAO);
        glm::mat4 rotationMatrix = glm::rotate(identityMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
        lightingShader.setUniformMatrix4("rotationMatrix", rotationMatrix);
        lightingShader.setUniformMatrix4("model", identityMatrix);

        //sets material properties
        lightingShader.setUniformInt("material.diffuseMap", 0);
        lightingShader.setUniformInt("material.specularMap", 0); //use the floor texture as a specular map
        lightingShader.setUniformInt("material.emissionMap", 2);
        lightingShader.setUniformFloat("material.shininess", 64.0f);
        //-----------------------------------------------------------------------

        glActiveTexture(GL_TEXTURE0); //sets texture unit as the current active texture unit
        glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? floorTextureGammaCorrected : floorTexture);//binds texture objects to the current active texture unit in texture target
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //--------------------------------------------------

        //draw cubes
        glBindTexture(GL_TEXTURE_2D, GAMMA_ENABLED ? cubeTextureGammaCorrected : cubeTexture); //binds texture objects to the current active texture unit in texture target
        //glBindTexture(GL_TEXTURE_2D, depthMap);
        drawTwoContainers(cubeVAO, lightingShader, projection, view, 1);
        drawCube(cubeVAO, lightingShader, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1, 1, 1));
        //-----------------------------------------------------------------------------------

        //Draw Point light
        lightSourceShader.activateShader();
        drawCube(lightObjectVAO, lightSourceShader, pointLight.position, glm::vec3(0.2, 0.2, 0.2));
        lightSourceShader.setUniformVec3("lightColor", pointLight.diffuse);
        //------------------------------------------------------------------------------------------------

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents(); //poll IO events(keys pressed/released, mouse moved etc.)
    }

    //clean up resources
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &screenQuadVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &screenQuadVBO);

    glfwTerminate();
    //-----------------------------------------------

    return 0;
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
    rotationMatrix = glm::rotate(rotationMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));

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
    model = translationMatrix * rotationMatrix * scaleMatrix;
    shader.setUniformMatrix4("model", model);
    shader.setUniformMatrix4("rotationMatrix", rotationMatrix);
    shader.setUniformBool("useSkybox", false);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //---------------------------------------------------

    glBindVertexArray(0);
}

void drawCube(GLuint cubeVAO, const Shader& shader, const glm::vec3 translationVec, glm::vec3 scale) {
    glBindVertexArray(cubeVAO);

    glm::mat4 identityMatrix = glm::mat4(1.0);
    glm::mat4 scaleMatrix = glm::scale(identityMatrix, scale);
    glm::mat4 translationMatrix = glm::translate(identityMatrix, translationVec);
    glm::mat4 model = translationMatrix * scaleMatrix;
    shader.setUniformMatrix4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, 36);
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
    exit(-1);
}