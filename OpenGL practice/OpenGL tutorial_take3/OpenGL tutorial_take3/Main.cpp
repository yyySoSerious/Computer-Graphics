#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader.h"
#include "Camera.h"
#include "Light.h"

//Window dimensions
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

//Change in  time between current frame and last frame
float deltaTime = 0.0f; 

////coordinates for the last mouse position
//float lastMouseX = (float) WINDOW_WIDTH / 2;
//float lastMouseY = (float) WINDOW_HEIGHT / 2;


//camera
Camera newCamera(glm::vec3(0.0f, 0.0f, 3.0f));

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos); 
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

void processInput(GLFWwindow* window, Shader& shader, Camera& camera);
void initializeTextures(const char* textureFile, GLint textureID);
void bindTextureToUnit(GLint textureID, GLint textureUnit);

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



    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //float vertices[] = {
    //    //positions         colours            texture coords 
    //    0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, // top right
    //    0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom right
    //   -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // bottom left
    //   -0.5f,  0.5f, 0.0f,  0.9f, 0.5f, 0.7f,  0.0f, 1.0f  // top left 
    //};

    //cube vertices
    float vertices[] = {
        //position              text coord      normal
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,     0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,     0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,     0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,     0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     -1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 1.0f,     0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,     0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,     0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f
    };

    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    unsigned int indices[] = {
        0, 1, 3,  //first triangle
        1, 2, 3   //second triangle
    };

    float texCoords[] = {
        0.0f, 0.0f,  // lower-left corner  
        1.0f, 0.0f,  // lower-right corner
        0.5f, 1.0f   // top-center corner
    };

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    //load shaders
    Shader lightingShader = Shader("lightingShader.vert", "lightingShader.frag");
    Shader lightSourceShader = Shader("lightSourceShader.vert", "lightSourceShader.frag");
    
    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //link position attribute in the vertex data to the shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //link colour attribute in the vertex data to the shader
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //link text coords attribute in the vertex data to the shader
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //link normal attribute in the vertex data to the shader
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbinds vbo

    glBindVertexArray(0); //unbinds vao

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); //unbinds ebo

    /****   set up light object   ****/
    unsigned int lightObjectVAO;
    glGenVertexArrays(1, &lightObjectVAO);
    glBindVertexArray(lightObjectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //VBO already contains the data so no need add data with glBufferData()
    
    //link text coords attribute in the vertex data to the shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    /********************************/

    //------------------------------------------------------------------------------------
    //create and bind texture objects
    unsigned int textures[3];
    glGenTextures(3, textures);

    stbi_set_flip_vertically_on_load(true);
    initializeTextures("../../Textures/container2.png", textures[0]); //texture 0
    initializeTextures("../../Textures/container2_specular.png", textures[1]); //texture 1
    initializeTextures("../../Textures/emissionMap.jpg", textures[2]); //texture 2
    
    // bind textures on corresponding texture units. If not done, the last bound texture object will be
    // on the default texture unit, which is most likey 0
    bindTextureToUnit(textures[0], 0);
    bindTextureToUnit(textures[1], 1);
    bindTextureToUnit(textures[2], 2);
    //produces wireframe polygons
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    

    glUniform1i(glGetUniformLocation(lightingShader.getProgramId(), "texture1"), 0); //set sampler (texture1) to texture unit 0
    glUniform1i(glGetUniformLocation(lightingShader.getProgramId(), "texture2"), 1); // ditto above

    //setting up a static camera
    /*glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);*/
   /* glm::mat4 view;
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));*/
    

    DirLight dirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.05f, 0.05f, 0.05f));
    glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
    }; 

    glm::vec3 colours[] = {
    glm::vec3(0.7f,  0.2f,  0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f,  0.0f, 1.0f),
    glm::vec3(0.2f,  0.5f, 0.8f)
    };

    const unsigned int numLights = 4;
    PointLight pointLights[numLights];
    for (unsigned int i = 0; i < numLights; ++i) {
        pointLights[i].position = pointLightPositions[i];
        pointLights[i].diffuse = colours[i];
        pointLights[i].ambient = pointLights[i].diffuse * 0.2f;// glm::vec4(0.2f);
    }


   // glUniform1i(horOffsetLocation, horizontalOffset);
    //main render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f; //The time of last frame

        float currentFrame = glfwGetTime(); //current time
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        
        glm::vec3 lightTranslate(1.2f, 1.0f, 2.0f);
        float currentTime = (float)glfwGetTime();
        //glm::vec3 lightPos(lightTranslate.x * sin(currentTime), lightTranslate.y * cos(currentTime), lightTranslate.z);


        processInput(window, lightingShader, newCamera);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        lightingShader.activateShader();
        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) + 1.0f) / 2.0f;
        int vertexColorLocation = glGetUniformLocation(lightingShader.getProgramId(), "ourColor");
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

        //setting up projection matrix and passing it to the shader
        glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.getProgramId(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        //setting up view matrix and passing it to the shader
        glm::mat4 view = newCamera.getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.getProgramId(), "view"), 1, GL_FALSE, &view[0][0]);

        //Draw cube boxes
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; ++i)
        {
            glm::mat4 translationMatrix = glm::mat4(1.0f);
            translationMatrix = glm::translate(translationMatrix, cubePositions[i]);

            glm::mat4 rotationMatrix = glm::mat4(1.0f);
            float angle = 20.0f;
            angle = (i % 3) == 0 ? (float)glfwGetTime() * angle : angle * i;
            rotationMatrix = glm::rotate(rotationMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            
            glm::mat4 model = translationMatrix * rotationMatrix;// glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(glGetUniformLocation(lightingShader.getProgramId(), "model"), 1, GL_FALSE, &model[0][0]);
            lightingShader.setUniformMatrix4("rotationMatrix", rotationMatrix);
            glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "viewPos"), 1, &newCamera.getEye()[0]);

            //sets material properties
            glUniform1i(glGetUniformLocation(lightingShader.getProgramId(), "material.diffuseMap"), 0);
            glUniform1i(glGetUniformLocation(lightingShader.getProgramId(), "material.specularMap"), 1);
            glUniform1i(glGetUniformLocation(lightingShader.getProgramId(), "material.emissionMap"), 2);
            glUniform1f(glGetUniformLocation(lightingShader.getProgramId(), "material.shininess"), 64.0f);
            //--------------------------------

            //sets light properties
            /*lightColor.x = sin(glfwGetTime() * 2.0f);
            lightColor.y = sin(glfwGetTime() * 0.7f);
            lightColor.z = sin(glfwGetTime() * 1.3f);*/
           
            //glm::vec3 diffuseColor = glm::vec3(1.0) * glm::vec3(0.5f);
            //glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
            //lightColor = diffuseColor;

            ////send uniform values for a flash light with a range of 50
            //glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "light.ambient"), 1, &ambientColor[0]);
            //glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "light.diffuse"), 1, &diffuseColor[0]);
            //glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "light.specular"), 1, &glm::vec3(1.0)[0]);
            //glm::vec3 flashLightPos(newCamera.getEye());
            //glUniform1f(glGetUniformLocation(lightingShader.getProgramId(), "light.constant"), 1.0f);
            //glUniform1f(glGetUniformLocation(lightingShader.getProgramId(), "light.linear"), 0.09f);
            //glUniform1f(glGetUniformLocation(lightingShader.getProgramId(), "light.quadratic"), 0.032f);
            //glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "light.position"), 1, &flashLightPos[0]);
            //glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "light.spotDirection"), 1, &newCamera.getForward()[0]);
            //glUniform1f(glGetUniformLocation(lightingShader.getProgramId(), "light.cutOff"), glm::cos(glm::radians(12.5f)));
            //glUniform1f(glGetUniformLocation(lightingShader.getProgramId(), "light.outerCutOff"), glm::cos(glm::radians(17.5f)));
            ////-------------------------------

            //send uniform values for directional Light
            glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "dirLight.direction"), 1, &dirLight.direction[0]);
            glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "dirLight.ambient"), 1, &dirLight.ambient[0]);
            glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "dirLight.diffuse"), 1, &dirLight.diffuse[0]);
            glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), "dirLight.specular"), 1, &dirLight.specular[0]);

            //send uniform values for point lights
            for (unsigned int i = 0; i < numLights; ++i) {
                std::string lightString = "pointLights[" + std::to_string(i) + "].";
                glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(),
                    std::string(lightString + "position").c_str()) , 1, &pointLights[i].position[0]);
                glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(), 
                    std::string(lightString + "ambient").c_str()), 1, &pointLights[i].ambient[0]);
                glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(),
                    std::string(lightString + "diffuse").c_str()), 1, &pointLights[i].diffuse[0]);
                glUniform3fv(glGetUniformLocation(lightingShader.getProgramId(),
                    std::string(lightString + "specular").c_str()), 1, &pointLights[i].specular[0]);
                glUniform1f(glGetUniformLocation(lightingShader.getProgramId(),
                    std::string(lightString + "constant").c_str()), pointLights[i].constant);
                glUniform1f(glGetUniformLocation(lightingShader.getProgramId(),
                    std::string(lightString + "linear").c_str()), pointLights[i].linear);
                glUniform1f(glGetUniformLocation(lightingShader.getProgramId(),
                    std::string(lightString + "quadratic").c_str()), pointLights[i].quadratic);
            }
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        //------------------------------------------


        //Draw Point Light objects

        //pass mvp matrices and light color to the lightSource shader
        lightSourceShader.activateShader();
        glUniformMatrix4fv(glGetUniformLocation(lightSourceShader.getProgramId(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightSourceShader.getProgramId(), "view"), 1, GL_FALSE, &view[0][0]);
        glBindVertexArray(lightObjectVAO);
        for (unsigned int i = 0; i < numLights; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLights[i].position);
            model = glm::scale(model, glm::vec3(0.2f));
            glUniformMatrix4fv(glGetUniformLocation(lightSourceShader.getProgramId(), "model"), 1, GL_FALSE, &model[0][0]);
            glUniform3fv(glGetUniformLocation(lightSourceShader.getProgramId(), "lightColor"), 1, &pointLights[i].diffuse[0]);
            //-------------------------------------------------------

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        //----------------------------------------------------------

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents(); //poll IO events(keys pressed/released, mouse moved etc.)
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    //clean up resources
    glfwTerminate();
    return 0;
}

//set texture parameters for the texture object and loads the texture image for the object
//Note: it expects the image to be RGB or RGBA format
void initializeTextures(const char* textureFile, GLint textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID); //binds texture to target, GL_TEXTURE_2D

    //sets texture wrapping and filtering options for the currently bound texture object
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //if we choose GL_CLAMP_TO_BORDER option for the wrapping
    float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    //load image from file
    int width, height, nrChannels;
    unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, 0);

    GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
    //create texture image for the bound texture object and generate mipmaps from the now attached texture image
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "ERROR: Cannot load texture file: " << textureFile << std::endl;
        exit(-1);//use something better
    }

    //free texture data
    stbi_image_free(data);
}

//bind texture to unit
void bindTextureToUnit(GLint textureID, GLint textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit); //activates texture unit
    glBindTexture(GL_TEXTURE_2D, textureID);
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
    glViewport(0, 0, width, height);
}

/*  This call back is called when there is mouse movement on the screen
*   Parameters:
*       window: Current window
*       xPos, yPos: Current mouse position
* */
void mouse_callback(GLFWwindow* window, double mouseX, double mouseY) {
    static float lastMouseX = mouseX;
    static float lastMouseY = mouseY;

    float xOffset = mouseX - lastMouseX;
    float yOffset = lastMouseY - mouseY;
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


