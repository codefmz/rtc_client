#pragma once

#include <QWidget>
#include <vector>
#include <cstdint>

class QPushButton;
class VideoWidget;

class VideoPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget* parent = nullptr);

    VideoWidget* videoWidget() const;
    bool isPaused() const;

    void inputOneFrame(const std::vector<uint8_t>& data, int width, int height);

signals:
    void pauseRequested();
    void resumeRequested();
    void pausedChanged(bool paused);

public slots:
    void setPaused(bool paused);
    void pause();
    void resume();

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateControlGeometry();
    void updatePlayButtonText();

    VideoWidget* mVideoWidget = nullptr;
    QWidget* mControlPanel = nullptr;
    QPushButton* mPlayButton = nullptr;
    bool mPaused = false;
};
