#include "videoplayerwidget.h"

#include "videowidget.h"

#include <QCursor>
#include <QEnterEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

VideoPlayerWidget::VideoPlayerWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    // 设置
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("VideoPlayerWidget { background: black; }");

    mVideoWidget = new VideoWidget(this);
    mVideoWidget->setMouseTracking(true);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(mVideoWidget);

    mControlPanel = new QWidget(this);
    mControlPanel->setObjectName("videoControlPanel");
    mControlPanel->setMouseTracking(true);
    mControlPanel->setStyleSheet(
        "#videoControlPanel {"
        "    background-color: rgba(0, 0, 0, 150);"
        "    border-radius: 6px;"
        "}"
        "#videoControlPanel QPushButton {"
        "    color: white;"
        "    background-color: rgba(255, 255, 255, 36);"
        "    border: 1px solid rgba(255, 255, 255, 80);"
        "    border-radius: 4px;"
        "    min-width: 72px;"
        "    min-height: 30px;"
        "    padding: 4px 12px;"
        "}"
        "#videoControlPanel QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 58);"
        "}"
        "#videoControlPanel QPushButton:pressed {"
        "    background-color: rgba(255, 255, 255, 82);"
        "}");

    auto* controlLayout = new QHBoxLayout(mControlPanel);
    controlLayout->setContentsMargins(4, 4, 4, 4);
    controlLayout->setSpacing(8);

    mPlayButton = new QPushButton(mControlPanel);
    updatePlayButtonText();
    controlLayout->addStretch();
    controlLayout->addWidget(mPlayButton);
    controlLayout->addStretch();

    connect(mPlayButton, &QPushButton::clicked, this, [this]() {
        setPaused(!mPaused);
    });

    mControlPanel->hide();
    mControlPanel->raise();
}

VideoWidget* VideoPlayerWidget::videoWidget() const
{
    return mVideoWidget;
}

bool VideoPlayerWidget::isPaused() const
{
    return mPaused;
}

void VideoPlayerWidget::inputOneFrame(const std::vector<unsigned char>& data, int width, int height)
{
    if (mPaused) {
        return;
    }

    mVideoWidget->inputOneFrame(data, width, height);
}

void VideoPlayerWidget::setPaused(bool paused)
{
    if (mPaused == paused) {
        return;
    }

    mPaused = paused;
    updatePlayButtonText();

    if (mPaused) {
        emit pauseRequested();
    } else {
        emit resumeRequested();
    }

    emit pausedChanged(mPaused);
}

void VideoPlayerWidget::pause()
{
    setPaused(true);
}

void VideoPlayerWidget::resume()
{
    setPaused(false);
}

void VideoPlayerWidget::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    updateControlGeometry();
    mControlPanel->show();
    mControlPanel->raise();
}

void VideoPlayerWidget::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);

    const QPoint cursorPos = mapFromGlobal(QCursor::pos());
    if (!rect().contains(cursorPos)) {
        mControlPanel->hide();
    }
}

void VideoPlayerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateControlGeometry();
}

void VideoPlayerWidget::updateControlGeometry()
{
    const int margin = 4;
    const int panelHeight = 46;
    const int panelWidth = qMax(160, width() - margin * 2);
    const int x = (width() - panelWidth) / 2;
    const int y = height() - panelHeight - margin;

    mControlPanel->setGeometry(x, qMax(margin, y), panelWidth, panelHeight);
}

void VideoPlayerWidget::updatePlayButtonText()
{
    if (!mPlayButton) {
        return;
    }

    mPlayButton->setText(mPaused ? QStringLiteral("继续") : QStringLiteral("暂停"));
}
