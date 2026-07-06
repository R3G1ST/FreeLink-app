#pragma once

#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QElapsedTimer>

class CircularProgress : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal progress READ progress WRITE setProgress)
    Q_PROPERTY(qreal spinAngle READ spinAngle WRITE setSpinAngle)
    Q_PROPERTY(qreal glowRadius READ glowRadius WRITE setGlowRadius)

public:
    enum State {
        Idle,
        Downloading,
        Extracting,
        Restarting,
        Complete,
        Error
    };

    explicit CircularProgress(QWidget *parent = nullptr);
    ~CircularProgress();

    void setState(State state);
    State state() const { return m_state; }

    void setProgress(qreal value);
    qreal progress() const { return m_progress; }

    void setSpinAngle(qreal angle);
    qreal spinAngle() const { return m_spinAngle; }

    void setGlowRadius(qreal radius);
    qreal glowRadius() const { return m_glowRadius; }

    void setStatusText(const QString &text);
    void setProgressText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void startSpinAnimation();
    void stopSpinAnimation();
    void startGlowAnimation();

    State m_state = Idle;
    qreal m_progress = 0;
    qreal m_spinAngle = 0;
    qreal m_glowRadius = 0;

    QString m_statusText;
    QString m_progressText;

    QPropertyAnimation *m_spinAnimation = nullptr;
    QPropertyAnimation *m_glowAnimation = nullptr;
    QTimer *m_dotsTimer = nullptr;
    int m_dotsCount = 0;
};
