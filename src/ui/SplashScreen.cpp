#include "include/ui/SplashScreen.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QLinearGradient>
#include <QFont>
#include <QFontMetrics>

SplashScreen *SplashScreen::s_instance = nullptr;

SplashScreen::SplashScreen(const QPixmap &pixmap)
    : QSplashScreen(pixmap.isNull() ? QPixmap(400, 300) : pixmap)
{
    s_instance = this;

    setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Center on screen
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        move((screenGeometry.width() - width()) / 2,
             (screenGeometry.height() - height()) / 2);
    }

    // Dots animation timer
    m_dotsTimer = new QTimer(this);
    connect(m_dotsTimer, &QTimer::timeout, this, [this]() {
        m_dotsCount = (m_dotsCount + 1) % 4;
        update();
    });
    m_dotsTimer->start(400);
}

SplashScreen::~SplashScreen()
{
    s_instance = nullptr;
}

void SplashScreen::setProgress(qreal value)
{
    m_progress = qBound(0.0, value, 1.0);
    update();
}

void SplashScreen::showStatus(const QString &message)
{
    m_statusMessage = message;
    m_dotsCount = 0;
    update();
    QApplication::processEvents();
}

void SplashScreen::fadeOut()
{
    m_dotsTimer->stop();

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(this, "windowOpacity");
    fadeAnim->setDuration(300);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    connect(fadeAnim, &QPropertyAnimation::finished, this, &SplashScreen::close);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(15, 15, 26, 240));
    painter.drawRoundedRect(rect(), 12, 12);

    // Border
    painter.setPen(QPen(QColor(139, 92, 246, 100), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12);

    int w = width();
    int h = height();
    int centerX = w / 2;

    // Logo (from resource)
    QPixmap logo(":/FreeLink/FreeLink.png");
    if (!logo.isNull()) {
        int logoSize = 64;
        logo = logo.scaled(logoSize, logoSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(centerX - logoSize / 2, 40, logo);
    }

    // Title
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor(255, 255, 255));
    painter.drawText(QRect(0, 115, w, 30), Qt::AlignCenter, "FreeLink");

    // Subtitle
    QFont subFont("Segoe UI", 9);
    painter.setFont(subFont);
    painter.setPen(QColor(160, 160, 180));
    painter.drawText(QRect(0, 145, w, 20), Qt::AlignCenter, "Internet Freedom Client");

    // Progress bar background
    int barX = 60;
    int barW = w - 120;
    int barY = 190;
    int barH = 6;
    QRect barRect(barX, barY, barW, barH);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 30, 50));
    painter.drawRoundedRect(barRect, 3, 3);

    // Progress bar fill
    if (m_progress > 0) {
        QLinearGradient grad(barX, barY, barX + barW, barY);
        grad.setColorAt(0, QColor(139, 92, 246));
        grad.setColorAt(1, QColor(109, 40, 217));
        painter.setBrush(grad);
        QRect fillRect(barX, barY, static_cast<int>(barW * m_progress), barH);
        painter.drawRoundedRect(fillRect, 3, 3);
    }

    // Percentage text
    QFont pctFont("Segoe UI", 8);
    painter.setFont(pctFont);
    painter.setPen(QColor(139, 92, 246));
    QString pctText = QString::number(static_cast<int>(m_progress * 100)) + "%";
    painter.drawText(QRect(barX, barY + 12, barW, 16), Qt::AlignCenter, pctText);

    // Status message with dots
    if (!m_statusMessage.isEmpty()) {
        QFont statusFont("Segoe UI", 9);
        painter.setFont(statusFont);
        painter.setPen(QColor(160, 160, 180));
        QString dots;
        for (int i = 0; i < m_dotsCount; i++) dots += ".";
        painter.drawText(QRect(0, h - 50, w, 20), Qt::AlignCenter, m_statusMessage + dots);
    }

    painter.end();
}
