#include <iostream>
#include <stdio.h>
#include <sstream>

// #include "SDL.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"
#include <glm/gtc/type_ptr.hpp>

using namespace std;
OpenGLWindow::state mode;

const char* glGetErrorString(GLenum error)
{
    switch(error)
    {
    case GL_NO_ERROR:
        return "GL_NO_ERROR";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "UNRECOGNIZED";
    }
}

void glPrintError(const char* label="Unlabelled Error Checkpoint", bool alwaysPrint=false)
{
    GLenum error = glGetError();
    if(alwaysPrint || (error != GL_NO_ERROR))
    {
        printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
    }
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
    FILE* shaderFile = fopen(shaderFilename, "r");
    if(!shaderFile)
    {
        return 0;
    }

    fseek(shaderFile, 0, SEEK_END);
    long shaderSize = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    char* shaderText = new char[shaderSize+1];
    size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
    shaderText[readCount] = '\0';
    fclose(shaderFile);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const char**)&shaderText, NULL);
    glCompileShader(shader);

    delete[] shaderText;

    return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
                       const char* fragShaderFilename)
{
    GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
    GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        cout << "Shader load error: " << message << endl;
        return 0;
    }

    return program;
}

OpenGLWindow::OpenGLWindow()
{
}


void OpenGLWindow::initGL()
{
    mode = view;
    std::string file;
if(1==1){
    std::cout << "Enter the .obj file you wish to load (from the objects folder)" << std::endl;
    std::cin >>file;
    std::cout << "Enter the RGBA color you wish the object to load in (in the format R G B A)" << std::endl;
    std::cin >> r;
    std::cin >> g;
    std::cin >> b;
    std::cin >> a;
    // std::cout << "Enter the camera coordinates you wish the object to load with (in the format X Y Z)" << std::endl;

}else{
    r=0.5f;
    g = 1.0f;
    b = 0.5f;
    a = 0.5f;
    file = "doggo.obj";
}

// We need to first specify what type of OpenGL context we need before we can create the window
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

sdlWin = SDL_CreateWindow(file.c_str(), //"MLLJET002 OpenGL Prac 1: ",
                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                          640, 480, SDL_WINDOW_OPENGL);
if (!sdlWin)
{
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if(glewInitResult != GLEW_OK)
    {
        const GLubyte* errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0,0,0,1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    int colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, r, g, b );//1.0f, 1.0f, 1.0f);

    // Get a handle for our "MVP" uniform
    OpenGLWindow::TransformationID = glGetUniformLocation(shader, "MVP");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    OpenGLWindow::Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

    // Camera matrix
    OpenGLWindow::View = glm::lookAt(
        // glm::vec3(4,3,3),
        glm::vec3(0, 0, 3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    glm::mat4 model_matrix = glm::mat4(1.0f);//glm::translate(glm::mat4(), glm::vec3(1.0f, 0.0f, 0.0f));//glm::make_mat4(model_arr);
    OpenGLWindow::Model = model_matrix;//0.5f,0.5f,0.5f,1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    OpenGLWindow::MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

    // Load the model that we want to use and buffer the vertex attributes
    // GeometryData geometry = loadOBJFile("tri.obj");
    GeometryData geometry;
    geometry.loadFromOBJFile("../objects/" +file); //"/home/minad/Documents/CSC3020H/Assignments/opengl-prac1/src/doggo.obj");
    vertex_count = geometry.vertexCount();
    int vertexLoc = glGetAttribLocation(shader, "position");
    float vertices[9] = { 0.0f,  0.5f, 0.0f,
                         -0.5f, -0.5f, 0.0f,
                          0.5f, -0.5f, 0.0f };
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    // glBufferData(GL_ARRAY_BUFFER, 9*sizeof(float), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, geometry.vertexCount() *3*sizeof(float), geometry.vertexData(), GL_STATIC_DRAW);

    glUseProgram(shader);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao);
    glVertexAttribPointer(
        0,        // attribute. No particular reason for 0, but must match the layout in the shader.
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );
    // glUniformMatrix4fv(TransformationID, 1, GL_TRUE, &MVP[0][0]);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);
    SDL_WarpMouseInWindow(sdlWin, 320, 240);
    int x, y;
    SDL_GetGlobalMouseState(&x, &y);
    // if(x==320&&y==240)
    x_i = x;
    y_i = y;
    z_i = 0;
    // std::cout << "X:" << (x) << "Y:" << (x);
    glPrintError("Setup complete", true);
}

void OpenGLWindow::initGL2()
{
    mode = view;
    std::string file;
    std::string file_2;

    if (1 == 0)
    {
        std::cout << "Enter the first .obj file you wish to load (from the objects folder)" << std::endl;
        std::cin >> file;
        std::cout << "Enter the second .obj file you wish to load (from the objects folder)" << std::endl;
        std::cin >> file;
        std::cout << "Enter the RGBA color you wish the objects to load in (in the format R G B A)" << std::endl;
        std::cin >> r;
        std::cin >> g;
        std::cin >> b;
        std::cin >> a;
        // std::cout << "Enter the camera coordinates you wish the object to load with (in the format X Y Z)" << std::endl;
    }
    else
    {
        r = 0.5f;
        g = 1.0f;
        b = 0.5f;
        a = 0.5f;
        file = "doggo.obj";
        file_2="doggo.obj";
    }

    // We need to first specify what type of OpenGL context we need before we can create the window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sdlWin = SDL_CreateWindow(file.c_str(), //"MLLJET002 OpenGL Prac 1: ",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    if (!sdlWin)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
    }
    SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
    SDL_GL_MakeCurrent(sdlWin, glc);
    SDL_GL_SetSwapInterval(1);

    glewExperimental = true;
    GLenum glewInitResult = glewInit();
    glGetError(); // Consume the error erroneously set by glewInit()
    if (glewInitResult != GLEW_OK)
    {
        const GLubyte *errorString = glewGetErrorString(glewInitResult);
        cout << "Unable to initialize glew: " << errorString;
    }

    int glMajorVersion;
    int glMinorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
    cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
    cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
    cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
    cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
    cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0, 0, 0, 1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // glGenVertexArrays(1, &vao_2);
    // glBindVertexArray(vao_2);

    // Note that this path is relative to your working directory
    // when running the program (IE if you run from within build
    // then you need to place these files in build as well)
    shader = loadShaderProgram("simple.vert", "simple.frag");
    glUseProgram(shader);

    int colorLoc = glGetUniformLocation(shader, "objectColor");
    glUniform3f(colorLoc, r, g, b); //1.0f, 1.0f, 1.0f);

    // Get a handle for our "MVP" uniform
    OpenGLWindow::TransformationID = glGetUniformLocation(shader, "MVP");

    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    OpenGLWindow::Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    // glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

    // Camera matrix
    OpenGLWindow::View = glm::lookAt(
        // glm::vec3(4,3,3),
        glm::vec3(0, 0, 3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    glm::mat4 model_matrix = glm::mat4(1.0f); //glm::translate(glm::mat4(), glm::vec3(1.0f, 0.0f, 0.0f));//glm::make_mat4(model_arr);
    OpenGLWindow::Model = model_matrix;       //0.5f,0.5f,0.5f,1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    OpenGLWindow::MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around

    // Load the model that we want to use and buffer the vertex attributes
    // GeometryData geometry = loadOBJFile("tri.obj");
    GeometryData geometry;
    GeometryData geometry_2;
    geometry.loadFromOBJFile("../objects/" + file); //"/home/minad/Documents/CSC3020H/Assignments/opengl-prac1/src/doggo.obj");
    geometry_2.loadFromOBJFile("../objects/" + file_2); //"/home/minad/Documents/CSC3020H/Assignments/opengl-prac1/src/doggo.obj");
    vertex_count = geometry.vertexCount();
    int vertexLoc = glGetAttribLocation(shader, "position");
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, geometry_2.vertexCount() * 3 * sizeof(float) +geometry.vertexCount() * 3 * sizeof(float), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER_BINDING, 0, geometry_2.vertexCount() * 3 * sizeof(float),geometry_2.vertexData());
    glBufferSubData(GL_ARRAY_BUFFER_BINDING, geometry_2.vertexCount() * 3 * sizeof(float), geometry.vertexCount() * 3 * sizeof(float), geometry.vertexData());

    glUseProgram(shader);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao);
    glVertexAttribPointer(
        0,        // attribute. No particular reason for 0, but must match the layout in the shader.
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );
    // glUniformMatrix4fv(TransformationID, 1, GL_TRUE, &MVP[0][0]);
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(vertexLoc);
    SDL_WarpMouseInWindow(sdlWin, 320, 240);
    int x, y;
    SDL_GetGlobalMouseState(&x, &y);
    // if(x==320&&y==240)
    x_i = x;
    y_i = y;
    z_i = 0;
    // std::cout << "X:" << (x) << "Y:" << (x);
    glPrintError("Setup complete", true);
}

void OpenGLWindow::render()
{
    // Compute the MVP matrix from keyboard and mouse input
    // computeMatricesFromInputs();
    // glm::mat4 ProjectionMatrix = getProjectionMatrix();
    // glm::mat4 ViewMatrix = getViewMatrix();
    // glm::mat4 ModelMatrix = glm::mat4(1.0);
    // glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    glUniformMatrix4fv(TransformationID, 1, GL_FALSE, &MVP[0][0]);

    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // in the "MVP" uniform
    glDrawArrays(GL_POINTS, 0, vertex_count);
    glDrawBuffer(GL_ARRAY_BUFFER);

    // Swap the front and back buffers on the window, effectively putting what we just "drew"
    // onto the screen (whereas previously it only existed in memory)
    SDL_GL_SwapWindow(sdlWin);

    // SDL_SetRelativeMouseMode(SDL_TRUE);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
    // A list of keycode constants is available here: https://wiki.libsdl.org/SDL_Keycode
    // Note that SDL provides both Scancodes (which correspond to physical positions on the keyboard)
    // and Keycodes (which correspond to symbols on the keyboard, and might differ across layouts)
    if(e.type == SDL_KEYDOWN)
    {
        if(e.key.keysym.sym == SDLK_ESCAPE)
        {
            return false;
        }
        if(e.key.keysym.sym == SDLK_c){
            OpenGLWindow::changeColor();
            int colorLoc = glGetUniformLocation(shader, "objectColor");
            glUniform3f(colorLoc, r, g, b); //1.0f, 1.0f, 1.0f);
        }
        if(e.key.keysym.sym == SDLK_t){
            mode = translate;
            SDL_WarpMouseInWindow(sdlWin, 320, 240);
            // OpenGLWindow::state = 1;
            // glm::mat4 model_matrix =glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f)); //glm::mat4(1.0f);
            // OpenGLWindow::MVP *= model_matrix; //0.5f,0.5f,0.5f,1.0f);
        }
        if(e.key.keysym.sym == SDLK_s){
            mode = scale;
            SDL_WarpMouseInWindow(sdlWin, 320, 240);
        }
        if (e.key.keysym.sym == SDLK_r)
        {
            if(mode == rotate_x)mode=rotate_y;
            else if(mode == rotate_y)mode=rotate_x;
            else mode = rotate_x;
            
            SDL_WarpMouseInWindow(sdlWin, 320, 240);
        }
        if (e.key.keysym.sym == SDLK_UP)
        {
            OpenGLWindow::translation(0.0f,0.5f,mode);
        }
        if (e.key.keysym.sym == SDLK_DOWN)
        {
            OpenGLWindow::translation(0.0f, -0.5f, mode);
                }
        if (e.key.keysym.sym == SDLK_LEFT)
        {
            OpenGLWindow::translation(-0.5f, 0.0f, mode);
                }
        if (e.key.keysym.sym == SDLK_RIGHT)
        {
            OpenGLWindow::translation(0.5f, 0.0f, mode);
                }
        if (e.key.keysym.sym == SDLK_2){

            
        }
        
    }
    if (e.type == SDL_MOUSEMOTION)
    {
        // int x_i, y_i;
        // SDL_GetGlobalMouseState(&x_i, &y_i);
        // if(OpenGLWindow::state==1){
        if(mode!=view){
        int x_f, y_f;
        SDL_GetGlobalMouseState(&x_f, &y_f);
        OpenGLWindow::changeColor();
        int colorLoc = glGetUniformLocation(shader, "objectColor");
        glUniform3f(colorLoc, r, g, b); //1.0f, 1.0f, 1.0f);
        OpenGLWindow::translation(x_f,y_f,mode);
        
    }
    }

    return true;

    // glm::mat4 myMatrix;
    // glm::vec4 myVector;
    // fill myMatrix and myVector somehow
    // glm::vec4 transformedVector = myMatrix * myVector; // Again, in this order ! this is important.
}

// void OpenGLWindow::trans(int x_f,int y_f){
//     float diff_x, diff_y, diff_z;
//     diff_x = 0.0f;
//     if (x_f > x_i)
//         diff_x = 0.01f;
//     if (x_f < x_i)
//         diff_x = -0.01f;
//     if (y_f > y_i)
//         diff_y = -0.01f;
//     if (y_f < y_i)
//         diff_y = 0.01f;
//     x_i = x_f;
//     y_i = y_f;
//     glm::mat4 model_matrix;

//     switch (mode){
//     case translate:
//         model_matrix = glm::translate(glm::mat4(), glm::vec3(diff_x, diff_y, diff_z)); //glm::mat4(1.0f);
//         break;
//     case rotate_x:
//         model_matrix = glm::rotate(glm::mat4(), diff_x, glm::vec3(0, 1, 0)); //glm::mat4(1.0f);
//         break;
//     case rotate_y:
//         model_matrix = glm::rotate(glm::mat4(), diff_y, glm::vec3(1, 0, 0)); //glm::mat4(1.0f);
//         break;
//     case rotate_z:
//         // glm::mat4 model_matrix = glm::rotate(glm::mat4(), diff_y*0.5+diff_x*0.5, glm::vec3(1, 0, 0)); //glm::mat4(1.0f);
//         break;
//     case scale:
//         model_matrix = glm::scale(glm::mat4(), glm::vec3(1 + diff_x, 1 + diff_y, 1 + diff_z)); //glm::mat4(1.0f);
//         break;
//     }
//     OpenGLWindow::MVP *= model_matrix;
// }


void OpenGLWindow::cleanup()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    SDL_DestroyWindow(sdlWin);
}
