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
    mainLayout->setSpacing(16);
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

    // Status
    statusLabel = new QLabel(tr("Preparing download..."));
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // Progress bar
    progressBar = new QProgressBar();
    progressBar->setObjectName("progressBar");
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat("%p%");
    mainLayout->addWidget(progressBar);

    // Progress details
    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLabel = new QLabel(tr("0 MB / 0 MB"));
    progressLabel->setObjectName("progressLabel");
    speedLabel = new QLabel(tr("Calculating..."));
    speedLabel->setObjectName("speedLabel");
    progressLayout->addWidget(progressLabel);
    progressLayout->addStretch();
    progressLayout->addWidget(speedLabel);
    mainLayout->addLayout(progressLayout);

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
        #statusLabel {
            font-size: 12px;
            color: #888888;
        }
        QProgressBar {
            border: 2px solid #2d2d4a;
            border-radius: 8px;
            text-align: center;
            background-color: #16162a;
            height: 24px;
            font-size: 11px;
            color: #ffffff;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #8b5cf6, stop:1 #6d28d9);
            border-radius: 6px;
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

    // Create temp file
    tempFile = new QTemporaryFile(QDir::tempPath() + "/FreeLink-Update-XXXXXX.zip", this);
    if (!tempFile->open()) {
        statusLabel->setText(tr("Failed to create temp file"));
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
        int percent = static_cast<int>((received * 100) / total);
        progressBar->setValue(percent);

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
                if (seconds > 60) {
                    statusLabel->setText(tr("Downloading... %1 min remaining").arg(seconds / 60));
                } else {
                    statusLabel->setText(tr("Downloading... %1 sec remaining").arg(seconds));
                }
            }
        }
    }
}

void UpdateDialog::onDownloadFinished()
{
    if (!currentReply) return;

    if (currentReply->error() != QNetworkReply::NoError) {
        statusLabel->setText(tr("Download failed: %1").arg(currentReply->errorString()));
        progressBar->setValue(0);
        return;
    }

    // Write to temp file
    tempFile->write(currentReply->readAll());
    tempFile->close();

    downloadComplete = true;
    statusLabel->setText(tr("Download complete! Ready to update."));
    progressBar->setValue(100);
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

    statusLabel->setText(tr("Preparing update..."));
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

    // Use 7-zip or PowerShell to extract
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
        statusLabel->setText(tr("Extraction failed: %1").arg(extractProcess.readAllStandardError()));
        restartButton->setEnabled(true);
        cancelButton->setEnabled(true);
        return;
    }

    // Create update script
    createUpdateScript();

    statusLabel->setText(tr("Update applied! Restarting..."));

    // Small delay then restart
    QTimer::singleShot(1000, this, &UpdateDialog::restartApp);
}

void UpdateDialog::createUpdateScript()
{
    QString appDir = QApplication::applicationDirPath();
    QString scriptPath;
    QString extractDir = appDir + "/_update_temp";

#ifdef Q_OS_WIN
    scriptPath = appDir + "/_update.bat";
    QFile script(scriptPath);
    if (script.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Write UTF-8 BOM for Windows batch Unicode support
        script.write("\xEF\xBB\xBF");
        QTextStream out(&script);
        out.setEncoding(QStringConverter::Utf8);
        out << "@echo off\n";
        out << "chcp 65001 >nul\n";
        out << "timeout /t 2 /nobreak > nul\n";
        out << "taskkill /f /im FreeLink.exe > nul 2>&1\n";
        out << "timeout /t 1 /nobreak > nul\n";

        // Copy new files
        out << "xcopy /E /Y /I \"" << extractDir << "\\*\" \"" << appDir << "\"\n";

        // Cleanup
        out << "rmdir /S /Q \"" << extractDir << "\"\n";
        out << "del /Q \"" << appDir << "\\_update.zip\" 2>nul\n";

        // Restart app
        out << "start \"\" \"" << appDir << "\\FreeLink.exe\"\n";

        // Delete self (delayed)
        out << "del /Q \"" << appDir << "\\_update.bat\" 2>nul\n";

        script.close();
    }
#else
    scriptPath = appDir + "/_update.sh";
    QFile script(scriptPath);
    if (script.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&script);
        out << "#!/bin/sh\n";
        out << "sleep 2\n";
        out << "killall FreeLink 2>/dev/null\n";
        out << "sleep 1\n";
        out << "cp -r \"" << extractDir << "/\"* \"" << appDir << "/\"\n";
        out << "rm -rf \"" << extractDir << "\"\n";
        out << "rm -f \"" << appDir << "/_update.zip\"\n";
        out << "\"$appDir/FreeLink\" &\n";
        out << "rm -f \"" << scriptPath << "\"\n";
        script.close();
        script.setPermissions(QFile::ExeOwner | QFile::ReadOwner | QFile::WriteOwner);
    }
#endif
}

void UpdateDialog::restartApp()
{
    QString appDir = QApplication::applicationDirPath();
    QString scriptPath;

#ifdef Q_OS_WIN
    scriptPath = appDir + "/_update.bat";
    QProcess::startDetached("cmd", {"/c", scriptPath});
#else
    scriptPath = appDir + "/_update.sh";
    QProcess::startDetached("sh", {scriptPath});
#endif

    qApp->exit(0);
}
