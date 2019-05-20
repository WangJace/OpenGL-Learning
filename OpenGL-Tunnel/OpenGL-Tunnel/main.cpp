//
//  main.cpp
//  OpenGL-Tunnel
//
//  Created by 王傲云 on 2019/5/16.
//  Copyright © 2019 Jace. All rights reserved.
//

#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLFrame.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager     shaderManager;          // 着色器管理器
GLMatrixStack       modelViewMatrix;        // 模型视图矩阵
GLMatrixStack       projectionMatrix;       // 投影矩阵
GLFrustum           viewFrustum;            // 视景体
GLGeometryTransform transformPipeline;      // 几何变换管线

// 4个批次容器类
GLBatch             floorBatch;             // 地面
GLBatch             ceilingBatch;           // 天花板
GLBatch             leftWallBatch;          // 左墙面
GLBatch             rightWallBatch;         // 右墙面


// 深度初始值
GLfloat             viewZ = -65.0f;

// 纹理标识符
#define TEXTURE_BRICK     0 // 墙面
#define TEXTURE_FLOOR     1 // 地板
#define TEXTURE_CEILING   2 // 纹理天花板
#define TEXTURE_COUNT     3 // 纹理个数
GLuint               textures[TEXTURE_COUNT]; // 纹理标记数组

// tag文件名数组
const char *szTextureFiles[TEXTURE_COUNT] = { "brick.tga", "floor.tga", "ceiling.tga" };

// 菜单栏修改纹理的属性 -- 缩小过滤器
void ProcessMenu(int value) {
    GLint iLoop;
    
    // 遍历3个纹理
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++) {
        /*
         绑定纹理
         glBindTexture (GLenum target, GLuint texture);
         参数1: 使用的纹理类型(GL_TEXTURE_1D/GL_TEXTURE_2D/GL_TEXTURE_3D)
         参数2: 纹理的标记
         */
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);
        
        /*
         配置纹理参数
         glTexParameteri (GLenum target, GLenum pname, GLint param);
         参数1: 使用的纹理类型(GL_TEXTURE_1D/GL_TEXTURE_2D/GL_TEXTURE_3D)
         参数2: 纹理参数（属性）
         参数3: 对应的参数值
         */
        switch (value) {
            case 0:
                // GL_TEXTURE_MIN_FILTER 缩小过滤器-->GL_NEAREST 邻近过滤(可选)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                break;
            case 1:
                // GL_TEXTURE_MIN_FILTER 缩小过滤器-->GL_LINEAR 线性过滤(可选)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                break;
            case 2:
                // 在最邻近Mip层，并执行最邻近过滤
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                break;
            case 3:
                // 在Mip层之间执行线性插补，并执行最邻近过滤
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                break;
            case 4:
                // 选择最邻近Mip层，并执行线性过滤
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                break;
            case 5:
                // 在Mip层之间执行线性插补，并执行线性过滤，又称三线性Mip贴图
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                break;
            case 6:
                // 设置各向异性过滤
                GLfloat fLargest;
                // 设置获取各向异性过滤的最大数量
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
                // 设置纹理参数
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
                break;
            case 7:
                // 设置纹理参数
                // 各向同性过滤,数量为1.0
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
                break;
            default:
                break;
        }
    }
    glutPostRedisplay();
}

// 在这个函数里能够在渲染环境中进行任何需要的初始化，在这里设置并初始化纹理对象
void SetupRC() {
    GLbyte *pBytes;
    int iWidth, iHeight, iComponents;
    GLenum eFormat;
    GLint iLoop;
    
    // 背景
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    shaderManager.InitializeStockShaders();
    
    /*
     1. 加载纹理
     glGenTextures (GLsizei n, GLuint *textures);
     参数1: 纹理数量
     参数2: 纹理对象标识数组
     */
    glGenTextures(TEXTURE_COUNT, textures);
    
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++) {
        /*
         2.绑定纹理
         glBindTexture (GLenum target, GLuint texture);
         参数1: 使用的纹理类型（GL_TEXTURE_1D/GL_TEXTURE_2D/GL_TEXTURE_3D）
         参数2: 纹理的标记
         */
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);
        /*
         3.加载tga文件
         GLbyte *gltReadTGABits(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat, GLbyte *pData = NULL);
         参数1: 纹理文件的名称
         参数2: 获取纹理文件的宽度
         参数3: 获取纹理文件的高度
         参数4: 获取纹理文件的组件
         参数5: 获取纹理文件的格式
         返回值: 指向图像数据的指针
         */
        pBytes = gltReadTGABits(szTextureFiles[iLoop], &iWidth, &iHeight, &iComponents, &eFormat);
        
        // 放大过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // 缩小过滤器
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // 环绕方式 -- S
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // 环绕方式 -- T
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        /*
         4.载入纹理
         void GLAPIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
         参数1:target,使用的纹理类型GL_TEXTURE_1D\GL_TEXTURE_2D\GL_TEXTURE_3D
         参数2:level ,Mip贴图的层次
         参数3:internalformat,纹理单元的颜色组成方式(从读取像素图中获得)
         参数4:width
         参数5:height
         参数6:border
         参数7:format
         参数8:type,像素数据的数据类型-GL_UNSIGNED_BYTE
         参数9:pixels,指向纹理图像数据的指针
         */
        glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
        
        /*
         5.生成MIP图层
         参数1:GL_TEXTURE_1D\GL_TEXTURE_2D\GL_TEXTURE_3D
         */
        glGenerateMipmap(GL_TEXTURE_2D);
        
        free(pBytes);
    }
    // 建立立体图像
    GLfloat z;
    /*
     void Begin(GLenum primitive, GLuint nVerts, GLuint nTextureUnits = 0);
     参数1: 绘图模式
     参数2: 顶点个数
     参数3: 纹理，默认等于0
     */
    floorBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    
    for (z = 60.0f; z >= 0.0f; z -= 10.0f) {
        // 指定左下角顶点
        floorBatch.Vertex3f(-10.0f, -10.0f, z);
        // 指定顶点对应纹理的坐标
        floorBatch.MultiTexCoord2f(0, 0, 0);
        
        // 指定右下角顶点以及纹理坐标
        floorBatch.Vertex3f(10.0f, -10.0f, z);
        floorBatch.MultiTexCoord2f(0, 1, 0);
        
        floorBatch.Vertex3f(-10.0f, -10.0f, z-10.0f);
        floorBatch.MultiTexCoord2f(0, 0, 1);
        
        floorBatch.Vertex3f(10.0f, -10.0f, z-10.0f);
        floorBatch.MultiTexCoord2f(0, 1, 1);
    }
    floorBatch.End();
    
    ceilingBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for(z = 60.0f; z >= 0.0f; z -=10.0f)
    {
        ceilingBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        ceilingBatch.Vertex3f(-10.0f, 10.0f, z - 10.0f);
        
        ceilingBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        ceilingBatch.Vertex3f(10.0f, 10.0f, z - 10.0f);
        
        ceilingBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        ceilingBatch.Vertex3f(-10.0f, 10.0f, z);
        
        ceilingBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        ceilingBatch.Vertex3f(10.0f, 10.0f, z);
    }
    ceilingBatch.End();
    
    leftWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for(z = 60.0f; z >= 0.0f; z -=10.0f)
    {
        leftWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        leftWallBatch.Vertex3f(-10.0f, -10.0f, z);
        
        leftWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        leftWallBatch.Vertex3f(-10.0f, 10.0f, z);
        
        leftWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        leftWallBatch.Vertex3f(-10.0f, -10.0f, z - 10.0f);
        
        leftWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        leftWallBatch.Vertex3f(-10.0f, 10.0f, z - 10.0f);
    }
    leftWallBatch.End();
    
    rightWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for(z = 60.0f; z >= 0.0f; z -=10.0f)
    {
        rightWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
        rightWallBatch.Vertex3f(10.0f, -10.0f, z);
        
        rightWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
        rightWallBatch.Vertex3f(10.0f, 10.0f, z);
        
        rightWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
        rightWallBatch.Vertex3f(10.0f, -10.0f, z - 10.0f);
        
        rightWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
        rightWallBatch.Vertex3f(10.0f, 10.0f, z - 10.0f);
    }
    rightWallBatch.End();
}

// 关闭渲染环境
void ShutdownRC() {
    // 删除纹理
    glDeleteTextures(TEXTURE_COUNT, textures);
}

// 前后移动视口来对方向键做出响应
void SpecialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) {
        viewZ += 0.5f;
    }
    else if (key == GLUT_KEY_DOWN) {
        viewZ -= 0.5f;
    }
    glutPostRedisplay();
}

// 改变视景体和视口，在改变窗口大小或初始化窗口调用
void ChangeSize(int w, int h) {
    if (h == 0) {
        h = 1;
    }
    
    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(80.0f, (GLfloat)w/(GLfloat)h, 1.0, 120.0);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

// 调用绘制场景
void RenderScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Translate(0.0f, 0.0f, viewZ);
    /*
     纹理替换矩阵着色器
     参数1: GLT_SHADER_TEXTURE_REPLACE
     参数2: 模型视图矩阵 & 投影矩阵
     参数3: 纹理层次
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
    
    // 地板
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_FLOOR]);
    floorBatch.Draw();
    
    // 天花板
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_CEILING]);
    ceilingBatch.Draw();
    
    // 左右墙壁
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BRICK]);
    leftWallBatch.Draw();
    rightWallBatch.Draw();
    
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
}

int main(int argc, char *argv[]) {
    gltSetWorkingDirectory(argv[0]);
    
    // 标准初始化
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Tunnel");
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
    
    // 添加菜单入口，改变过滤器
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("GL_NEAREST", 0);
    glutAddMenuEntry("GL_LINEAR", 1);
    glutAddMenuEntry("GL_NEAREST_MIPMAP_NEAREST", 2);
    glutAddMenuEntry("GL_NEAREST_MIPMAP_LINEAR", 3);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_NEAREST", 4);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_LINEAR", 5);
    glutAddMenuEntry("Anisotropic Filter", 6);
    glutAddMenuEntry("Anisotropic Off", 7);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}
