#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<iostream>
#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"

//Window dimensions
const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

//Change in  time between current frame and last frame
float deltaTime = 0.0f; 
//camera
Camera newCamera(glm::vec3(0.0f, 0.0f, 3.0f));

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos); 
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

void processInput(GLFWwindow* window, Shader& shader, Camera& camera);

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

    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    float planeVertices[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };
    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    //load shaders
    Shader modelShader = Shader("modelShader.vert", "modelShader.frag");

    DirLight dirLight(glm::vec3(-0.2f, -1.0f, -0.3f));// , glm::vec3(0.05f, 0.05f, 0.05f));
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

    Model modelObject("../../Models/backpack/backpack.obj");

    //main render loop
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f; //The time of last frame

        float currentFrame = glfwGetTime(); //current time
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, modelShader, newCamera);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        modelShader.activateShader();

        //setting up projection matrix and passing it to the shader
        glm::mat4 projection = glm::perspective(glm::radians(newCamera.getFOV()), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(modelShader.getProgramId(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        //setting up view matrix and passing it to the shader
        glm::mat4 view = newCamera.getViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(modelShader.getProgramId(), "view"), 1, GL_FALSE, &view[0][0]);

        //setting up model matrix and passing it to the shader
        glm::mat4 rotationMatrix(1.0f);
        glm::mat4 model(1.0f); 
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        modelShader.setUniformMatrix4("rotationMatrix", rotationMatrix);
        modelShader.setUniformMatrix4("model", model);
        modelShader.setUniformVec3("viewPos", newCamera.getEye());
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //draw object
        modelObject.draw(modelShader);

        //send uniform values for directional Light
        glUniform3fv(glGetUniformLocation(modelShader.getProgramId(), "dirLight.direction"), 1, &dirLight.direction[0]);
        glUniform3fv(glGetUniformLocation(modelShader.getProgramId(), "dirLight.ambient"), 1, &dirLight.ambient[0]);
        glUniform3fv(glGetUniformLocation(modelShader.getProgramId(), "dirLight.diffuse"), 1, &dirLight.diffuse[0]);
        glUniform3fv(glGetUniformLocation(modelShader.getProgramId(), "dirLight.specular"), 1, &dirLight.specular[0]);

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents(); //poll IO events(keys pressed/released, mouse moved etc.)
    }

    //clean up resources
    glfwTerminate();
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
    glViewport(0, 0, width, height);
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


