//
//  main.cpp
//  OpenGL-Rectangle
//
//  Created by 王傲云 on 2019/4/12.
//  Copyright © 2019 Jace. All rights reserved.
//

#include <stdio.h>
#include "GLTools.h"
#include <GLUT/GLUT.h>

GLBatch triangleBatch;
GLShaderManager shaderManager;

GLfloat blockSize = 0.1f;     // 边长长度
// 正方形的4点坐标
GLfloat vVerts[] = {
    -blockSize, -blockSize, 0.0f,
    blockSize, -blockSize, 0.0f,
    blockSize, blockSize, 0.0f,
    -blockSize, blockSize, 0.0f
};

GLfloat xPot = 0.0f;     // x轴的移动距离
GLfloat yPot = 0.0f;     // y轴的移动距离

// 窗口大小改变时接受新的宽度和高度，其中0，0代表窗口中视口的左下角坐标，w，h代表像素
void ChangeSize(int w, int h) {
    // 设置渲染区域
    glViewport(0, 0, w, h);
}

// 为程序做一次性的设置
void SetupRC() {
    // 设置背景颜色
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    // 初始化着色管理器
    shaderManager.InitializeStockShaders();
    // 设置三角形，其中数组vVert包含所有3个顶点（x,y，z）的笛卡尔坐标系。
    //    GLfloat vVerts[] = {
    //        -0.5f, 0.0f, 0.0f,
    //        0.5f, 0.0f, 0.0f,
    //        0.0f, 0.5f, 0.0f,
    //    };
    // 批次处理
    triangleBatch.Begin(GL_TRIANGLE_FAN, 4);
    triangleBatch.CopyVertexData3f(vVerts);
    triangleBatch.End();
}

// 开始渲染
void RenderScene(void) {
    // 清除一个或一组特定的缓冲区
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    // 设置一组浮点数来表示红色
    GLfloat vRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
    // 传递到存储着色器，即GLT_SHADER_IDENTITY着色器，这个着色器只是使用指定颜色以默认笛卡尔坐标系在屏幕上渲染几何图形
    //    shaderManager.UseStockShader(GLT_SHADER_IDENTITY, vRed);
    // 利用矩阵帮助移动
    /*
     * mFinalTransform 结果矩阵
     * mTransformMatrix 平移矩阵
     * mRotationMatrix 旋转矩阵
     */
    M3DMatrix44f mFinalTransform, mTransformMatrix, mRotationMatrix;
    
    // 平移 x,y,z,w(缩放因子，默认为1)
    // 3D中平移原理与矩阵之间的关系
    /*
     * 参数1: 矩阵
     * 参数2、3、4: X，Y，Z上平移距离
     */
    m3dTranslationMatrix44(mTransformMatrix, xPot, yPot, 0.0f);
    // 一边旋转一边移动
    static float yRot = 0.0f;
    /*
     参数1: 矩阵
     参数2: 弧度
     参数3: X：1/0     1:围绕X轴旋转，0:不围绕X轴旋转
     参数4: Y：1/0
     参数5: Z：1/0
     */
    m3dRotationMatrix44(mRotationMatrix, m3dDegToRad(yRot), 0, 0, 1);
    yRot += 5.0f;     // 修改旋转度数
    m3dMatrixMultiply44(mFinalTransform, mTransformMatrix, mRotationMatrix);
    // 平面着色器
    /*
     1.平移矩阵 mTransformMatrix 与每个顶点 相乘 -> 新顶点（顶点着色器）
     2.将片元着色红色（片元着色器）
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, mFinalTransform, vRed);
    // 提交着色器
    triangleBatch.Draw();
    // 将在后台缓冲区进行渲染，然后在结束时交换到前台
    glutSwapBuffers();
}

void specialKey(int key, int x, int y) {
    GLfloat stepSize = 0.025f;
    if( key == GLUT_KEY_UP) {
        yPot += stepSize;
    }
    else if(key == GLUT_KEY_DOWN) {
        yPot -= stepSize;
    }
    else if(key == GLUT_KEY_LEFT) {
        xPot -= stepSize;
    }
    else if(key == GLUT_KEY_RIGHT) {
        xPot += stepSize;
    }
    if(xPot < -1.0+blockSize) {
        xPot = -1.0+blockSize;
    }
    if(xPot > 1.0-blockSize) {
        xPot = 1.0-blockSize;
    }
    if(yPot < -1.0+blockSize) {
        yPot = -1.0+blockSize;
    }
    if(yPot > 1.0-blockSize) {
        yPot = 1.0-blockSize;
    }
    // 请求重新渲染绘制
    glutPostRedisplay();
}

int main(int argc, char* argv[]) {
    // 设置当前工作目录，针对MAC OS X
    gltSetWorkingDirectory(argv[0]);
    // 初始化GLUT库
    glutInit(&argc, argv);
    // 初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指双缓冲窗口、RGBA颜色模式、深度测试、模版缓冲区
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    // GLUT窗口大小，标题窗口
    glutInitWindowSize(800, 600);
    glutCreateWindow("Triangle");
    // 注册回调函数
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(specialKey);
    // 驱动程序的初始化中没有出现任何问题
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "glew error:%s\n", glewGetErrorString(err));
        return 1;
    }
    // 调用SetupRC
    SetupRC();
    glutMainLoop();
    return 0;
}
