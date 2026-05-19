
#include "videowidget.h"
#include <QPainter>
#include <QDebug>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QFile>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMouseEvent>
#include <QTimer>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QScreen>
#include <QDateTime>

#include <fstream>
#include "plog/Log.h"

VideoWidget::VideoWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    mVideoH = 0;
    mVideoW = 0;
    mIsOpenGLInited = false;

    mVertices = {{
        {{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f}},
        {{ 1.0f, -1.0f, 0.0f, 1.0f, 0.0f}},
        {{-1.0f,  1.0f, 0.0f, 0.0f, 1.0f}},
        {{ 1.0f,  1.0f, 0.0f, 1.0f, 1.0f}},
    }};
}

VideoWidget::~VideoWidget()
{
}


void VideoWidget::inputOneFrame(const std::vector<uchar> &data, int width, int height)
{
    QMetaObject::invokeMethod(this, [=]() {
        if (mVideoW <= 0 || mVideoH <= 0 || mVideoW != width || mVideoH != height) {
            PLOGD << "video size change:" << width << height;

        }

        mVideoFrame = std::move(data);
        mVideoW = width;
        mVideoH = height;

        update(); //调用update将执行 paintEvent函数
    });
}

void VideoWidget::initializeGL()
{
    PLOGD << "initializeGL called.";
    mIsOpenGLInited = true;
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    std::ifstream vShaderFile;
    std::string vsFilePath = std::string(RES_DIR) + "//shader//shader.vs";
    vShaderFile.open(vsFilePath);
    if (!vShaderFile.is_open()) {
        PLOGE << "Failed to open vertex shader file:" << vsFilePath.c_str();
        return;
    }

    std::stringstream vShaderCode;
    vShaderCode << vShaderFile.rdbuf();
    vShaderFile.close();

    std::ifstream fShaderFile;
    std::string fsFilePath = std::string(RES_DIR) + "//shader.fs";
    fShaderFile.open(fsFilePath);
    if (!fShaderFile.is_open()) {
        PLOGE << "Failed to open fragment shader file:" << fsFilePath.c_str();
        return;
    }

    std::stringstream fShaderCode;
    fShaderCode << fShaderFile.rdbuf();
    fShaderFile.close();

    QOpenGLShader vsShader(QOpenGLShader::Vertex, this);
    vsShader.compileSourceCode(vShaderCode.str().c_str());
    QOpenGLShader fsShader(QOpenGLShader::Fragment, this);
    fsShader.compileSourceCode(fShaderCode.str().c_str());

    mShaderProgram = new QOpenGLShaderProgram(this);
    mShaderProgram->addShader(&vsShader);
    mShaderProgram->addShader(&fsShader);
    mShaderProgram->link();

    unsigned int indices[] = {
        0, 1, 2,
        1, 3, 2
    };

    //生成顶点数组对象与顶点缓冲对象
    mVAO = new QOpenGLVertexArrayObject(this);
    mVAO->create();
    mVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    mVBO->create();
    mEBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    mEBO->create();

    //绑定顶点数组对象与顶点缓冲对象
    mVAO->bind();
    mVBO->bind();
    mEBO->bind();

    // 复制顶点数组到缓冲中供OpenGL使用
    mVBO->allocate(mVertices.data(), sizeof(mVertices));
    mEBO->allocate(indices, sizeof(indices));

    //解析顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); //启用顶点属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 创建纹理
    mTextureY = new QOpenGLTexture(QOpenGLTexture::Target2D);
    mTextureY->create();
    mTextureY->setMinificationFilter(QOpenGLTexture::Linear);
    mTextureY->setMagnificationFilter(QOpenGLTexture::Linear);
    mTextureY->setWrapMode(QOpenGLTexture::ClampToBorder);

    mTextureUV = new QOpenGLTexture(QOpenGLTexture::Target2D);
    mTextureUV->create();
    mTextureUV->setMinificationFilter(QOpenGLTexture::Linear);
    mTextureUV->setMagnificationFilter(QOpenGLTexture::Linear);
    mTextureUV->setWrapMode(QOpenGLTexture::ClampToBorder);
}

void VideoWidget::resizeGL(int window_W, int window_H)
{
    PLOGD << "resizeGL, window_W = " << window_W << ", window_H = " << window_H;
    if (window_H == 0 || window_W == 0 || mVideoH == 0 || mVideoW == 0) {
        return;
    }

    glViewport(0, 0, window_W, window_H);
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float winAspect = (float)window_W / window_H;
    float videoAspect = (float)mVideoW / mVideoH;

    if (winAspect > videoAspect) {
        scaleX = videoAspect / winAspect;
    } else {
        scaleY = winAspect / videoAspect;
    }

    float vertices[] = {
        -scaleX, -scaleY, 0.0f,  0.0f, 0.0f,
         scaleX, -scaleY, 0.0f,  1.0f, 0.0f,
        -scaleX,  scaleY, 0.0f,  0.0f, 1.0f,
         scaleX,  scaleY, 0.0f,  1.0f, 1.0f,
    };

    mVBO->bind();
    mVBO->write(0, vertices, sizeof(vertices));
    mVBO->release();
}

void VideoWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!mVideoFrame.empty()) {
        const uint8_t *yuv420Data = (uint8_t *)mVideoFrame.data();
        const uint8_t *uvPlane = yuv420Data + mVideoW * mVideoH;

        mShaderProgram->bind();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 设置像素对齐, GPU默认是4字节对齐的, 但YUV420P的Y分量是1字节对齐的

        glActiveTexture(GL_TEXTURE0);
        mTextureY->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, mVideoW, mVideoH, 0, GL_RED, GL_UNSIGNED_BYTE, yuv420Data);
        mShaderProgram->setUniformValue("texY", 0); // 将纹理单元0传递给着色器中的uniform变量textureY

        PLOGD << "VideoWidget::paintGL, mVideoW = " << mVideoW << ", mVideoH = " << mVideoH;
        glActiveTexture(GL_TEXTURE1);
        mTextureUV->bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, mVideoW / 2, mVideoH / 2, 0, GL_RG, GL_UNSIGNED_BYTE, uvPlane);
        mShaderProgram->setUniformValue("texUV", 1);

        mVAO->bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        mTextureY->release();
        mTextureUV->release();
        mShaderProgram->release();
    }
}
