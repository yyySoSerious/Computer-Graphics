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
#include "Model.h"

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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

unsigned int textureFromFile(const char* textureFile);
void processInput(GLFWwindow* window, Shader& shader, Camera& camera);
void drawTwoContainers(GLuint cubeVAO, const Shader& shader, const glm::mat4& projection, const glm::mat4& view, float scale);
unsigned int cubeMapFromFile(const std::vector<const char*>& faces);
void drawCube(GLuint cubeVAO, const Shader& shader, const glm::vec3 translationVec, float scale);
void createFBO(unsigned int& framebuffer, unsigned int& texColorBuffer);
void createMSAA_FBO(unsigned int& multisampledFBO, unsigned int& msaa_texColorBuffer, unsigned int numSamples);

int main(int argc, char* argv[]) {
    const unsigned int NUM_SAMPLES = 4;

    //initialize glfw and set context options
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    //glEnable(GL_MULTISAMPLE); //enable multisampling anti-aliasing(MSAA)
    //--------------------------------------------------------------------

    //load shaders
    Shader shader = Shader("allPurposeShader.vert", "allPurposeShader.frag");
    Shader skyboxShader = Shader("Skybox.vert", "Skybox.frag");
    Shader modelShader = Shader("modelShader.vert", "modelShader.frag", "modelShader.geom");
    Shader shaderRed = Shader("allPurposeShader.vert", "red.frag");
    Shader shaderGreen = Shader("allPurposeShader.vert", "green.frag");
    Shader shaderBlue = Shader("allPurposeShader.vert", "blue.frag");
    Shader shaderYellow = Shader("allPurposeShader.vert", "yellow.frag");
    Shader basicShader = Shader("basicShader.vert", "basicShader.frag", "basicShader.geom");
    Shader normalDisplayShader = Shader("modelShader.vert", "yellow.frag", "modelShader2.geom");
    Shader instancingShader = Shader("Instancing.vert", "Instancing.frag");
    Shader planetShader = Shader("Planet.vert", "Planet.frag");
    Shader asteroidShader = Shader("asteroid.vert", "asteroid.frag");
    Shader screenShader = Shader("screenShader.vert", "screenShader.frag");
    //--------------------------------------------------------------

    float skyboxVertices[] = {
        // positions 
        //back face CW
        -1.0f,  1.0f, -1.0f, //top-right
        -1.0f, -1.0f, -1.0f, //bottom-right
         1.0f, -1.0f, -1.0f, //bottom-left
         1.0f, -1.0f, -1.0f, //bottom-left
         1.0f,  1.0f, -1.0f, //top-left
        -1.0f,  1.0f, -1.0f, //top-right

        //left face CW
        -1.0f, -1.0f,  1.0f, //bottom-right
        -1.0f, -1.0f, -1.0f, //bottom-left
        -1.0f,  1.0f, -1.0f, //top-left
        -1.0f,  1.0f, -1.0f, //top-left
        -1.0f,  1.0f,  1.0f, //top-right
        -1.0f, -1.0f,  1.0f, //bottom-right

        //right face CW
         1.0f, -1.0f, -1.0f, //bottom-right
         1.0f, -1.0f,  1.0f, //bottom-left
         1.0f,  1.0f,  1.0f, //top-left
         1.0f,  1.0f,  1.0f, //top-left
         1.0f,  1.0f, -1.0f, //top-right
         1.0f, -1.0f, -1.0f, //bottom-right

         //front face CW
        -1.0f, -1.0f,  1.0f, //bottom-left
        -1.0f,  1.0f,  1.0f, //top-left
         1.0f,  1.0f,  1.0f, //top-right
         1.0f,  1.0f,  1.0f, //top-right
         1.0f, -1.0f,  1.0f, //bottom-right
        -1.0f, -1.0f,  1.0f, //bottom-left

        //top face CW
        -1.0f,  1.0f, -1.0f, //top-left
         1.0f,  1.0f, -1.0f, //top-right
         1.0f,  1.0f,  1.0f, //bottom-right
         1.0f,  1.0f,  1.0f, //bottom-right
        -1.0f,  1.0f,  1.0f, //bottom-left
        -1.0f,  1.0f, -1.0f, //top-left

        //bottom face CW
        -1.0f, -1.0f, -1.0f, //top-right
        -1.0f, -1.0f,  1.0f, //bottom-right
         1.0f, -1.0f, -1.0f, //top-left
         1.0f, -1.0f, -1.0f, //top-left
        -1.0f, -1.0f,  1.0f, //bottom-right
         1.0f, -1.0f,  1.0f  //bottom-left
    };

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

    float points[] = {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    float quadVertices[] = {
        // positions     // colors
        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
         0.05f,  0.05f,  0.0f, 1.0f, 1.0f
    };

    std::vector<glm::vec3> vegetation;
    vegetation.push_back(glm::vec3(-1.0f, 0.0f, -0.48f));
    vegetation.push_back(glm::vec3(2.0f, 0.0f, 0.51f));
    vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));
    //--------------------------------------------------------

    //100000 transformation model matrices for asteroids
    unsigned int amount = 1000;
    std::vector<glm::mat4> modelMatrices(amount);
    //modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime());
    float radius = 50.0;
    float offset = 2.5f;
    for (unsigned int i = 0; i < amount; ++i) {
        glm::mat4 model = glm::mat4(1.0f);
        //translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; //keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        //scale: scale between 0.05 and 0.25
        float scale = (rand() % 20) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));

        //rotation: add random rotation around a (semi) randomly picked rotation axis vector
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        //add to list of matrices
        modelMatrices[i] = model;
    }
    //--------------------------------------------------------

    //load model into object
    Model modelObject("../../Models/backpack/backpack.obj");
    Model asteroid("../../Models/rock model/rock.obj", &modelMatrices);
    Model planet("../../Models/planet/planet.obj");
    //----------------------------------------------------------

    //setup MSAA_framebuffer
    unsigned int multisampledFBO, msaa_texColorBuffer;
    createMSAA_FBO(multisampledFBO, msaa_texColorBuffer, NUM_SAMPLES);
    //------------------------------------------------------------------------------------------------------------------

    //setup Iintermediate framebuffer
    unsigned int intermediateFBO, screenTexture;
    createFBO(intermediateFBO, screenTexture);
    //-------------------------------------------------------------------------------------------------------------

    //skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    //----------------------------------------

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

    //quad VAO (general quad)
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

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
    
    //points vao
    unsigned int pointsVAO, pointsVBO;
    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);

    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0); //unbinds vao
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo
    //--------------------------------------------------------------------------------------------

    //-----------------------------------------------------------------------------------
    //setting up uniform block object 
    //-------------------------------------------------------------------------------------------
    //get relevant block indices
    unsigned int uniformBlockIndexRed = glGetUniformBlockIndex(shaderRed.getProgramId(), "Matrices");
    unsigned int uniformBlockIndexGreen = glGetUniformBlockIndex(shaderGreen.getProgramId(), "Matrices");
    unsigned int uniformBlockIndexBlue = glGetUniformBlockIndex(shaderBlue.getProgramId(), "Matrices");
    unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(shaderYellow.getProgramId(), "Matrices");

    //link each shader's uniform block indices to uniform binding point 0
    glUniformBlockBinding(shaderRed.getProgramId(), uniformBlockIndexRed, 0); //bind uniform block to binding loc
    glUniformBlockBinding(shaderGreen.getProgramId(), uniformBlockIndexRed, 0);
    glUniformBlockBinding(shaderBlue.getProgramId(), uniformBlockIndexBlue, 0);
    glUniformBlockBinding(shaderYellow.getProgramId(), uniformBlockIndexYellow, 0);
    //create ubo buffer
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4)); //bind uniform buffer object to binding point(loc) 0
    //----------------------------------------------------------------------------------------------------------

    //sending uniforms to the shader
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM, 0);
    //-----------------------------------------------------------------------------------------

    //load textures
    unsigned int cubeTexture = textureFromFile("../../Textures/container.jpg"); //textureFromFile("iron_texture.jpg");
    unsigned int floorTexture = textureFromFile("../../Textures/marble.jpg");
    //-----------------------------------------------

    //load cubemap
    std::vector<const char*> faces{ 
        "../../Textures/mountain_ranges_in_ocean Skybox/right.jpg",
        "../../Textures/mountain_ranges_in_ocean Skybox/left.jpg", 
        "../../Textures/mountain_ranges_in_ocean Skybox/top.jpg",
        "../../Textures/mountain_ranges_in_ocean Skybox/bottom.jpg", 
        "../../Textures/mountain_ranges_in_ocean Skybox/front.jpg", 
        "../../Textures/mountain_ranges_in_ocean Skybox/back.jpg"
    };
    unsigned int cubemapTexture = cubeMapFromFile(faces);
    //-----------------------------------------------------------------------------------------------------

    //100 translation vectors
    glm::vec2 translations[100];
    int index = 0;
    offset = 0.1f;
    for (int y = -10; y < 10; y += 2)
    {
        for (int x = -10; x < 10; x += 2)
        {
            glm::vec2 translation;
            translation.x = (float)x / 10.0f + offset;
            translation.y = (float)y / 10.0f + offset;
            translations[index++] = translation;
        }
    }
    //----------------------------------------------------


    //setting up instanced array
    glBindVertexArray(quadVAO);
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(translations), translations, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1); //tells OpenGL to update the attribute in location 2 every (1) instance 
    glBindVertexArray(0);
    //-----------------------------------------------------

    //main render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f; //The time of last frame
        float currentFrame = glfwGetTime(); //current time
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, shader, newCamera);

        //draw scene in MSAA framebuffer (first pass)
        glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        //glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //setting up projection and view matrices
        //glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = newCamera.getViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        //------------------------------------------------------------------------------------
        
        //activate shader and pass uniforms to it
        shader.activateShader();
        shader.setUniformVec3("cameraPos", newCamera.getEye());
        //----------------------------------------------------------------------------------

        ////draw 100 quads
        //instancingShader.activateShader();
        ///*for (unsigned int i = 0; i < 100; ++i) {
        //    instancingShader.setUniformVec2("offsets[" + std::to_string(i) + "]", translations[i]);
        //}*/
        //glBindVertexArray(quadVAO);
        //glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
        ////------------------------------------------------------------------------------------------

        ////draw planet and meterites
        ////planet
        //planetShader.activateShader();
        //planetShader.setUniformMatrix4("projection", projection);
        //planetShader.setUniformMatrix4("view", view);
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        //planetShader.setUniformMatrix4("model", model);
        //planet.draw(planetShader);

        ////meteorites
        //asteroidShader.activateShader();
        //asteroidShader.setUniformMatrix4("projection", projection);
        //asteroidShader.setUniformMatrix4("view", view);
        //asteroid.draw(asteroidShader);
        ////----------------------------------------------------------------------------------------------

        ////draw points
        //basicShader.activateShader();
        //glBindVertexArray(pointsVAO);
        //glDrawArrays(GL_POINTS, 0, 4);
        ////-----------------------------------------------------------------------------------------------

        ////draw 4 cubes
        ////RED
        //shaderRed.activateShader();
        //drawCube(cubeVAO, shaderRed, glm::vec3(-0.75f, 0.75f, 0.0f), 1);
        ////GREEN
        //shaderGreen.activateShader();
        //drawCube(cubeVAO, shaderGreen, glm::vec3(0.75f, 0.75f, 0.0f), 1);
        ////YELLOW
        //shaderYellow.activateShader();
        //drawCube(cubeVAO, shaderYellow, glm::vec3(-0.75f, -0.75f, 0.0f), 1);
        ////YELLOW
        //shaderBlue.activateShader();
        //drawCube(cubeVAO, shaderBlue, glm::vec3(0.75f, -0.75f, 0.0f), 1);
        //---------------------------------------------------------------------------------------------------------------------
        
        ////draws floor
        //glStencilMask(0x00); //disable writing to the stencil buffer, prevents floor from affecting the rendering of the cube border
        //glBindVertexArray(planeVAO);
        //shader.setUniformMatrix4("model", glm::mat4(1.0f));


        //glActiveTexture(GL_TEXTURE0); //sets texture unit as the current active texture unit
        //glBindTexture(GL_TEXTURE_2D, floorTexture);//binds texture objects to the current active texture unit in texture target
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        ////--------------------------------------------------

        ////draw cubes
        //glBindTexture(GL_TEXTURE_2D, cubeTexture); //binds texture objects to the current active texture unit in texture target
        //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture); //binds texture objects to the current active texture unit in texture target
        //drawTwoContainers(cubeVAO, shader, projection, view, 1);
        ////-----------------------------------------------------------------------------------

        //draw model with normals displayed
        auto drawModel = [&modelObject, &projection, &view](Shader &shader)->void {
            shader.activateShader();
            shader.setUniformMatrix4("projection", projection);
            shader.setUniformMatrix4("view", view);

            glm::mat4 scaleMatrix = glm::mat4(1.0f);
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, -10.0f));
            glm::mat4 model = translationMatrix * rotationMatrix * scaleMatrix;
            shader.setUniformMatrix4("rotationMatrix", rotationMatrix);
            shader.setUniformMatrix4("model", model);
            shader.setUniformVec3("cameraPos", newCamera.getEye());
            shader.setUniformFloat("time", glfwGetTime());

            //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture); //binds texture objects to the current active texture unit in texture target
            modelObject.draw(shader); //draw model object
        };
        drawModel(modelShader);
        normalDisplayShader.activateShader();
        drawModel(normalDisplayShader);
        //-------------------------------------------------------------------------------------

        ////draw skybox
        ////------------------------------------------------------------------------------------
        //view = glm::mat4(glm::mat3(view)); 

        ////removes translation from view matrix//ensures skybox passes depth test because its depth value is 1.0 
        ////(set in the skybox shader) and the default values for the depth buffer when they are not overwritten is
        //// 1.0 as well
        //glDepthFunc(GL_LEQUAL);

        //skyboxShader.activateShader();
        //skyboxShader.setUniformMatrix4("projection", projection);
        //skyboxShader.setUniformMatrix4("view", view);

        //glBindVertexArray(skyboxVAO);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //glDepthFunc(GL_LESS);
        ////------------------------------------------------------------------------------------

        ////second pass: blit multisampled buffer(s) to an intermediate buffer
        //glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
        //glBlitFramebuffer(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        ////third pass: empty contents from the screen texture (attached to the intermediate FBO) to the screen 
        //glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0); //back to default
        //glDisable(GL_DEPTH_TEST); //To ensure quad draws in front of everything else
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        //screenShader.activateShader();
        //glBindVertexArray(screenQuadVAO);
        //glBindTexture(GL_TEXTURE_2D, screenTexture);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        //-------------------------------------------------------------------------------------------------------------------

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
}

void drawTwoContainers(GLuint cubeVAO, const Shader& shader, const glm::mat4& projection, const glm::mat4& view, float scale) {
    //passing view and projection matrices to the shader
    shader.setUniformMatrix4("projection", projection);
    shader.setUniformMatrix4("view", view);
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

void drawCube(GLuint cubeVAO, const Shader& shader, const glm::vec3 translationVec, float scale) {
    glBindVertexArray(cubeVAO);
    glm::mat4 model = glm::translate(glm::mat4(1.0), translationVec);
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

void createMSAA_FBO(unsigned int& multisampledFBO, unsigned int& msaa_texColorBuffer, unsigned int numSamples) {
    //setup MSAA framebuffer
    glGenFramebuffers(1, &multisampledFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);

    //generate multisampled texture as color attachement
    glGenTextures(1, &msaa_texColorBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa_texColorBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGB, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
