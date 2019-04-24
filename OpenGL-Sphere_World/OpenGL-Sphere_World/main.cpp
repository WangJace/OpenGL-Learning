//
//  main.cpp
//  OpenGL-Sphere_World
//
//  Created by 王傲云 on 2019/4/23.
//  Copyright © 2019 Jace. All rights reserved.
//

#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];
GLShaderManager     shaderManager;          // 着色器管理器
GLMatrixStack       modelViewMatrix;        // 模型视图矩阵堆栈
GLMatrixStack       projectionMatrix;       // 投影矩阵堆栈
GLFrustum           viewFrustum;            // 视景体
GLGeometryTransform transformPipeline;      // 几何图形变换管道

GLTriangleBatch     torusBatch;             // 圆环批处理类
GLBatch             floorBatch;             // 地板批处理类

/* 定义公转球的批处理类（公转自转）*/
GLTriangleBatch     sphereBatch;            // 球批处理类

// 角色帧 照相机角色帧
GLFrame             cameraFrame;

void SetupRC() {
    shaderManager.InitializeStockShaders();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // 绘制圆环
    gltMakeTorus(torusBatch, 0.4f, 0.15f, 30.0f, 30.0f);
    // 绘制球体
    gltMakeSphere(sphereBatch, 0.1f, 26.0f, 13.0f);
    // 绘制地板
    floorBatch.Begin(GL_LINES, 324);
    // 地板的宽度
    for (GLfloat x = -20.0f; x <= 20.0f; x += 0.5) {
        floorBatch.Vertex3f(x, -0.55f, 20.0f);
        floorBatch.Vertex3f(x, -0.55f, -20.0f);
        
        floorBatch.Vertex3f(20.0f, -0.55f, x);
        floorBatch.Vertex3f(-20.0f, -0.55f, x);
    }
    floorBatch.End();
    
    // 随机防止球体 - 50个
    for (int i = 0; i < NUM_SPHERES; i++) {
        // y轴不变，x、z值随机产生
        GLfloat x = (GLfloat)(((rand() % 400) - 200) * 0.1f);
        GLfloat z = (GLfloat)(((rand() % 400) - 200) * 0.1f);
        spheres[i].SetOrigin(x, 0.0f, z);
    }
}

void ChangeSize(int w, int h) {
    if (h == 0) {
        h = 1;
    }
    glViewport(0, 0, w, h);
    // 设置投影矩阵
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    // 将投影矩阵添加到projectionMatrix中
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

void RenderScene() {
    static GLfloat vFloorColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    static GLfloat vTrousColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    static GLfloat vSphereColor[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    // 基于时间动画
    static CStopWatch rotTime;
    float yRot = rotTime.GetElapsedSeconds() * 60.0f;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // modelViewMatrix.PushMatrix();
    
    // 设置观察者矩阵
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.PushMatrix(mCamera);
    
    // 添加光源
    M3DVector4f vLightPos = { 0.0f, 10.0f, 5.0f, 1.0f };
    M3DVector4f vLightEyePos;
    
    // 将照相机的mCamera 与 光源矩阵vLightPos 矩阵相乘 vLightEyePos
    m3dTransformVector4(vLightEyePos, vLightPos, mCamera);
    
    // 绘制地板
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor);
    floorBatch.Draw();
    
    // 绘制悬浮随机小球体
    for (int i = 0; i < NUM_SPHERES; i++) {
        modelViewMatrix.PushMatrix();
        modelViewMatrix.MultMatrix(spheres[i]);
        // shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vSphereColor);
        /*
         默认光源着色器
         参数1:GLT_SHADER_POINT_LIGHT_DIFF
         参数2:模型视图矩阵
         参数3:投影矩阵
         参数4:光源位置
         参数5:漫反射的颜色
         */
        shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightEyePos, vSphereColor);
        sphereBatch.Draw();
        modelViewMatrix.PopMatrix();
    }
    
    // 向屏幕z轴负方向移动2.5个单位
    modelViewMatrix.Translate(0.0f, 0.0f, -2.5f);
    // 旋转
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    // 将结果压栈
    modelViewMatrix.PushMatrix();
    
    // 绘制圆环
    // shaderManager.UserStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vTrousColor);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightEyePos, vTrousColor);
    torusBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    // 绘制公转的球体
    modelViewMatrix.Rotate(yRot * -3.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
    // 将结果压栈
    modelViewMatrix.PushMatrix();
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vSphereColor);
    sphereBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    modelViewMatrix.PopMatrix();
    
    glutSwapBuffers();
    glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y) {
    float linar = 0.1f;
    float angular = float(m3dDegToRad(5.0f));
    if (key == GLUT_KEY_UP) {
        cameraFrame.MoveForward(linar);
    }
    else if (key == GLUT_KEY_DOWN) {
        cameraFrame.MoveForward(-linar);
    }
    else if (key == GLUT_KEY_RIGHT) {
        cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
    }
    else if (key == GLUT_KEY_LEFT) {
        cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
    }
}

int main(int argc, char* argv[]) {
    gltSetWorkingDirectory(argv[0]);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL SphereWorld");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    SetupRC();
    glutMainLoop();
    return 0;
}
