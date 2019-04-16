//
//  main.cpp
//  OpenGL-GeometricPrimitives
//
//  Created by 王傲云 on 2019/4/12.
//  Copyright © 2019 Jace. All rights reserved.
//

/*
 * 实现功能：点击屏幕，将固定位置上的顶点数据以7种不同形态展示
 */

#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

/*
 GLMatrixStack 变化管线使用的矩阵堆栈
 
 GLMatrixStack 构造函数允许指定堆栈的最大深度、默认的堆栈深度为64，这个矩阵堆栈在初始化时已经在堆栈中包含了单位矩阵
 GLMatrixStack::GLMatrixStack(int iStackDepth = 64);
 
 void GLMatrixStack::LoadIdentity(void);     // 通过调用顶部载入这个单位矩阵
 
 void GLMAtrixStack::LoadMatrix(const M3DMatrix44f mMatrix);     // 在堆栈顶部载入任何矩阵
 */

// 各种需要的类
GLShaderManager          shaderManager;
GLMatrixStack            modelViewMatrix;
GLMatrixStack            projectionMatrix;
GLFrame                  cameraFrame;
GLFrame                  objectFrame;
GLFrustum                viewFrustum;     // 投影矩阵

// 容器类（7种不同的图元对应的7种容器对象）
GLBatch                  pointBatch;
GLBatch                  lineBatch;
GLBatch                  lineStripBatch;
GLBatch                  lineLoopBatch;
GLBatch                  triangleBatch;
GLBatch                  triangleStripBatch;
GLBatch                  triangleFanBatch;

// 几何变换的管道
GLGeometryTransform      transformPipeline;
M3DMatrix44f             shadowMatrix;

GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

// 跟种效果步骤
int nStep = 0;

// 此函数在呈现上下文中进行任何需要的初始化
// 这是第一次做任何与OpenGL相关的任务
void SetupRC() {
    // 灰色的背景
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    // 初始化着色管理器
    shaderManager.InitializeStockShaders();
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);
    // 设置变换管线以使用两个矩阵堆栈
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    cameraFrame.MoveForward(-15.0f);
    
    /*
     常见函数：
     void GLBatch::Begin(GLenum primitive, GLuint nVerts, GLuint nTextureUnits = 0);
     参数1:表示使用的图元
     参数2:顶点数
     参数3:纹理坐标（可选）
     
     // 负责定点坐标
     void GLBatch::CopyVertexData3f(M3DVector3f *vVerts);
     
     // 结束，表示已经完成数据复制工作
     void GLBatch::End(void)
     */
    // 定义一些店，类似佛罗里达州的形状
    GLfloat vCoast[24][3] = {
        {2.80, 1.20, 0.0 }, {2.0,  1.20, 0.0 },
        {2.0,  1.08, 0.0 }, {2.0,  1.08, 0.0 },
        {0.0,  0.80, 0.0 }, {-.32, 0.40, 0.0 },
        {-.48, 0.2, 0.0 }, {-.40, 0.0, 0.0 },
        {-.60, -.40, 0.0 }, {-.80, -.80, 0.0 },
        {-.80, -1.4, 0.0 }, {-.40, -1.60, 0.0 },
        {0.0, -1.20, 0.0 }, { .2, -.80, 0.0 },
        {.48, -.40, 0.0 }, {.52, -.20, 0.0 },
        {.48,  .20, 0.0 }, {.80,  .40, 0.0 },
        {1.20, .80, 0.0 }, {1.60, .60, 0.0 },
        {2.0, .60, 0.0 }, {2.2, .80, 0.0 },
        {2.40, 1.0, 0.0 }, {2.80, 1.0, 0.0 }};
    
    // 用点的形式 -- 表示佛罗里达州的形状
    pointBatch.Begin(GL_POINTS, 24);
    pointBatch.CopyVertexData3f(vCoast);
    pointBatch.End();
    
    // 通过线的形式 -- 表示佛罗里达州的形状
    lineBatch.Begin(GL_LINES, 24);
    lineBatch.CopyVertexData3f(vCoast);
    lineBatch.End();
    
    // 通过线段的形式 -- 表示佛罗里达州的形状
    lineStripBatch.Begin(GL_LINE_STRIP, 24);
    lineStripBatch.CopyVertexData3f(vCoast);
    lineStripBatch.End();
    
    // 通过线环的形式 -- 表示佛罗里达州的形状
    lineLoopBatch.Begin(GL_LINE_LOOP, 24);
    lineLoopBatch.CopyVertexData3f(vCoast);
    lineLoopBatch.End();
    
    // 通过三角形创建金字塔
    GLfloat vPyramid[18][3] = {
        -2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f,
        
        2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,
        
        2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,
        
        -2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f,
        // 封底
        2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, 2.0f,
        2.0f, .0f, -2.0f,
        
        -2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, -2.0f,
    };
    
    // GL_TRIANGLES 没3个定点定义一个新的三角形
    triangleBatch.Begin(GL_TRIANGLES, 18);
    triangleBatch.CopyVertexData3f(vPyramid);
    triangleBatch.End();
    
    // 三角形扇形 -- 六边形
    GLfloat vPoints[100][3];     // 超过我们需要的数组
    int nVerts = 0;
    GLfloat r = 3.0f;
    
    // 原点(x, y, z) = (0, 0, 0);
    vPoints[nVerts][0] = 0.0f;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;
    
    // M3D_2PI 表示一个圆的角度
    for (GLfloat angle = 0; angle < M3D_2PI; angle += M3D_2PI / 6.0f) {
        // 数组下标自增（每自增1次就表示一个顶点）
        nVerts++;
        // 弧长 = 半径 * 角度，这里的角度是弧度制，不是平时的角度制
        // x 点坐标 cos(angle) * 半径
        vPoints[nVerts][0] = float(cos(angle)) * r;
        // y 点坐标 sin(angle) * 半径
        vPoints[nVerts][1] = float(cos(angle)) * r;
        // z 点坐标
        vPoints[nVerts][2] = -0.5f;
    }
    // 结束扇形 前面一共绘制了7个顶点（包括圆形）
    printf("三角形扇形顶点数：%d\n", nVerts);
    // 添加闭合的终点
    nVerts++;
    vPoints[nVerts][0] = r;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = -0.5f;
    
    // GL_TRIANGLE_FAN 以一个圆心为中心呈扇形排列，共用相邻顶点的一组三角形
    triangleFanBatch.Begin(GL_TRIANGLE_FAN, 8);
    triangleFanBatch.CopyVertexData3f(vPoints);
    triangleFanBatch.End();
    
    // 三角形条带，一个小环或圆柱段
    // 顶点下标
    int iCounter = 0;
    // 半径
    GLfloat radius = 3.0f;
    // 从0度～360度，以0.3弧度为步长
    for (GLfloat angle = 0.0f; angle <= M3D_2PI; angle += 0.3f) {
        // 获取圆形的顶点的x, y
        GLfloat x = radius * cos(angle);
        GLfloat y = radius * sin(angle);
        
        // 绘制2个三角形（顶点的(x, y)值一样，z值不一样）
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = -0.5f;
        iCounter++;
        
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = 0.5f;
        iCounter++;
    }
    // 关闭循环
    printf("三角形带的顶点数：%d\n", iCounter);
    // 结束循环，在循环位置生成2个三角形
    vPoints[iCounter][0] = vPoints[0][0];
    vPoints[iCounter][1] = vPoints[0][1];
    vPoints[iCounter][2] = -0.5f;
    iCounter++;
    
    vPoints[iCounter][0] = vPoints[1][0];
    vPoints[iCounter][1] = vPoints[1][1];
    vPoints[iCounter][2] = 0.5f;
    iCounter++;
    
    // GL_TRIANGLE_STRIP 共用一个条带（strip）上的顶点的一组三角形
    triangleStripBatch.Begin(GL_TRIANGLE_STRIP, iCounter);
    triangleStripBatch.CopyVertexData3f(vPoints);
    triangleStripBatch.End();
}

void DrawWireFramedBatch(GLBatch *pBatch) {
    /*
     -----------------------画绿色部分-----------------------
     GLShaderManager 中的uniform 值 ———— 平面着色器
     参数1：平面着色器
     参数2：为几何图形变换管线指定一个 4*4 变换矩阵
           --transformPipeline 变换管线（指定了2个矩阵堆栈）
     参数3：颜色值
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    pBatch->Draw();
    
    /*
     -----------------------边框部分-----------------------
     glEnable(GLenum mode); 用于启用各种功能，功能由参数决定
     参数列表：http://blog.csdn.net/augusdi/article/details/23747081
     注意：glEnagle() 不能写在 glBegin() 和 glEnd() 中间
     GL_POLYGON_OFFSET_LINE  根据函数glPolygonOffset的设置，启用线的深度偏移
     GL_LINE_SMOOTH          执行后，过虑线点的锯齿
     GL_BLEND                启用颜色混合。例如实现半透明效果
     GL_DEPTH_TEST           启用深度测试 根据坐标的远近自动隐藏被遮住的图形
     
     glDisable(GLenum mode); 用于关闭指定的功能，功能由参数决定
     */
    // 画黑色边框
    glPolygonOffset(-1.0f, -1.0f); // 偏移深度，在同一位置要绘制填充和边线，会产生z冲突，所以要偏移
    glEnable(GL_POLYGON_OFFSET_LINE);
    // 画反锯齿，让黑边好看些
    glEnable(GL_LINE_SMOOTH);
    // 开启混合
    glEnable(GL_BLEND);
    // 指定混合因子
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 绘制线框几何黑色版 三种模式，实心，边框，点，可以作用在正面，背面，或者两面
    // 通过调用glPolygonMode将多边形正面或者背面设为线框模式，实现线框渲染
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // 设置线条宽度
    glLineWidth(2.5f);
    
    /*
     GLShaderManager 中的uniform 值 ———— 平面着色器
     参数1：平面着色器
     参数2：为几何图形变换指定一个 4*4 变换矩阵
           --transformPipeline.GetModelViewProjectionMatrix() 获取的
           GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    // 绘制
    pBatch->Draw();
    
    // 复原原本的设置
    // 通过调用glPolygonMode将多边形正面或者背面设为全部填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // 关闭GL_POLYGON_OFFSET_LINE 模型
    glDisable(GL_POLYGON_OFFSET_LINE);
    // 设置线条宽度
    glLineWidth(1.0f);
    // 关闭混合
    glDisable(GL_BLEND);
    // 关闭线条光滑功能
    glDisable(GL_LINE_SMOOTH);
}

// 召唤场景
void RenderScene(void) {
    // 清除屏幕缓存区、深度缓冲区、模版缓存区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    // 压栈
    modelViewMatrix.PushMatrix();
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    
    // 矩阵乘以矩阵堆栈的顶部矩阵，相乘j的结果随后将存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mCamera);
    
    M3DMatrix44f mObjectFrame;
    // 只要使用 GetMatrix 函数就可以获取矩阵堆栈顶部的值，这个函数可以进行2次重载。用来使用GLShadeerManager的。或者是获取顶部矩阵的顶点副本数据
    objectFrame.GetMatrix(mObjectFrame);
    
    // 矩阵乘以矩阵堆栈的顶部矩阵，相乘j的结果随后将存储在堆栈的顶部
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    /*
     GLShaderManager 中的uniform 值 ———— 平面着色器
     参数1：平面着色器
     参数2：为几何图形变换指定一个 4*4 变换矩阵
           --transformPipeline.GetModelViewProjectionMatrix() 获取的
           GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    
    switch(nStep) {
        case 0:
            //设置点的大小
            glPointSize(4.0f);
            pointBatch.Draw();
            glPointSize(1.0f);
            break;
        case 1:
            //设置线的宽度
            glLineWidth(2.0f);
            lineBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 2:
            glLineWidth(2.0f);
            lineStripBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 3:
            glLineWidth(2.0f);
            lineLoopBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 4:
            DrawWireFramedBatch(&triangleBatch);
            break;
        case 5:
            DrawWireFramedBatch(&triangleStripBatch);
            break;
        case 6:
            DrawWireFramedBatch(&triangleFanBatch);
            break;
    }
    
    // 还原到以前的模型视图矩阵（单位矩阵）
    modelViewMatrix.PopMatrix();
    
    // 进行缓冲区交换
    glutSwapBuffers();
}

// 特殊键位处理（上、下、左、右移动）
void SpecialKeys(int key, int x, int y) {
    // 通过移动世界坐标系，来实现物体旋转的功能
    if (key == GLUT_KEY_UP) {
        // 围绕一个指定的X，Y，Z轴旋转
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    }
    if (key == GLUT_KEY_DOWN) {
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
    }
    if (key == GLUT_KEY_LEFT) {
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    }
    if (key == GLUT_KEY_RIGHT) {
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    }
    glutPostRedisplay();
}

// 根据空格次数，切换不同的“窗口名称”
// key: 敲击的键位
// x,y: 光标的位置
void KeyPressFunc(unsigned char key, int x, int y) {
    // 空格的ASCII码是32
    if (key == 32) {
        nStep++;
        if (nStep > 6) {
            nStep = 0;
        }
        
        switch (nStep) {
            case 0:
                glutSetWindowTitle("GL_POINTS");
                break;
            case 1:
                glutSetWindowTitle("GL_LINES");
                break;
            case 2:
                glutSetWindowTitle("GL_LINE_STRIP");
                break;
            case 3:
                glutSetWindowTitle("GL_LINE_LOOP");
                break;
            case 4:
                glutSetWindowTitle("GL_TRIANGLES");
                break;
            case 5:
                glutSetWindowTitle("GL_TRIANGLE_STRIP");
                break;
            case 6:
                glutSetWindowTitle("GL_TRIANGLE_FAN");
                break;
        }
        
        glutPostRedisplay();
    }
}

// 鼠标点击事件
void MouseKey(int button, int state, int x, int y) {
    //button:那个键位，左键(GLUT_LEFT_BUTTON)、右键(GLUT_RIGHT_BUTTON)
    //state:按下(GLUT_UP)/抬起(GLUT_DOWN)
    //x,y：光标位置
}

// 窗口已修改大小，或刚刚创建，都会执行此方法
// 使用窗口维度设置视口和投影矩阵
void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
    // 创建投影矩阵，并将它载入投影矩阵堆栈中
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // 获取矩阵堆栈顶部的单元矩阵
    modelViewMatrix.LoadIdentity();
}

int main(int argc, char* argv[]) {
    gltSetWorkingDirectory(argv[0]);
    glutInit(&argc, argv);
    
    // 申请一个颜色缓存区、深度缓存区、双缓存区、模版缓存区
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    
    // 设置window的c尺寸
    glutInitWindowSize(800, 600);
    // 创建window的名称
    glutCreateWindow("GL_POINTS");
    // 注册回调函数（改变尺寸）
    glutReshapeFunc(ChangeSize);
    // 注册一个键盘ASCII码的键位事件的回调函数，当有键盘按下时，则执行回调函数
    glutKeyboardFunc(KeyPressFunc);
    // 特殊键位函数（上下左右）
    glutSpecialFunc(SpecialKeys);
    // 注册鼠标点击事件
    glutMouseFunc(MouseKey);
    // 显示函数
    glutDisplayFunc(RenderScene);
    
    // 判断一下是否能初始化glew库，确保项目能正常使用OpenGL框架
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    // 绘制
    SetupRC();
    
    // runloop运行循环
    glutMainLoop();
    
    return 0;
}
