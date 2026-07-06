#include "include/ui/update/UpdateDialog.h"
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <QStyle>
#include <QTimer>
#include <QUrl>
#include <QUuid>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
    , networkManager(new QNetworkAccessManager(this))
    , currentReply(nullptr)
    , tempFile(nullptr)
    , downloadComplete(false)
    , updateApplied(false)
    , bytesReceived(0)
    , bytesTotal(0)
{
    setupUI();
    applyStyle();
    setFixedSize(420, 280);
    setWindowTitle(tr("FreeLink Update"));
}

UpdateDialog::~UpdateDialog()
{
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }
    if (tempFile) {
        tempFile->close();
        tempFile->deleteLater();
    }
}

void UpdateDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Title
    titleLabel = new QLabel(tr("FreeLink Update"));
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Version info
    versionLabel = new QLabel();
    versionLabel->setObjectName("versionLabel");
    versionLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(versionLabel);

    // Circular progress widget
    circularProgress = new CircularProgress(this);
    circularProgress->setFixedSize(160, 160);
    circularProgress->setStatusText(tr("Preparing download..."));

    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->addStretch();
    progressLayout->addWidget(circularProgress);
    progressLayout->addStretch();
    mainLayout->addLayout(progressLayout);

    // Progress details
    QHBoxLayout *detailLayout = new QHBoxLayout();
    progressLabel = new QLabel(tr("0 MB / 0 MB"));
    progressLabel->setObjectName("progressLabel");
    speedLabel = new QLabel(tr("Calculating..."));
    speedLabel->setObjectName("speedLabel");
    detailLayout->addWidget(progressLabel);
    detailLayout->addStretch();
    detailLayout->addWidget(speedLabel);
    mainLayout->addLayout(detailLayout);

    // Buttons
    buttonBox = new QDialogButtonBox();
    buttonBox->setObjectName("buttonBox");

    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setObjectName("cancelButton");
    cancelButton->setMinimumWidth(100);

    restartButton = new QPushButton(tr("Restart Now"));
    restartButton->setObjectName("restartButton");
    restartButton->setMinimumWidth(100);
    restartButton->hide();

    buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);
    buttonBox->addButton(restartButton, QDialogButtonBox::AcceptRole);
    mainLayout->addWidget(buttonBox);

    connect(cancelButton, &QPushButton::clicked, this, &UpdateDialog::onCancelClicked);
    connect(restartButton, &QPushButton::clicked, this, &UpdateDialog::onRestartClicked);
}

void UpdateDialog::applyStyle()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #1a1a2e;
            color: #e0e0e0;
        }
        #titleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #8b5cf6;
            padding: 8px;
        }
        #versionLabel {
            font-size: 13px;
            color: #a0a0a0;
        }
        #progressLabel, #speedLabel {
            font-size: 11px;
            color: #666666;
        }
        QPushButton {
            background-color: #2d2d4a;
            color: #e0e0e0;
            border: 1px solid #3d3d5c;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #3d3d5c;
            border-color: #8b5cf6;
        }
        QPushButton:pressed {
            background-color: #8b5cf6;
        }
        #restartButton {
            background-color: #8b5cf6;
            color: white;
            border: none;
            font-weight: bold;
        }
        #restartButton:hover {
            background-color: #7c3aed;
        }
    )");
}

void UpdateDialog::startUpdate(const QString &url, const QString &version)
{
    downloadUrl = url;
    targetVersion = version;

    QString currentVersion = QString(NKR_VERSION);
    versionLabel->setText(tr("Current: %1 → New: %2").arg(currentVersion, targetVersion));

    // Set initial state
    circularProgress->setState(CircularProgress::Downloading);
    circularProgress->setStatusText(tr("Downloading update"));

    // Create temp file
    tempFile = new QTemporaryFile(QDir::tempPath() + "/FreeLink-Update-XXXXXX.zip", this);
    if (!tempFile->open()) {
        circularProgress->setState(CircularProgress::Error);
        circularProgress->setStatusText(tr("Failed to create temp file"));
        return;
    }

    // Start download
    QNetworkRequest request;
    request.setUrl(QUrl(downloadUrl));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    currentReply = networkManager->get(request);

    connect(currentReply, &QNetworkReply::downloadProgress,
            this, &UpdateDialog::onDownloadProgress);
    connect(currentReply, &QNetworkReply::finished,
            this, &UpdateDialog::onDownloadFinished);

    timer.start();
}

void UpdateDialog::onDownloadProgress(qint64 received, qint64 total)
{
    bytesReceived = received;
    bytesTotal = total;

    if (total > 0) {
        double percent = (received * 100.0) / total;
        circularProgress->setProgress(percent / 100.0);

        double receivedMB = received / (1024.0 * 1024.0);
        double totalMB = total / (1024.0 * 1024.0);
        progressLabel->setText(tr("%1 MB / %2 MB")
            .arg(receivedMB, 0, 'f', 1)
            .arg(totalMB, 0, 'f', 1));

        // Calculate speed
        qint64 elapsed = timer.elapsed();
        if (elapsed > 0) {
            double speed = (received * 1000.0) / elapsed / (1024.0 * 1024.0);
            speedLabel->setText(tr("%1 MB/s").arg(speed, 0, 'f', 1));

            // Calculate ETA
            if (speed > 0) {
                double remaining = (total - received) / (speed * 1024.0 * 1024.0);
                int seconds = static_cast<int>(remaining);
                QString eta;
                if (seconds > 60) {
                    eta = tr("%1 min remaining").arg(seconds / 60);
                } else {
                    eta = tr("%1 sec remaining").arg(seconds);
                }
                circularProgress->setProgressText(tr("%1 MB/s • %2").arg(speed, 0, 'f', 1).arg(eta));
            }
        }
    }
}

void UpdateDialog::onDownloadFinished()
{
    if (!currentReply) return;

    if (currentReply->error() != QNetworkReply::NoError) {
        circularProgress->setState(CircularProgress::Error);
        circularProgress->setStatusText(tr("Download failed"));
        return;
    }

    // Write to temp file
    tempFile->write(currentReply->readAll());
    tempFile->setAutoRemove(false);
    tempFile->close();

    downloadComplete = true;
    circularProgress->setState(CircularProgress::Complete);
    circularProgress->setStatusText(tr("Download complete!"));
    circularProgress->setProgress(1.0);
    progressLabel->setText(tr("%1 MB / %1 MB").arg(bytesTotal / (1024.0 * 1024.0), 0, 'f', 1));
    speedLabel->setText(tr("Complete"));

    // Show restart button
    cancelButton->hide();
    restartButton->show();
    restartButton->setFocus();

    currentReply->deleteLater();
    currentReply = nullptr;
}

void UpdateDialog::onCancelClicked()
{
    if (currentReply) {
        currentReply->abort();
    }
    reject();
}

void UpdateDialog::onRestartClicked()
{
    if (!downloadComplete || !tempFile) {
        QMessageBox::warning(this, tr("Error"), tr("Update not ready"));
        return;
    }

    circularProgress->setState(CircularProgress::Extracting);
    circularProgress->setStatusText(tr("Preparing update"));
    restartButton->setEnabled(false);
    cancelButton->setEnabled(false);

    // Extract and apply update
    extractAndApplyUpdate();
}

void UpdateDialog::extractAndApplyUpdate()
{
    QString appDir = QApplication::applicationDirPath();
    QString zipPath = tempFile->fileName();
    QString extractDir = appDir + "/_update_temp";

    // Create temp extraction directory
    QDir().mkpath(extractDir);

    // Use PowerShell to extract
    QStringList args;
#ifdef Q_OS_WIN
    args << "-Command" << QString(
        "Expand-Archive -Path '%1' -DestinationPath '%2' -Force"
    ).arg(zipPath, extractDir);
#else
    args << "-c" << QString("unzip -o '%1' -d '%2'").arg(zipPath, extractDir);
#endif

    QProcess extractProcess;
#ifdef Q_OS_WIN
    extractProcess.start("powershell", args);
#else
    extractProcess.start("sh", args);
#endif
    extractProcess.waitForFinished(60000);

    if (extractProcess.exitCode() != 0) {
        circularProgress->setState(CircularProgress::Error);
        circularProgress->setStatusText(tr("Extraction failed"));
        restartButton->setEnabled(true);
        cancelButton->setEnabled(true);
        return;
    }

    // Create update script
    createUpdateScript();

    circularProgress->setState(CircularProgress::Restarting);
    circularProgress->setStatusText(tr("Update applied! Restarting"));
    circularProgress->setProgress(1.0);

    // Small delay then restart
    QTimer::singleShot(1500, this, &UpdateDialog::restartApp);
}

void UpdateDialog::createUpdateScript()
{
    // No external script needed - we run commands via QProcess in restartApp()
}

void UpdateDialog::restartApp()
{
#ifdef Q_OS_WIN
    // Run PowerShell commands directly instead of script file
    QString appDir = QApplication::applicationDirPath();
    QString extractDir = appDir + "/_update_temp";

    // Find the subdirectory inside _update_temp (ZIP has FreeLink/ root)
    QString findCmd = QString(
        "Stop-Process -Name FreeLink -Force -ErrorAction SilentlyContinue; "
        "Start-Sleep -Seconds 2; "
        "$src = (Get-ChildItem -Path '%1' -Directory | Select-Object -First 1).FullName; "
        "if (-not $src) { $src = '%1' }; "
        "Copy-Item -Path \"$src\\*\" -Destination '%2' -Recurse -Force; "
        "Remove-Item -Path '%1' -Recurse -Force -ErrorAction SilentlyContinue; "
        "Remove-Item -Path '%2\\_update.zip' -Force -ErrorAction SilentlyContinue; "
        "Start-Process -FilePath '%2\\FreeLink.exe'"
    ).arg(extractDir, appDir);

    QProcess::startDetached("powershell", {"-ExecutionPolicy", "Bypass", "-Command", findCmd});
#else
    QString scriptPath = QApplication::applicationDirPath() + "/_update.sh";
    QProcess::startDetached("sh", {scriptPath});
#endif

    qApp->exit(0);
}
