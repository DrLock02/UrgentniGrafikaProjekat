#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string>& faces);

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

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;


// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutoff;
    float outer_cutoff;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 shrinePosition = glm::vec3(0.0f,0.0f,-8.0f);
    glm::vec3 patosPosition = glm::vec3(-0.45f,-0.5f,-0.45f);
    glm::vec3 sukunaPosition = glm::vec3(0.0f,-0.5f,-2.0f);
    glm::vec3 pillarPosition = glm::vec3(-5.0f,-0.5f,5.0f);
    glm::vec3 lumberPosition = glm::vec3(7.0f,0.1f,8.0f);
    glm::vec3 mahoragaPosition = glm::vec3(-0.45f,-0.5f,10.0f);
    glm::vec3 buildingPosition = glm::vec3(9.0f,-0.5f,-1.3f);
    glm::vec3 towerPosition = glm::vec3(-9.0f,-0.55f,13.0f);
    float shrineScale = 1.15f;
    float patosScale = 5.0f;
    float sukunaScale = 1.15f;
    float pillarScale = 0.22f;
    float lumberScale = 0.22f;
    float mahoragaScale = 0.17f;
    float buildingScale = 0.6f;
    float towerScale = 0.3f;
    SpotLight spotlight1;
    SpotLight spotlight2;

    float far_plane = 115.5f;
    float near_plane = 89.0f;
    float lightPosScale = 1.0f;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Domain Expansion: Malevolent Shrine", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");

    Shader skyboxShader("resources/shaders/Skybox.vs", "resources/shaders/Skybox.fs");

    Shader simpleDepthShader("resources/shaders/shadow_mapping_depth.vs","resources/shaders/shadow_mapping_depth.fs");

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // skybox vertex initialization

    unsigned int skyboxVAO, skyboxVBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(0);

    // creating and loading skybox
    unsigned int cubemapTexture;
        vector<std::string> faces
                {
                        "resources/textures/skybox/left.png",
                        "resources/textures/skybox/right.png",
                        "resources/textures/skybox/top.png",
                        "resources/textures/skybox/bottom.png",
                        "resources/textures/skybox/back.png",
                        "resources/textures/skybox/front.png"
                };
        cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    ourShader.use();
    ourShader.setInt("material.texture_diffuse1", 0);
    ourShader.setInt("material.texture_specular1", 1);
    ourShader.setInt("shadowMap", 2);



    Model ourModel("resources/objects/MalevolentShrineOBJ/untitled.obj");
    ourModel.SetShaderTextureNamePrefix("material.");

    Model modelPatos("resources/objects/PatosOBJ/untitled.obj");
    modelPatos.SetShaderTextureNamePrefix("material.");

    Model modelSukuna("resources/objects/SukunaOBJ/untitled.obj");
    modelSukuna.SetShaderTextureNamePrefix("material.");

    Model modelPillar("resources/objects/BrokenPillarOBJ/untitled.obj");
    modelPillar.SetShaderTextureNamePrefix("material.");

    Model modelBuilding("resources/objects/RuinedBuildingOBJ/untitled.obj");
    modelBuilding.SetShaderTextureNamePrefix("material.");

    Model modelLumber("resources/objects/WasteLumberOBJ/untitled.obj");
    modelLumber.SetShaderTextureNamePrefix("material.");

    Model modelTower("resources/objects/RuinedTowerOBJ/untitled.obj");
    modelTower.SetShaderTextureNamePrefix("material.");

    Model modelMahoraga("resources/objects/MahoragaOBJ/untitled.obj");
    modelMahoraga.SetShaderTextureNamePrefix("material.");
    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);



    // load models
    // -----------

    SpotLight& spotlight1 = programState->spotlight1;
    spotlight1.position = glm::vec3(0.077f, 27.783f, -8.0f);
    spotlight1.direction = glm::vec3(0.0f,-1.0f,0.0f);
    spotlight1.ambient = glm::vec3(0.12, 0.05, 0.05);
    spotlight1.diffuse = glm::vec3(2.5, 1.0, 1.0);
    spotlight1.specular = glm::vec3(1.0, 1.0, 1.0);

    spotlight1.constant = 1.0f;
    spotlight1.linear = 0.003f;
    spotlight1.quadratic = 0.001f;

    spotlight1.cutoff = 7.0f;
    spotlight1.outer_cutoff = 15.0f;

    SpotLight& spotlight2 = programState->spotlight2;
    spotlight2.position = glm::vec3(-0.3f, 18.69f, 8.9f);
    spotlight2.direction = glm::vec3(0.0f,-1.0f,0.0f);
    spotlight2.ambient = glm::vec3(0.12, 0.05, 0.05);
    spotlight2.diffuse = glm::vec3(2.47, 2.26, 1.21);
    spotlight2.specular = glm::vec3(1.0, 1.0, 1.0);

    spotlight2.constant = 1.0f;
    spotlight2.linear = 0.007f;
    spotlight2.quadratic = 0.001f;

    spotlight2.cutoff = 4.0f;
    spotlight2.outer_cutoff = 9.0f;

    glm::vec3 lightPos = glm::vec3(-60.0f,-50.0f,-60.0f);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = programState->near_plane, far_plane = programState->far_plane, lightPosScale = programState->lightPosScale;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
        lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
        lightView = glm::lookAt(-lightPos*lightPosScale, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
        // render scene from light's point of view
        simpleDepthShader.use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->shrinePosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->shrineScale));    // it's a bit too big for our scene, so scale it down
        simpleDepthShader.setMat4("model", model);
        ourModel.Draw(simpleDepthShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->patosPosition);
        model = glm::scale(model, glm::vec3(programState->patosScale));
        simpleDepthShader.setMat4("model", model);
//        glCullFace(GL_FRONT);
        modelPatos.Draw(simpleDepthShader);
//        glCullFace(GL_BACK);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->sukunaPosition);
        model = glm::scale(model, glm::vec3(programState->sukunaScale));
        simpleDepthShader.setMat4("model", model);
        modelSukuna.Draw(simpleDepthShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->pillarPosition);
        model = glm::scale(model, glm::vec3(programState->pillarScale));
        simpleDepthShader.setMat4("model", model);
        modelPillar.Draw(simpleDepthShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->lumberPosition);
        model = glm::scale(model, glm::vec3(programState->lumberScale));
        simpleDepthShader.setMat4("model", model);
        modelLumber.Draw(simpleDepthShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->towerPosition);
        model = glm::scale(model, glm::vec3(programState->towerScale));
        simpleDepthShader.setMat4("model", model);
        modelTower.Draw(simpleDepthShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->buildingPosition);
        model = glm::scale(model, glm::vec3(programState->buildingScale));
        simpleDepthShader.setMat4("model", model);
        modelBuilding.Draw(simpleDepthShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->mahoragaPosition);
        model = glm::rotate(model, glm::radians(180.0f),glm::vec3(0.0f,1.0f,0.0f));
        model = glm::scale(model, glm::vec3(programState->mahoragaScale));
        simpleDepthShader.setMat4("model", model);
        modelMahoraga.Draw(simpleDepthShader);
        model = glm::mat4(1.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // don't forget to enable shader before setting uniforms
        ourShader.use();
        ourShader.setVec3("dirlight.direction", lightPos);
        ourShader.setVec3("dirlight.ambient", glm::vec3(0.1f));
        ourShader.setVec3("dirlight.diffuse", glm::vec3(0.3f));
        ourShader.setVec3("dirlight.specular", glm::vec3(1.0f));

        ourShader.setVec3("spotlight1.position", spotlight1.position);
        ourShader.setVec3("spotlight1.direction", spotlight1.direction);
        ourShader.setVec3("spotlight1.ambient", spotlight1.ambient);
        ourShader.setVec3("spotlight1.diffuse", spotlight1.diffuse);
        ourShader.setVec3("spotlight1.specular", spotlight1.specular);
        ourShader.setFloat("spotlight1.constant", spotlight1.constant);
        ourShader.setFloat("spotlight1.linear", spotlight1.linear);
        ourShader.setFloat("spotlight1.quadratic", spotlight1.quadratic);
        ourShader.setFloat("spotlight1.cutoff", glm::cos(glm::radians(spotlight1.cutoff)));
        ourShader.setFloat("spotlight1.outer_cutoff", glm::cos(glm::radians(spotlight1.outer_cutoff)));

        ourShader.setVec3("spotlight2.position", spotlight2.position);
        ourShader.setVec3("spotlight2.direction", spotlight2.direction);
        ourShader.setVec3("spotlight2.ambient", spotlight2.ambient);
        ourShader.setVec3("spotlight2.diffuse", spotlight2.diffuse);
        ourShader.setVec3("spotlight2.specular", spotlight2.specular);
        ourShader.setFloat("spotlight2.constant", spotlight2.constant);
        ourShader.setFloat("spotlight2.linear", spotlight2.linear);
        ourShader.setFloat("spotlight2.quadratic", spotlight2.quadratic);
        ourShader.setFloat("spotlight2.cutoff", glm::cos(glm::radians(spotlight2.cutoff)));
        ourShader.setFloat("spotlight2.outer_cutoff", glm::cos(glm::radians(spotlight2.outer_cutoff)));

        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);
        ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);


        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->shrinePosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->shrineScale));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->patosPosition);
        model = glm::scale(model, glm::vec3(programState->patosScale));
        ourShader.setMat4("model", model);
//        glCullFace(GL_FRONT);
        modelPatos.Draw(ourShader);
//        glCullFace(GL_BACK);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->sukunaPosition);
        model = glm::scale(model, glm::vec3(programState->sukunaScale));
        ourShader.setMat4("model", model);
        modelSukuna.Draw(ourShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->pillarPosition);
        model = glm::scale(model, glm::vec3(programState->pillarScale));
        ourShader.setMat4("model", model);
        modelPillar.Draw(ourShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->lumberPosition);
        model = glm::scale(model, glm::vec3(programState->lumberScale));
        ourShader.setMat4("model", model);
        modelLumber.Draw(ourShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->towerPosition);
        model = glm::scale(model, glm::vec3(programState->towerScale));
        ourShader.setMat4("model", model);
        modelTower.Draw(ourShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->buildingPosition);
        model = glm::scale(model, glm::vec3(programState->buildingScale));
        ourShader.setMat4("model", model);
        modelBuilding.Draw(ourShader);
        model = glm::mat4(1.0f);
        model = glm::translate(model, programState->mahoragaPosition);
        model = glm::rotate(model, glm::radians(180.0f),glm::vec3(0.0f,1.0f,0.0f));
        model = glm::scale(model, glm::vec3(programState->mahoragaScale));
        ourShader.setMat4("model", model);
        modelMahoraga.Draw(ourShader);
        model = glm::mat4(1.0f);

        // drawing skybox
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        {
            skyboxShader.use();
            skyboxShader.setMat4("projection", projection);
            skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(UPWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(DOWNWARD, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::DragFloat("Far plane", &programState->far_plane, 0.05, 0.1, 200.0);
        ImGui::DragFloat("Near plane", &programState->near_plane, 0.05, 0.1, 200.0);
        ImGui::DragFloat("LightPosScale", &programState->lightPosScale, 0.01, 0.01, 1.0);
        ImGui::DragFloat3("Shrine position", (float*)&programState->shrinePosition);
        ImGui::DragFloat("Shrine scale", &programState->shrineScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("Cutoff", &programState->spotlight1.cutoff, 0.05, 1.0, 20.0);
        ImGui::DragFloat("Outer Cutoff", &programState->spotlight1.outer_cutoff, 0.05, 1.0, 20.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadCubemap(vector<std::string>& faces){

    unsigned int skyboxID;
    glGenTextures(1, &skyboxID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID);

    int width, height, nrChannels;
    unsigned char *data;
    for(unsigned int i = 0; i < faces.size(); i++) {

        data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if(data) {
            glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }else{
            std::cerr << "Failed to load cubemap face at path: " << faces[i] << '\n';
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return skyboxID;
}