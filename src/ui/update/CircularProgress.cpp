#include "include/ui/update/CircularProgress.h"
#include <QPainter>
#include <QConicalGradient>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QEasingCurve>

CircularProgress::CircularProgress(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(120, 120);

    // Spin animation
    m_spinAnimation = new QPropertyAnimation(this, "spinAngle");
    m_spinAnimation->setDuration(1200);
    m_spinAnimation->setStartValue(0.0);
    m_spinAnimation->setEndValue(360.0);
    m_spinAnimation->setLoopCount(-1);
    m_spinAnimation->setEasingCurve(QEasingCurve::Linear);

    // Glow animation
    m_glowAnimation = new QPropertyAnimation(this, "glowRadius");
    m_glowAnimation->setDuration(1500);
    m_glowAnimation->setStartValue(0.0);
    m_glowAnimation->setKeyValueAt(0.5, 12.0);
    m_glowAnimation->setEndValue(0.0);
    m_glowAnimation->setLoopCount(-1);
    m_glowAnimation->setEasingCurve(QEasingCurve::InOutSine);

    // Dots animation
    m_dotsTimer = new QTimer(this);
    connect(m_dotsTimer, &QTimer::timeout, this, [this]() {
        m_dotsCount = (m_dotsCount + 1) % 4;
        update();
    });
}

CircularProgress::~CircularProgress()
{
    m_spinAnimation->stop();
    m_glowAnimation->stop();
    m_dotsTimer->stop();
}

void CircularProgress::setState(State state)
{
    m_state = state;

    switch (state) {
    case Downloading:
        startSpinAnimation();
        startGlowAnimation();
        m_dotsTimer->start(400);
        break;
    case Extracting:
        m_spinAnimation->setDuration(800);
        startSpinAnimation();
        m_glowAnimation->stop();
        m_dotsTimer->start(400);
        break;
    case Restarting:
        m_spinAnimation->stop();
        m_glowAnimation->stop();
        m_dotsTimer->start(400);
        break;
    case Complete:
    case Error:
        stopSpinAnimation();
        m_glowAnimation->stop();
        m_dotsTimer->stop();
        break;
    default:
        stopSpinAnimation();
        m_glowAnimation->stop();
        m_dotsTimer->stop();
        break;
    }

    update();
}

void CircularProgress::setProgress(qreal value)
{
    m_progress = qBound(0.0, value, 1.0);
    update();
}

void CircularProgress::setSpinAngle(qreal angle)
{
    m_spinAngle = angle;
    update();
}

void CircularProgress::setGlowRadius(qreal radius)
{
    m_glowRadius = radius;
    update();
}

void CircularProgress::setStatusText(const QString &text)
{
    m_statusText = text;
    m_dotsCount = 0;
    update();
}

void CircularProgress::setProgressText(const QString &text)
{
    m_progressText = text;
    update();
}

void CircularProgress::startSpinAnimation()
{
    if (m_spinAnimation->state() != QPropertyAnimation::Running) {
        m_spinAnimation->start();
    }
}

void CircularProgress::stopSpinAnimation()
{
    if (m_spinAnimation->state() == QPropertyAnimation::Running) {
        m_spinAnimation->stop();
        m_spinAngle = 0;
    }
}

void CircularProgress::startGlowAnimation()
{
    if (m_glowAnimation->state() != QPropertyAnimation::Running) {
        m_glowAnimation->start();
    }
}

void CircularProgress::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int centerX = w / 2;
    int centerY = h / 2;
    int radius = qMin(w, h) / 2 - 10;

    // Glow effect
    if (m_glowRadius > 0 && m_state == Downloading) {
        int glowR = static_cast<int>(radius + m_glowRadius);
        QRadialGradient glowGrad(centerX, centerY, glowR);
        glowGrad.setColorAt(0, QColor(139, 92, 246, 40));
        glowGrad.setColorAt(0.7, QColor(139, 92, 246, 10));
        glowGrad.setColorAt(1, QColor(139, 92, 246, 0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(glowGrad);
        painter.drawEllipse(QPoint(centerX, centerY), glowR, glowR);
    }

    // Background circle
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 20, 35));
    painter.drawEllipse(QPoint(centerX, centerY), radius, radius);

    // Track
    QPen trackPen(QColor(40, 40, 60), 6, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(trackPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(centerX - radius, centerY - radius, radius * 2, radius * 2, 0, 360 * 16);

    if (m_state == Downloading || m_state == Complete) {
        // Progress arc
        QConicalGradient grad(centerX, centerY, 90);
        grad.setColorAt(0, QColor(139, 92, 246));
        grad.setColorAt(0.5, QColor(109, 40, 217));
        grad.setColorAt(1, QColor(139, 92, 246));

        QPen progressPen(grad, 6, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(progressPen);
        int spanAngle = static_cast<int>(m_progress * 360 * 16);
        painter.drawArc(centerX - radius, centerY - radius, radius * 2, radius * 2, 90 * 16, -spanAngle);
    } else if (m_state == Extracting || m_state == Restarting) {
        // Spinning arc
        QConicalGradient spinGrad(centerX, centerY, 90 - m_spinAngle);
        spinGrad.setColorAt(0, QColor(139, 92, 246, 255));
        spinGrad.setColorAt(0.3, QColor(109, 40, 217, 200));
        spinGrad.setColorAt(1, QColor(139, 92, 246, 0));

        QPen spinPen(spinGrad, 6, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(spinPen);
        painter.drawArc(centerX - radius, centerY - radius, radius * 2, radius * 2, 0, 270 * 16);
    }

    // Center content - percentage
    QFont centerFont("Segoe UI", 14, QFont::Bold);
    painter.setFont(centerFont);

    if (m_state == Complete) {
        painter.setPen(QColor(76, 175, 80));
        painter.drawText(QRect(centerX - radius, centerY - 12, radius * 2, 24), Qt::AlignCenter, "✓");
    } else if (m_state == Error) {
        painter.setPen(QColor(244, 67, 54));
        painter.drawText(QRect(centerX - radius, centerY - 12, radius * 2, 24), Qt::AlignCenter, "✕");
    } else {
        painter.setPen(QColor(255, 255, 255));
        QString pctText = QString::number(static_cast<int>(m_progress * 100)) + "%";
        painter.drawText(QRect(centerX - radius, centerY - 12, radius * 2, 24), Qt::AlignCenter, pctText);
    }

    painter.end();
}
