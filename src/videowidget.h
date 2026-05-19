#ifndef SHOWVIDEOWIDGET_H
#define SHOWVIDEOWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QFile>
#include <vector>
#include <array>

class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();

    void inputOneFrame(const std::vector<uchar> &data, int width, int height);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int window_W, int window_H) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    GLuint textureUniformY;
    GLuint textureUniformUV;

    QOpenGLShaderProgram *mShaderProgram;
    QOpenGLTexture* mTextureY;
    QOpenGLTexture* mTextureUV;

    QOpenGLVertexArrayObject *mVAO;
    QOpenGLBuffer *mVBO;
    QOpenGLBuffer *mEBO;

    int mVideoW; //视频分辨率宽
    int mVideoH; //视频分辨率高

    std::vector<uchar> mVideoFrame; //当前视频帧数据
    std::array<std::array<float, 5>, 4> mVertices;
    bool mIsOpenGLInited; //openGL初始化函数是否执行过了
};

#endif // SHOWVIDEOWIDGET_H
