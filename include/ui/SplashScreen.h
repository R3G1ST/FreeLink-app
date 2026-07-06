#pragma once

#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)

public:
    explicit SplashScreen(const QPixmap &pixmap = QPixmap());
    ~SplashScreen();

    void setProgress(qreal value);
    qreal progress() const { return m_progress; }
    void showStatus(const QString &message);

    void fadeOut();

    static SplashScreen *instance() { return s_instance; }

private:
    void paintEvent(QPaintEvent *event) override;

    qreal m_progress = 0;
    QString m_statusMessage;
    QTimer *m_dotsTimer = nullptr;
    int m_dotsCount = 0;
    qreal m_fadeOpacity = 1.0;

    static SplashScreen *s_instance;
};
