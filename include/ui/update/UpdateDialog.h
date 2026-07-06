#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMovie>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTemporaryFile>
#include <QProcess>
#include <QElapsedTimer>
#include <QTimer>

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();

    void startUpdate(const QString &downloadUrl, const QString &version);

signals:
    void updateFinished(bool success);
    void updateProgress(int percent, const QString &status);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onCancelClicked();
    void onRestartClicked();

private:
    void setupUI();
    void applyStyle();
    void extractAndApplyUpdate();
    void createUpdateScript();
    void restartApp();

    // UI Elements
    QLabel *titleLabel;
    QLabel *versionLabel;
    QLabel *statusLabel;
    QLabel *progressLabel;
    QLabel *speedLabel;
    QProgressBar *progressBar;
    QPushButton *cancelButton;
    QPushButton *restartButton;
    QDialogButtonBox *buttonBox;

    // Network
    QNetworkAccessManager *networkManager;
    QNetworkReply *currentReply;
    QTemporaryFile *tempFile;

    // State
    QString downloadUrl;
    QString targetVersion;
    bool downloadComplete;
    bool updateApplied;
    qint64 bytesReceived;
    qint64 bytesTotal;
    QElapsedTimer timer;
};
