//
//  main.cpp
//  OpenGL-Front_Back_Cull-Depth_Test
//
//  Created by 王傲云 on 2019/4/15.
//  Copyright © 2019 Jace. All rights reserved.
//

#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLGeometryTransform.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

// 设置角色帧，作为相机
GLFrame             viewFrame;

// 使用GLFrustum类来设置透视投影
GLFrustum           viewFrustum;
GLTriangleBatch     torusBatch;
GLMatrixStack       modelViewMatrix;
GLMatrixStack       projectionMatrix;
GLGeometryTransform transfromPipeline;
GLShaderManager     shaderManager;

// 标记：背面剔除、深度测试
int isCull = 0;
int isDepth = 0;

// 右键菜单栏选项，点击菜单栏中的选项会调用此方法
void ProcessMenu(int value) {
    switch (value) {
        case 1:
            isCull = !isCull;
            break;
        case 2:
            isDepth = !isDepth;
            break;
        case 3:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case 4:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case 5:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

//召唤场景
void RenderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (isCull) {
        // 实现背面剔除
        // 1.开启背面剔除功能
        glEnable(GL_CULL_FACE);
        // 2.指定模型
        glFrontFace(GL_CCW);
        // 3.执行剔除
        glCullFace(GL_BACK);
    }
    else {
        glDisable(GL_CULL_FACE);
    }
    
    // 设置深度测试
    if (isDepth) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
    
    modelViewMatrix.PushMatrix(viewFrame);
    
    GLfloat vRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
    
    // 默认光源着色器
    // 参数1:GLT_SHADER_DEFAULT_LIGHT
    // 参数2:模型视图矩阵
    // 参数3:投影矩阵
    // 参数4:颜色
    
    // transformPipeline 管理堆栈，模型视图堆栈/投影矩阵堆栈
    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transfromPipeline.GetModelViewMatrix(), transfromPipeline.GetProjectionMatrix(), vRed);
    
    // 绘制
    torusBatch.Draw();
    
    // 出栈
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
}

// context 图像上下文
void SetupRC() {
    // 设置清平颜色 -- 可以理解为设置背景颜色，存放在颜色缓存区中
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    // 初始化固定管线
    shaderManager.InitializeStockShaders();
    
    // 观察者
    // 参数1:表示z轴的h值
    viewFrame.MoveForward(7.0f);
    
    // 创建一个甜甜圈🍩
    /*
     void gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
     参数1:GLTriangleBatch
     参数2:外圆半径，从圆心到外圆的距离
     参数3:内边圆半径，从圆心到内圆的距离
     参数4/5:三角形带的数量 numMajor = 2 * numMinor
     */
    gltMakeTorus(torusBatch, 1.0f, 0.3f, 52, 26);
    
    // 设置点的大小
    glPointSize(4.0f);
}

// 键位设置，通过不同的键位对其进行设置
// 控制Camera的移动，从而改变视口
void SpecialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) {
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 1.0f, 0.0f, 0.0f);
    }
    else if (key == GLUT_KEY_DOWN) {
        viewFrame.RotateWorld(m3dDegToRad(5.0), 1.0f, 0.0f, 0.0f);
    }
    else if (key == GLUT_KEY_LEFT) {
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 0.0f, 1.0f, 0.0f);
    }
    else if (key == GLUT_KEY_RIGHT) {
        viewFrame.RotateWorld(m3dDegToRad(5.0), 0.0f, 1.0f, 0.0f);
    }
    
    // 重新刷新window
    glutPostRedisplay();
}

void ChangeSize(int w, int h) {
    // 防止h变为0
    if (h == 0) {
        h = 1;
    }
    
    // 设置视图窗口尺寸
    glViewport(0, 0, w, h);
    
    // setPerspective函数的参数是一个从顶点方向看去的视觉角度（用角度值表示）
    // 设置透视模式，初始化其透视矩阵
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    // 把透视矩阵加载到透视矩阵堆栈中
    // viewFrustum.GetProjectionMatrix() 获取投影矩阵
    // projectionMatrix.LoadMatrix(矩阵) 把矩阵加载到projectionMatrix中来
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // 初始化渲染管线
    transfromPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

int main(int argc, char *argv[]) {
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Geometry Test Program");
    
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
    
    // 设置右键菜单选项
    glutCreateMenu(ProcessMenu);
    /*
     添加右击菜单栏选项
     参数1:菜单栏中选项的内容
     参数2:标记值
    */
    glutAddMenuEntry("正面背面剔除", 1);
    glutAddMenuEntry("深度测试", 2);
    glutAddMenuEntry("Set Fill Mode", 3);
    glutAddMenuEntry("Set Line Mode", 4);
    glutAddMenuEntry("Set Point Mode", 5);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    SetupRC();
    
    glutMainLoop();
    return 0;
}
