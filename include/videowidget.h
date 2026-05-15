#ifndef SHOWVIDEOWIDGET_H
#define SHOWVIDEOWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QFile>

#include "videoRawFrame.h"

namespace Ui {
class VideoWidget;
}

struct FaceInfoNode
{
    QRect faceRect;
};

///显示视频用的widget（使用OPENGL绘制YUV420P数据）
///这个仅仅是显示视频画面的控件

class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = 0);
    ~VideoWidget();

    void setPlayerId(QString id) { //用于协助拖拽 区分是哪个窗口
        mPlayerId=id;
    }

    QString getPlayerId() {
        return mPlayerId;
    }

    void setShowFaceRect(bool value) {
        mIsShowFaceRect = value;
    }

    qint64 getLastGetFrameTime(){
        return mLastGetFrameTime;
    }

    void setCloseAble(bool isCloseAble);
    void clear();
    void setIsPlaying(bool value);
    void setPlayFailed(bool value);
    void setCameraName(QString name);
    void setVideoWidth(int w, int h);

    void inputOneFrame(VideoRawFramePtr videoFrame);

signals:
    void sig_CloseBtnClick();
    void sig_Drag(QString id_from, QString id_to);

protected:
    void enterEvent(QEnterEvent  *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void resizeGL(int window_W, int window_H) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;

private:
    ///OPenGL用于绘制图像
    GLuint textureUniformY; //y纹理数据位置
    GLuint textureUniformU; //u纹理数据位置
    GLuint textureUniformV; //v纹理数据位置
    GLuint id_y; //y纹理对象ID
    GLuint id_u; //u纹理对象ID
    GLuint id_v; //v纹理对象ID
    QOpenGLTexture* m_pTextureY;  //y纹理对象
    QOpenGLTexture* m_pTextureU;  //u纹理对象
    QOpenGLTexture* m_pTextureV;  //v纹理对象
    QOpenGLShader *m_pVSHader;  //顶点着色器程序对象
    QOpenGLShader *m_pFSHader;  //片段着色器对象
    QOpenGLShaderProgram *m_pShaderProgram; //着色器程序容器
    GLfloat *m_vertexVertices; // 顶点矩阵

    float mPicIndex_X; //按比例显示情况下 图像偏移量百分比 (相对于窗口大小的)
    float mPicIndex_Y; //
    int m_nVideoW; //视频分辨率宽
    int m_nVideoH; //视频分辨率高

    VideoRawFramePtr mVideoFrame;
    QList<FaceInfoNode> mFaceInfoList;

    bool mIsOpenGLInited; //openGL初始化函数是否执行过了

    ///OpenGL用于绘制矩形
    bool mIsShowFaceRect;
    GLuint m_posAttr;
    GLuint m_colAttr;
    QOpenGLShaderProgram *m_program;

    bool mCurrentVideoKeepAspectRatio; //当前模式是否是按比例 当检测到与全局变量不一致的时候 则重新设置openGL矩阵

    QString mPlayerId;
    bool mIsPlaying;
    bool mPlayFailed; //播放失败
    bool mIsCloseAble; //是否显示关闭按钮

    QString mCameraName;
    qint64 mLastGetFrameTime; //上一次获取到帧的时间戳
    void resetGLVertex(int window_W, int window_H);

private:
    Ui::VideoWidget *ui;

};

#endif // SHOWVIDEOWIDGET_H
