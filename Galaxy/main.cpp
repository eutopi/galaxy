//
//  main.cpp
//  Galaxy
//
//  Created by Tongyu Zhou on 4/23/19.
//  Copyright © 2019 AIT. All rights reserved.
//

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#if defined(__APPLE__)
#include <GLUT/GLUT.h>
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif
#include <GL/glew.h>        // must be downloaded
#include <GL/freeglut.h>    // must be downloaded unless you have an Apple
#endif

#include <string>

const unsigned int windowWidth = 512, windowHeight = 512;

// OpenGL major and minor versions
int majorVersion = 3, minorVersion = 0;

// row-major matrix 4x4
struct mat4
{
    float m[4][4];
public:
    mat4() {}
    mat4(float m00, float m01, float m02, float m03,
         float m10, float m11, float m12, float m13,
         float m20, float m21, float m22, float m23,
         float m30, float m31, float m32, float m33)
    {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }
    
    mat4 operator*(const mat4& right)
    {
        mat4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; k++) result.m[i][j] += m[i][k] * right.m[k][j];
            }
        }
        return result;
    }
    operator float*() { return &m[0][0]; }
};


// 3D point in homogeneous coordinates
struct vec4
{
    float v[4];
    
    vec4(float x = 0, float y = 0, float z = 0, float w = 1)
    {
        v[0] = x; v[1] = y; v[2] = z; v[3] = w;
    }
    
    vec4 operator*(const mat4& mat)
    {
        vec4 result;
        for (int j = 0; j < 4; j++)
        {
            result.v[j] = 0;
            for (int i = 0; i < 4; i++) result.v[j] += v[i] * mat.m[i][j];
        }
        return result;
    }
    
    vec4 operator+(const vec4& vec)
    {
        vec4 result(v[0] + vec.v[0], v[1] + vec.v[1], v[2] + vec.v[2], v[3] + vec.v[3]);
        return result;
    }
};

// 2D point in Cartesian coordinates
struct vec2
{
    float x, y;
    
    vec2(float x = 0.0, float y = 0.0) : x(x), y(y) {}
    
    vec2 operator+(const vec2& v)
    {
        return vec2(x + v.x, y + v.y);
    }
    
    vec2 operator-(const vec2& v)
    {
        return vec2(x - v.x, y - v.y);
    }
    
    vec2 operator*(float s)
    {
        return vec2(x * s, y * s);
    }
    
    float length() { return sqrt(x * x + y * y); }
};


class Shader
{
protected:
    
    unsigned int shaderProgram;
    
    void getErrorInfo(unsigned int handle)
    {
        int logLen;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0)
        {
            char * log = new char[logLen];
            int written;
            glGetShaderInfoLog(handle, logLen, &written, log);
            printf("Shader log:\n%s", log);
            delete log;
        }
    }
    
    // check if shader could be compiled
    void checkShader(unsigned int shader, char * message)
    {
        int OK;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &OK);
        if (!OK)
        {
            printf("%s!\n", message);
            getErrorInfo(shader);
        }
    }
    
    // check if shader could be linked
    void checkLinking(unsigned int program)
    {
        int OK;
        glGetProgramiv(program, GL_LINK_STATUS, &OK);
        if (!OK)
        {
            printf("Failed to link shader program!\n");
            getErrorInfo(program);
        }
    }
    
public:
    Shader() {
        shaderProgram = 0;
    }
    
    void CompileShader(const char *vertexSource, const char *fragmentSource)
    {
        // create vertex shader from string
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) { printf("Error in vertex shader creation\n"); exit(1); }
        
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        checkShader(vertexShader, "Vertex shader error");
        
        // create fragment shader from string
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) { printf("Error in fragment shader creation\n"); exit(1); }
        
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        checkShader(fragmentShader, "Fragment shader error");
        
        // attach shaders to a single program
        shaderProgram = glCreateProgram();
        if (!shaderProgram) { printf("Error in shader program creation\n"); exit(1); }
        
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        
    }
    
    void LinkShader()
    {
        // program packaging
        glLinkProgram(shaderProgram);
        checkLinking(shaderProgram);
        
    }
    
    //deconstructor
    ~Shader() {
        glDeleteProgram(shaderProgram);
    }
    
    void Run()
    {
        // make this program run
        glUseProgram(shaderProgram);
    }
    
    virtual void UploadColor(vec4 color) {}
    virtual void UploadStripeColor(vec4 color) {}
    virtual void UploadStripeWidth(vec4 color) {}
    virtual void UploadM(mat4 M) {}
    virtual void UploadSamplerID() {}
    virtual void UploadSubTextureID(int i) {}
    virtual void UploadTime(float time) {}
    virtual void UploadDimension(int dim) {}
    
};

class TexturedShader : public Shader
{
public:
    TexturedShader()
    {
        
        const char *vertexSource = R"(
#version 410
        precision highp float;
        
        in vec2 vertexPosition;
        in vec2 vertexTexCoord;
        uniform mat4 M;
        out vec2 texCoord;
        
        void main()
        {
            texCoord = vertexTexCoord;
            gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, 1) * M;
        }
        )";
        
        // fragment shader in GLSL
        const char *fragmentSource = R"(
#version 410
        precision highp float;
        
        uniform sampler2D samplerUnit;
        in vec2 texCoord;
        out vec4 fragmentColor;
        
        void main()
        {
            fragmentColor = texture(samplerUnit, texCoord);
        }
        )";
        
        CompileShader(vertexSource, fragmentSource);
        
        // connect Attrib Array to input variables of the vertex shader
        glBindAttribLocation(shaderProgram, 0, "vertexPosition"); // vertexPosition gets values from Attrib Array 0
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        
        // connect the fragmentColor to the frame buffer memory
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor"); // fragmentColor goes to the frame buffer memory
        
        LinkShader();
        
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    
    void UploadColor(vec4 color) {
        int location = glGetUniformLocation(shaderProgram, "vertexColor");
        if (location >= 0) glUniform3fv(location, 1, &color.v[0]); // set uniform variable vertexColor
        else printf("uniform vertex color cannot be set\n");
    }
    
    void UploadM(mat4 M) {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M for textures cannot be set\n");
    }
    
};

class AnimatedTexturedShader : public Shader
{
public:
    AnimatedTexturedShader()
    {
        
        const char *vertexSource = R"(
#version 410
        precision highp float;
        
        in vec2 vertexPosition;
        in vec2 vertexTexCoord;
        uniform mat4 M;
        out vec2 texCoord;
        
        void main()
        {
            texCoord = vertexTexCoord;
            gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, 1) * M;
        }
        )";
        
        // fragment shader in GLSL
        const char *fragmentSource = R"(
#version 410
        precision highp float;
        
        uniform sampler2D samplerUnit;
        uniform int subTextureID;
        uniform int dim;
        in vec2 texCoord;
        out vec4 fragmentColor;
        
        void main()
        {
            int i = subTextureID % dim;
            int j = subTextureID / dim;
            fragmentColor = texture(samplerUnit, (vec2(i, j) + texCoord) / dim);
        }
        )";
        
        CompileShader(vertexSource, fragmentSource);
        
        // connect Attrib Array to input variables of the vertex shader
        glBindAttribLocation(shaderProgram, 0, "vertexPosition"); // vertexPosition gets values from Attrib Array 0
        glBindAttribLocation(shaderProgram, 1, "vertexTexCoord");
        
        // connect the fragmentColor to the frame buffer memory
        glBindFragDataLocation(shaderProgram, 0, "fragmentColor"); // fragmentColor goes to the frame buffer memory
        
        LinkShader();
        
    }
    
    void UploadSamplerID()
    {
        int samplerUnit = 0;
        int location = glGetUniformLocation(shaderProgram, "samplerUnit");
        glUniform1i(location, samplerUnit);
        glActiveTexture(GL_TEXTURE0 + samplerUnit);
    }
    
    void UploadColor(vec4 color) {
        int location = glGetUniformLocation(shaderProgram, "vertexColor");
        if (location >= 0) glUniform3fv(location, 1, &color.v[0]); // set uniform variable vertexColor
        else printf("uniform vertex color cannot be set\n");
    }
    
    void UploadM(mat4 M) {
        int location = glGetUniformLocation(shaderProgram, "M");
        if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, M);
        else printf("uniform M for textures cannot be set\n");
    }
    
    void UploadSubTextureID(int i) {
        int location = glGetUniformLocation(shaderProgram, "subTextureID");
        if (location >= 0) glUniform1i(location, i);
        else printf("sub texture id cannot be set\n");
    }
    
    void UploadTime(float time) {
        int i = (int)floor(time * 10) % 36;
        UploadSubTextureID(i);
    }
    
    void UploadDimension(int dim) {
        int location = glGetUniformLocation(shaderProgram, "dim");
        if (location >= 0) glUniform1i(location, dim);
        else printf("dimension cannot be set\n");
    }
    
};


extern "C" unsigned char* stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);

class Texture {
    unsigned int textureId;
public:
    Texture(const std::string& inputFileName){
        unsigned char* data;
        int width; int height; int nComponents = 4;
        
        data = stbi_load(inputFileName.c_str(), &width, &height, &nComponents, 0);
        
        if(data == NULL) {
            printf("NULL :(");
            return;
            
        }
        
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        delete data;
    }
    
    void Bind()
    {
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
};


class Material {
    
    Shader* shader;
    
public:
    Material(Shader* shader) : shader(shader) {}
    
    virtual void UploadAttributes() {}
};

class TextureMaterial : public Material {
    
    TexturedShader* shader;
    Texture* texture;
    vec4 color;
    
public:
    TextureMaterial(TexturedShader* shader, vec4 color, Texture* texture) :
    Material(shader), shader(shader), color(color), texture(texture){}
    
    void UploadAttributes() {
        if(texture)
        {
            shader->UploadSamplerID();
            texture->Bind();
        }
        else
        shader->UploadColor(color);
        
    }
};

class AnimatedTexturedMaterial : public Material {
    
    AnimatedTexturedShader* shader;
    Texture* texture;
    vec4 color;
    int dim;
    
public:
    AnimatedTexturedMaterial(AnimatedTexturedShader* shader, vec4 color, Texture* texture, int dim) :
    Material(shader), shader(shader), color(color), texture(texture), dim(dim){}
    
    void UploadAttributes() {
        if(texture)
        {
            shader->UploadSamplerID();
            texture->Bind();
            shader->UploadDimension(dim);
        }
        else
        shader->UploadColor(color);
        
    }
};

class Geometry{
    
protected: unsigned int vao;    // vertex array object id
    
public:
    Geometry(){
        glGenVertexArrays(1, &vao);    // create a vertex array object
    }
    
    virtual void Draw() = 0;
};

class Triangle : public Geometry
{
    unsigned int vbo;        // vertex buffer object
    
public:
    Triangle()
    {
        glBindVertexArray(vao);        // make it active
        
        glGenBuffers(1, &vbo);        // generate a vertex buffer object
        
        // vertex coordinates: vbo -> Attrib Array 0 -> vertexPosition of the vertex shader
        glBindBuffer(GL_ARRAY_BUFFER, vbo); // make it active, it is an array
        static float vertexCoords[] = { 0, 0, 1, 0, 0, 1 };    // vertex data on the CPU
        
        glBufferData(GL_ARRAY_BUFFER,    // copy to the GPU
                     sizeof(vertexCoords),    // size of the vbo in bytes
                     vertexCoords,        // address of the data array on the CPU
                     GL_STATIC_DRAW);    // copy to that part of the memory which is not modified
        
        // map Attribute Array 0 to the currently bound vertex buffer (vbo)
        glEnableVertexAttribArray(0);
        
        // data organization of Attribute Array 0
        glVertexAttribPointer(0,    // Attribute Array 0
                              2, GL_FLOAT,        // components/attribute, component type
                              GL_FALSE,        // not in fixed point format, do not normalized
                              0, NULL);        // stride and offset: it is tightly packed
    }
    
    void Draw()
    {
        glBindVertexArray(vao);    // make the vao and its vbos active playing the role of the data source
        glDrawArrays(GL_TRIANGLES, 0, 3); // draw a single triangle with vertices defined in vao
    }
};

class Quad : public Geometry
{
    unsigned int vbo;
    
public:
    Quad()
    {
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        static float vertexCoords[] = { -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5};
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }
    
    void Draw()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
};

class TexturedQuad : public Quad
{
    unsigned int vboTex;
    
public:
    TexturedQuad()
    {
        glBindVertexArray(vao);
        glGenBuffers(1, &vboTex);
        
        glBindBuffer(GL_ARRAY_BUFFER, vboTex);
        static float texCoords[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        
        // define the texture coordinates here
        // assign the array of texture coordinates to
        // vertex attribute array 1
    }
    
    void Draw()
    {
        glEnable(GL_BLEND); // necessary for transparent pixels
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisable(GL_BLEND);
    }
};

class Mesh{
    
    Geometry *geometry;
    Material *material;
    
public:
    Mesh(Geometry *geometry,
         Material *material) : geometry(geometry), material(material) {}
    
    void Draw() {
        material->UploadAttributes();
        geometry->Draw();
    }
};

bool keyboardState[256] = {false};
bool blackHolePlaced = false;
vec2 blackHolePos = vec2(0, 0.4);

class Camera {
    
    vec2 center;
    vec2 old_center;
    float horizontal_size;
    float vertical_size;
public:
    Camera(vec2 center, float horizontal_size, float vertical_size) {
        this->center = center;
        this->old_center = center;
        this->horizontal_size = horizontal_size;
        this->vertical_size = vertical_size;
    }
    
    mat4 GetViewTransformationMatrix() {
        mat4 M = {1/horizontal_size,0,0,0,
            0,1/vertical_size,0,0,
            0,0,1,0,
            -center.x,-center.y,0,1};
        return M;
    }
    
    void Move(double dt, double t) {
        
        // Quake
        if (keyboardState['q']) {
            center.x = center.x+sinf(100*t)/400;
            center.y = center.y+sinf(100*t)/400;
        }
        else {
            center = old_center;
        }
        
        if (keyboardState['i']) {
            center.y = center.y + dt;
        }
        if (keyboardState['k']) {
            center.y = center.y - dt;
        }
        if (keyboardState['l']) {
            center.x = center.x + dt;
        }
        if (keyboardState['j']) {
            center.x = center.x - dt;
        }
        glutPostRedisplay();
    }
};

Camera camera(vec2(0,0),1.5,1.5);

class Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    
public:
    Object(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation) :
    shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation) {}
    
    virtual void UploadAttributes() {}
    
    void Draw() {
        UploadAttributes();
        mesh->Draw();
    }
    
    virtual void SetTime(float time) {}
    virtual void Move(float dt, float time_lapsed) {}
    virtual Shader* GetShader() {return shader;}
    virtual vec2 GetLocation() {return position;}
    virtual bool ShouldBeDeleted() {return false;}
    virtual void Control(std::vector<Object*> objects, int me,
                         std::vector<std::vector<Object*>> asteroid_objects) {}
    virtual void HitByProjectile(Object* projectile) {}
    virtual void TargetHit() {}
    virtual bool IsEnemy() {return false;}
    virtual bool DoneExploding(float time) {return false;}
    virtual void SetDramatic() {}
    virtual void DramaticExit() {}
    virtual bool IsBlackHole() {return false;}
};

class AvatarObject : public Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    float velocity = 0.3;
    float acceleration;
    float invMass = 0.3;
    float lastTime = 0;
    float force = 1;
    bool aPressed, dPressed, wPressed, sPressed;
    
public:
    AvatarObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation) {
        aPressed = false;
        dPressed = false;
        wPressed = false;
        sPressed = false;
    }
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    void SetTime(float time) {
        shader->Run();
        shader->UploadTime(time);
    }
    
    void Move(float dt, float time_lapsed) {
        if (keyboardState['a'] || keyboardState['d'] || keyboardState['w'] || keyboardState['s']) {
            force = force + 2*dt;
            acceleration = force*invMass;
            velocity = velocity + acceleration * dt; //new velocity
            if (keyboardState['a']) {
                position.x = position.x - (velocity * dt);
                aPressed = true;
            }
            if (keyboardState['d']) {
                position.x = position.x + (velocity * dt);
                dPressed = true;
            }
            if (keyboardState['w']) {
                position.y = position.y + (velocity * dt);
                wPressed = true;
            }
            if (keyboardState['s']) {
                position.y = position.y - (velocity * dt);
                sPressed = true;
            }
        }
        else {
            force = force + 4*dt;
            acceleration = force*invMass;
            velocity = velocity - acceleration * dt;
            if (aPressed) {
                position.x = position.x - (velocity * dt);
            }
            if (dPressed) {
                position.x = position.x + (velocity * dt);
            }
            if (wPressed) {
                position.y = position.y + (velocity * dt);
            }
            if (sPressed) {
                position.y = position.y - (velocity * dt);
            }
            if (velocity < 0.4) {
                force = 1;
                velocity = 0.3;
                aPressed = false;
                dPressed = false;
                wPressed = false;
                sPressed = false;
            }
        }
        //float c = -force/velocity; //drag coefficient
        //velocity = velocity * exp(-dt * c * invMass); //drag
        
    }
    
    vec2 GetLocation() {
        return position;
    }
};

class ProjectileObject : public Object {
    Shader *shader;
    Mesh *mesh;
    vec2 init_position, position, scaling;
    float orientation;
    bool deleted = false;
    
public:
    ProjectileObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation) {
        init_position = position;
    }
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    vec2 GetLocation() {return position;}
    
    void Move(float dt, float time_lapsed) {
        position.y = position.y + dt*2;
        if (position.y > init_position.y + 1) {
            deleted = true;
        }
    }
  
    bool ShouldBeDeleted() {
        return deleted;
    }
    
    void Control(std::vector<Object*> objects, int me,
                 std::vector<std::vector<Object*>> asteroid_objects) {
        for(int i = 0; i < objects.size(); i++) {
            if(i != me) {
                objects[i]->HitByProjectile(this);
            }
        }
        for(int i = 0; i < asteroid_objects.size(); i++) {
            for(int j = 0; j < asteroid_objects[i].size(); j++) {
                asteroid_objects[i][j]->HitByProjectile(this);
            }
        }
    }
    
    void TargetHit() {
        deleted = true;
    }
    
};

class FireballObject : public Object {
    Shader *shader;
    Mesh *mesh;
    vec2 init_position, position, scaling;
    float orientation;
    bool deleted = false;
    vec2 norm_path;
    
public:
    FireballObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation, vec2 norm_path) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation), norm_path(norm_path) {
        init_position = position;
    }
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    vec2 GetLocation() {return position;}
    
    void Move(float dt, float time_lapsed) {
        position = position + norm_path*dt*2;
        //printf("%f", dt);
        if (position.y > init_position.y+1.5 || position.y < init_position.y-1.5 ||
            position.x > init_position.x+1.5 || position.x < init_position.x-1.5) {
            deleted = true;
        }
    }
    
    bool ShouldBeDeleted() {
        return deleted;
    }
    
    void Control(std::vector<Object*> objects, int me,
                 std::vector<std::vector<Object*>> asteroid_objects) {
        for(int i = 0; i < objects.size(); i++) {
            if(i != me) {
                objects[i]->HitByProjectile(this);
            }
        }
        for(int i = 0; i < asteroid_objects.size(); i++) {
            for(int j = 0; j < asteroid_objects[i].size(); j++) {
                asteroid_objects[i][j]->HitByProjectile(this);
            }
        }
    }
    
    void TargetHit() {
        deleted = true;
    }
    
};

class EnemyObject : public Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    bool deleted = false;
    bool dramatic = false;
    bool enemy = true;
    float velocity = 0.0001;
    
public:
    EnemyObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation) {}
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    void HitByProjectile(Object* projectile) {
        vec2 dist = position - projectile->GetLocation();
        float radius = 0.2; //change later
        if (dist.length() < radius) {
            deleted = true;
            projectile->TargetHit();
        }
    }
    
    bool ShouldBeDeleted() {
        if (scaling.x < 0.01 || scaling.y < 0.01) {
            deleted = true;
        }
        return deleted;
    }
    
    bool IsEnemy() {return enemy;}
    
    void SetDramatic() {
        dramatic = true;
        enemy = false;
    }
    
    void DramaticExit() {
        if (dramatic) {
            scaling = scaling - vec2(0.0001, 0.0001);
            orientation = orientation + 60;
        }
    }
    
    vec2 GetLocation() {return position;}
    
    void Move(float dt, float time_lapsed) {
        if (blackHolePlaced) {
            vec2 path = blackHolePos - position;
            
            float m1 = 40; //blackhole mass
            float m2 = 0.5; //asteroid mass
            float r = path.length()*100;
            float force = 9.81*((m1*m2)/r*r); //law of gravitation
            
            float acceleration = force*(1/m2);
            velocity = velocity + acceleration * dt; //new velocity
            
            vec2 norm_path = vec2(path.x/path.length(), path.y/path.length());
            position = position + norm_path*(velocity * (dt/1000));
        }
    }
};

class EnemyMovingHeartObject : public Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    bool deleted = false;
    
public:
    EnemyMovingHeartObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation) {}
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    void HitByProjectile(Object* projectile) {
        vec2 dist = position - projectile->GetLocation();
        float radius = 0.2; //change later
        if (dist.length() < radius) {
            deleted = true;
            projectile->TargetHit();
        }
    }
    
    vec2 GetLocation() {return position;}
    
    void SetTime(float time) {
        shader->Run();
        shader->UploadTime(time);
    }
    
    bool ShouldBeDeleted() {
        return deleted;
    }
    
    void Move(float dt, float time_lapsed) {
        float t = time_lapsed/2;
        float scale = 15.0;
        position.x = (16 * pow(sinf(t), 3.0))/scale;
        position.y = (13 * cosf(t) - 5 * cosf(2*t) - 2 * cosf(3*t) - cosf(4*t))/scale;
    }
    
    bool IsEnemy() {return true;}
};

class EnemyMovingEggObject : public Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    bool deleted = false;
    
public:
    EnemyMovingEggObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation) {}
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    void HitByProjectile(Object* projectile) {
        vec2 dist = position - projectile->GetLocation();
        float radius = 0.2; //change later
        if (dist.length() < radius) {
            deleted = true;
            projectile->TargetHit();
        }
    }
    
    vec2 GetLocation() {return position;}
    
    void SetTime(float time) {
        shader->Run();
        shader->UploadTime(time);
    }
    
    bool ShouldBeDeleted() {
        return deleted;
    }
    
    void Move(float dt, float time_lapsed) {
        float t = time_lapsed/2;
        float k = 2;
        float new_position_x = cosf(k*t)*cosf(t);
        float new_position_y = cosf(k*t)*sinf(t);
        
        vec2 direction = vec2(new_position_x - position.x, new_position_y - position.y);
        vec2 norm_direction = vec2(direction.x/direction.length(), direction.y/direction.length());
        // dot products
        if (norm_direction.x < 0) {
            orientation = 180 + acos(norm_direction.y)*(180/M_PI);
        }
        else if (norm_direction.x > 0) {
            orientation = 180 - acos(norm_direction.y)*(180/M_PI);
        }
        
        position.x = new_position_x;
        position.y = new_position_y;
    }
    
    bool IsEnemy() {return true;}
};

class SeekerObject : public Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    bool deleted = false;
    Object* avatar;
    
public:
    SeekerObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation, Object* avatar) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation), avatar(avatar) {}
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    void HitByProjectile(Object* projectile) {
        vec2 dist = position - projectile->GetLocation();
        float radius = 0.2; //change later
        if (dist.length() < radius) {
            deleted = true;
            projectile->TargetHit();
        }
    }
    
    vec2 GetLocation() {return position;}
    
    void SetTime(float time) {
        shader->Run();
        shader->UploadTime(time);
    }
    
    bool ShouldBeDeleted() {
        return deleted;
    }
    
    void Move(float dt, float time_lapsed) {
        
        vec2 path = avatar->GetLocation() - position;
        if (abs(path.x) > 0.1 || abs(path.y) > 0.1) {
            vec2 norm_path = vec2(path.x/path.length(), path.y/path.length());
            
            // dot products
            if (norm_path.x < 0) {
                orientation = 270 + acos(norm_path.y)*(180/M_PI);
            }
            else if (norm_path.x > 0) {
                orientation = 270 - acos(norm_path.y)*(180/M_PI);
            }
            
            position = position + norm_path*(dt/5)*2;
        }
    }
    
    bool IsEnemy() {return true;}
};

class ExplodingObject : public Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    bool deleted = false;
    float start_time;
    float time_lapsed;
    
public:
    ExplodingObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation, float start_time, float time_lapsed) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation), start_time(start_time), time_lapsed(time_lapsed) {}
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    void SetTime(float time) {
        shader->Run();
        shader->UploadTime(time*1.5);
    }
    
    bool DoneExploding(float time) {
        int start_i = (int)floor(time_lapsed * 10) % 36;
        int i = (int)floor(time * 10) % 36;
        if ((start_i-1)%36 == i || (time - time_lapsed) > 3.4) {
            return true;
        }
        else return false;
    }
};

class BlackHoleObject : public Object{
    Shader *shader;
    Mesh *mesh;
    vec2 position, scaling;
    float orientation;
    
public:
    BlackHoleObject(Shader *shader, Mesh *mesh, vec2 position, vec2 scaling, float orientation) :
    Object(shader, mesh, position, scaling, orientation), shader(shader), mesh(mesh), position(position), scaling(scaling), orientation(orientation) {}
    
    void UploadAttributes() {
        mat4 S = {scaling.x,0,0,0,
            0,scaling.y,0,0,
            0,0,1,0,
            0,0,0,1};
        
        float radians = orientation/180*M_PI;
        mat4 R = {cos(radians),sin(radians),0,0,
            -sin(radians),cos(radians),0,0,
            0,0,1,0,
            0,0,0,1};
        
        mat4 T = {1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            position.x,position.y,0,1};
        
        mat4 V = camera.GetViewTransformationMatrix();
        mat4 M = S * R * T * V; // scaling, rotation, and translation
        shader->UploadM(M);
    }
    
    bool IsBlackHole() {return true;}
};

class Scene {
    TexturedShader* textureShader;
    AnimatedTexturedShader* animatedShader;
    int asteroid_dim = 6;
    
    std::vector<Material*> materials;
    std::vector<Geometry*> geometries;
    std::vector<Mesh*> meshes;
    std::vector<Object*> objects;
    
    std::vector<Material*> asteroid_materials;
    std::vector<Geometry*> asteroid_geometries;
    std::vector<Mesh*> asteroid_meshes;
    std::vector<std::vector<Object*>> asteroid_objects;
public:
    Scene() {
        textureShader = 0;
        animatedShader = 0;
    }
    void Initialize() {
        
        textureShader = new TexturedShader();
        animatedShader = new AnimatedTexturedShader();
        
        //add avatar
        Texture* t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/spaceship.png");
        materials.push_back(new TextureMaterial(textureShader, vec4(1, 0, 0), t));
        geometries.push_back(new TexturedQuad());
        meshes.push_back(new Mesh(geometries[0], materials[0]));
        objects.push_back(new AvatarObject(textureShader, meshes[0], vec2(0, -0.75), vec2(0.8,0.8), 180));
        
        Texture* t1 = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/orb.png");
        materials.push_back(new AnimatedTexturedMaterial(animatedShader, vec4(1, 0, 0), t1, 5));
        geometries.push_back(new TexturedQuad());
        meshes.push_back(new Mesh(geometries[1], materials[1]));
        objects.push_back(new EnemyMovingHeartObject(animatedShader, meshes[1], vec2(-1.2,0.9), vec2(0.2,0.2), 0));
        
        Texture* t2 = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/rocket.png");
        materials.push_back(new TextureMaterial(textureShader, vec4(1, 0, 0), t2));
        geometries.push_back(new TexturedQuad());
        meshes.push_back(new Mesh(geometries[2], materials[2]));
        objects.push_back(new EnemyMovingEggObject(textureShader, meshes[2], vec2(-1.2,0.9), vec2(0.3,0.3), 0));
        
        Texture* t3 = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/fish.png");
        materials.push_back(new TextureMaterial(textureShader, vec4(1, 0, 0), t3));
        geometries.push_back(new TexturedQuad());
        meshes.push_back(new Mesh(geometries[3], materials[3]));
        objects.push_back(new SeekerObject(textureShader, meshes[3], vec2(-1.2,0.9), vec2(0.2,0.2), 270, objects[0]));
        
        //add enemies
        srand(time(0));
        for( int i=0; i < asteroid_dim; i++) {
            asteroid_objects.push_back(std::vector<Object*>());
            for (int j=0; j < asteroid_dim; j++) {
                Texture* t;
                int r = rand() % 4;
                switch (r) {
                    case 0:
                        t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/asteroid.png");
                        break;
                    case 1:
                        t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/asteroid1.png");
                        break;
                    case 2:
                        t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/asteroid2.png");
                        break;
                    case 3:
                        t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/asteroid3.png");
                        break;
                }
                
                float angle = rand() % 360;
                
                asteroid_materials.push_back(new TextureMaterial(textureShader, vec4(1, 0, 0), t));
                asteroid_geometries.push_back(new TexturedQuad());
                asteroid_meshes.push_back(new Mesh(asteroid_geometries[asteroid_dim*i + j], asteroid_materials[asteroid_dim*i + j]));
                asteroid_objects[i].push_back(new EnemyObject(textureShader, asteroid_meshes[asteroid_dim*i + j], vec2(-0.75+(j*0.3), -0.4+(i*0.3)), vec2(0.2,0.2), angle));
            }
        }
        
    }
    ~Scene() {
        for(int i = 0; i < materials.size(); i++) delete materials[i];
        for(int i = 0; i < geometries.size(); i++) delete geometries[i];
        for(int i = 0; i < meshes.size(); i++) delete meshes[i];
        for(int i = 0; i < objects.size(); i++) delete objects[i];
        
        for(int i = 0; i < asteroid_materials.size(); i++) delete asteroid_materials[i];
        for(int i = 0; i < asteroid_geometries.size(); i++) delete asteroid_geometries[i];
        for(int i = 0; i < asteroid_meshes.size(); i++) delete asteroid_meshes[i];
        for(int i = 0; i < asteroid_objects.size(); i++) {
           for(int j = 0; j < asteroid_objects[i].size(); j++) {
               delete asteroid_objects[i][j];
           }
        }
        
        if(textureShader) delete textureShader;
        if(animatedShader) delete animatedShader;
    }
    
    void Draw()
    {
        for(int i = 0; i < asteroid_objects.size(); i++) {
            for(int j = 0; j < asteroid_objects[i].size(); j++) {
                asteroid_objects[i][j]->GetShader()->Run();
                asteroid_objects[i][j]->Draw();
            }
        }
        
        for(int i = 0; i < objects.size(); i++) {
            objects[i]->GetShader()->Run();
            objects[i]->Draw();
        }
    }
    
    void SetTime(float time) {
        for(int i = 0; i < objects.size(); i++) objects[i]->SetTime(time);
    }
    
    void Move(float time, float time_lapsed) {
        for(int i = 0; i < objects.size(); i++) {
            objects[i]->Move(time, time_lapsed);
            objects[i]->Control(objects, i, asteroid_objects);
            if(objects[i]->ShouldBeDeleted()) {
                if(objects[i]->IsEnemy()) {Explode(objects[i]->GetLocation(), time, time_lapsed);}
                materials.erase(materials.begin()+i);
                geometries.erase(geometries.begin()+i);
                meshes.erase(meshes.begin()+i);
                objects.erase(objects.begin()+i);
            }
            if(objects[i]->DoneExploding(time_lapsed)) {
                materials.erase(materials.begin()+i);
                geometries.erase(geometries.begin()+i);
                meshes.erase(meshes.begin()+i);
                objects.erase(objects.begin()+i);
            }
        }
        for(int i = 0; i < asteroid_objects.size(); i++) {
            for(int j = 0; j < asteroid_objects[i].size(); j++) {
                asteroid_objects[i][j]->Move(time, time_lapsed);
                asteroid_objects[i][j]->DramaticExit();
                if(asteroid_objects[i][j]->ShouldBeDeleted()) {
                    if(asteroid_objects[i][j]->IsEnemy()) {Explode(asteroid_objects[i][j]->GetLocation(), time, time_lapsed);}
                    asteroid_objects[i].erase(asteroid_objects[i].begin()+j);
                }
            }
        }
    }
    
    void Explode(vec2 position, float time, float time_lapsed) {
        // explosion thing here
        Texture* t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/boom.png");
        materials.push_back(new AnimatedTexturedMaterial(animatedShader, vec4(1, 0, 0), t, 6));
        geometries.push_back(new TexturedQuad());
        meshes.push_back(new Mesh(geometries[objects.size()], materials[objects.size()]));
        objects.push_back(new ExplodingObject(animatedShader, meshes[objects.size()], position, vec2(0.4,0.4), 0, time, time_lapsed));
    }
    
    void AsteroidDisappear() {
        for (int i = 0; i < asteroid_objects.size(); i++) {
            for (int j = 0; j < asteroid_objects[i].size(); j++) {
                if (rand() % 1000 < 1) {
                    asteroid_objects[i][j]->SetDramatic();
                }
            }
        }
    }
    
    void placeBlackHole() {
        int length = objects.size();
        
        Texture* t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/blackhole.png");
        materials.push_back(new TextureMaterial(textureShader, vec4(1, 0, 0), t));
        geometries.push_back(new TexturedQuad());
        meshes.push_back(new Mesh(geometries[length], materials[length]));
        objects.push_back(new BlackHoleObject(textureShader, meshes[length], blackHolePos, vec2(0.5,0.5), 0));
        
        blackHolePlaced = true;
    }
    
    void removeBlackHole() {
        for (int i = 0; i < objects.size(); i++) {
            if (objects[i]->IsBlackHole()) {
                materials.erase(materials.begin()+i);
                geometries.erase(geometries.begin()+i);
                meshes.erase(meshes.begin()+i);
                objects.erase(objects.begin()+i);
                blackHolePlaced = false;
            }
        }
    }
    
    std::vector<Material*> GetMaterials() {
        return materials;
    }
    
    void AddMaterial(Material* m) {
        materials.push_back(m);
    }
    
    std::vector<Geometry*> GetGeometries() {
        return geometries;
    }
    
    void AddGeometry(Geometry* g) {
        geometries.push_back(g);
    }
    
    std::vector<Mesh*> GetMeshes() {
        return meshes;
    }
    
    void AddMesh(Mesh* m) {
        meshes.push_back(m);
    }
    
    std::vector<Object*> GetObjects() {
        return objects;
    }
    
    void AddObject(Object* o) {
        objects.push_back(o);
    }
    
};

Scene *gScene = 0;
TexturedShader* projectileShader = 0;
TexturedShader* fireballShader = 0;
float lastProjectileTime = 0;
bool mouseDown = false;
float cx, cy;

void shootProjectile() {
    if (lastProjectileTime >= 0) {
        projectileShader = new TexturedShader();
        
        std::vector<Object*> objects = gScene->GetObjects();
        int length = objects.size();
        
        Texture* t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/bullet.png");
        gScene->AddMaterial(new TextureMaterial(projectileShader, vec4(1, 0, 0), t));
        gScene->AddGeometry(new TexturedQuad());
        
        std::vector<Material*> materials = gScene->GetMaterials();
        std::vector<Geometry*> geometries = gScene->GetGeometries();
        gScene->AddMesh(new Mesh(geometries[length], materials[length]));
        
        std::vector<Mesh*> meshes = gScene->GetMeshes();
        vec2 projectile_location = objects[0]->GetLocation() + vec2(0, 0.1);
        gScene->AddObject(new ProjectileObject(projectileShader, meshes[length], projectile_location, vec2(0.4,0.4), 0));
        
        lastProjectileTime = -1; //cooldown time
    }
}

void shootFireball(float x, float y) {
    fireballShader = new TexturedShader();
    
    std::vector<Object*> objects = gScene->GetObjects();
    int length = objects.size();
    
    Texture* t = new Texture("/Users/Tongyu/Documents/AIT_Budapest/Graphics/Galaxy/Galaxy/fireball.png");
    gScene->AddMaterial(new TextureMaterial(fireballShader, vec4(1, 0, 0), t));
    gScene->AddGeometry(new TexturedQuad());
    
    std::vector<Material*> materials = gScene->GetMaterials();
    std::vector<Geometry*> geometries = gScene->GetGeometries();
    gScene->AddMesh(new Mesh(geometries[length], materials[length]));
    
    std::vector<Mesh*> meshes = gScene->GetMeshes();
    vec2 path = vec2(x,y) - objects[0]->GetLocation();
    vec2 norm_path = vec2(path.x/path.length(), path.y/path.length());
    vec2 projectile_location = objects[0]->GetLocation() + norm_path*0.1;
    
    float rotate_angle = 0;
    // dot products
    if (x < 0) {
        rotate_angle = acos(norm_path.y)*(180/M_PI);
    }
    else if (x > 0) {
        rotate_angle = -acos(norm_path.y)*(180/M_PI);
    }
    
    gScene->AddObject(new FireballObject(fireballShader, meshes[length], projectile_location, vec2(0.4,0.4), 60+rotate_angle, norm_path));
}

// initialization, create an OpenGL context
void onInitialization()
{
    glViewport(0, 0, windowWidth, windowHeight);
    
    gScene = new Scene();
    gScene->Initialize();
}

void onExit()
{
    delete gScene;
    delete projectileShader;
    delete fireballShader;
    printf("exit");
}

// window has become invalid: redraw
void onDisplay()
{
    glClearColor(0.07, 0.01, 0.16, 0); // background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen
    
    gScene->Draw();
    
    glutSwapBuffers(); // exchange the two buffers
}

void onMouse(int button, int state, int x, int y) {
    cx = (float)x / (float)windowWidth;
    cy = (float)y / (float)windowHeight;
    
    // convert to coordinates
    cx = (cx-0.5)/0.5;
    cy = -(cy-0.5)/0.5;
    
    mouseDown = (state == GLUT_DOWN);
    
    printf("Clicked on pixel %f, %f\n", cx, cy);
}

void onMouseDrag(int x, int y) {
    cx = (float)x / (float)windowWidth;
    cy = (float)y / (float)windowHeight;
    
    // convert to coordinates
    cx = (cx-0.5)/0.5;
    cy = -(cy-0.5)/0.5;
}

void onKeyboardUp(unsigned char key, int i, int j) {
    if (key == ' ') {
        shootProjectile();
    }
    if (keyboardState['b']) {
        if (!blackHolePlaced) {
            gScene->placeBlackHole();
        }
        else {
            gScene->removeBlackHole();
        }
    }
    keyboardState[key] = false;
}

void onKeyboard(unsigned char key, int i, int j) {
    keyboardState[key] = true;
    //showTriangle = !showTriangle;
}

void onIdle( ) {
    // time elapsed since program started, in seconds
    double t = glutGet(GLUT_ELAPSED_TIME) * 0.001;
    // variable to remember last time idle was called
    static double lastTime = 0.0;
    // time difference between calls: time step
    double dt = abs(t - lastTime);
    // store time
    lastTime = t;
    lastProjectileTime = lastProjectileTime + dt;
    camera.Move(dt, t);
    
    gScene->SetTime(t*2);
    gScene->Move(dt, t*2);
    
    if (mouseDown && !keyboardState['b']) {
        shootFireball(cx, cy);
    }
    if (keyboardState['q']) {
        gScene->AsteroidDisappear();
    }
    
    
    glutPostRedisplay();
}

int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
#if !defined(__APPLE__)
    glutInitContextVersion(majorVersion, minorVersion);
#endif
    glutInitWindowSize(windowWidth, windowHeight);     // application window is initially of resolution 512x512
    glutInitWindowPosition(50, 50);            // relative location of the application window
#if defined(__APPLE__)
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);  // 8 bit R,G,B,A + double buffer + depth buffer
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutCreateWindow("Triangle Rendering");
    
#if !defined(__APPLE__)
    glewExperimental = true;
    glewInit();
#endif
    printf("GL Vendor    : %s\n", glGetString(GL_VENDOR));
    printf("GL Renderer  : %s\n", glGetString(GL_RENDERER));
    printf("GL Version (string)  : %s\n", glGetString(GL_VERSION));
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    printf("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
    printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    onInitialization();
    
    glutDisplayFunc(onDisplay); // register event handlers
    glutMouseFunc(onMouse);
    glutMotionFunc(onMouseDrag);
    glutKeyboardFunc(onKeyboard);
    glutKeyboardUpFunc(onKeyboardUp);
    glutIdleFunc(onIdle);
    
    glutMainLoop();
    onExit();
    return 1;
}
