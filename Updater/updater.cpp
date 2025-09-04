#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtGui/QPalette>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

class DownloadSorterUpdater : public QObject {
    Q_OBJECT

   private:
    const QString appName = "Download Sorter";
    const QString remoteManifestUrl =
        "https://raw.githubusercontent.com/Deadbush225/DownloadSorter/main/"
        "manifest.json";
    // Use the GitHub API to get the latest release info
    const QString latestReleaseApiUrl =
        "https://api.github.com/repos/Deadbush225/DownloadSorter/releases/"
        "latest";

    QNetworkAccessManager networkManager;

   public:
    DownloadSorterUpdater(QObject* parent = nullptr) : QObject(parent) {}

    void checkForUpdates() {
        // Download remote manifest
        QString remoteManifestPath = downloadFile(remoteManifestUrl);
        if (remoteManifestPath.isEmpty()) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Failed to download manifest file.");
            return;
        }

        // Read local manifest
        QString localManifestPath =
            QCoreApplication::applicationDirPath() + "/manifest.json";
        QString localManifest;

        if (QFileInfo::exists(localManifestPath)) {
            QFile file(localManifestPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                localManifest = file.readAll();
                file.close();
            }
        }

        if (localManifest.isEmpty()) {
            localManifest = "{\"version\": \"0.0.0\"}";
        }

        // Read remote manifest
        QFile remoteManifestFile(remoteManifestPath);
        QString remoteManifest;
        if (remoteManifestFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            remoteManifest = remoteManifestFile.readAll();
            remoteManifestFile.close();
        }

        // Extract versions
        QString localVersion = getJsonValue(localManifest, "version");
        QString remoteVersion = getJsonValue(remoteManifest, "version");

        // Compare versions
        if (compareVersions(localVersion, remoteVersion) < 0) {
            QMessageBox::information(
                nullptr, "Update Available",
                "A new version is available: " + remoteVersion);

            // Build installer URL based on manifest version
            QString installerUrl =
                QString(
                    "https://github.com/Deadbush225/DownloadSorter/releases/"
                    "download/%1/download-sorter-%1.exe")
                    .arg(remoteVersion);

            // Download installer
            QString installerPath = downloadFile(installerUrl);
            if (!installerPath.isEmpty()) {
                // Run installer
                QProcess process;
                bool success =
                    process.startDetached(installerPath, QStringList());

                if (!success) {
                    QMessageBox::critical(nullptr, "Update Error",
                                          "Failed to run the installer.");
                }
            } else {
                QMessageBox::critical(
                    nullptr, "Update Error",
                    "Failed to download the latest installer.");
            }
        } else {
            QMessageBox::information(
                nullptr, "Up to Date",
                "You already have the latest version: " + localVersion);
        }
    }

   private:
    QString downloadFile(const QString& url) {
        QNetworkRequest request(url);
        QNetworkReply* reply = networkManager.get(request);

        // Use event loop to make the request synchronous
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            return "";
        }

        // Get file name from URL
        QFileInfo fileInfo(url);
        QString fileName = fileInfo.fileName();

        // Save to temp directory
        QString tempPath = QDir::tempPath() + "/" + fileName;
        QFile file(tempPath);

        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            reply->deleteLater();
            return tempPath;
        }

        reply->deleteLater();
        return "";
    }

    int compareVersions(const QString& localVersion,
                        const QString& remoteVersion) {
        return localVersion.compare(remoteVersion);
    }

    QString getJsonValue(const QString& json, const QString& key) {
        QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains(key)) {
                return obj.value(key).toString();
            }
        }
        return "";
    }
};

#include "updater.moc"

static void setDarkTheme() {
    QApplication::setStyle("Fusion");

    QPalette dark_palette;
    QColor baseColor(31, 31, 31);
    QColor textColor(Qt::white);
    QColor highlightColor(0, 136, 57);
    QColor disabledTextColor(Qt::darkGray);

    dark_palette.setColor(QPalette::Window, baseColor);
    dark_palette.setColor(QPalette::WindowText, textColor);
    dark_palette.setColor(QPalette::Base, baseColor.darker(160));
    dark_palette.setColor(QPalette::AlternateBase, baseColor);
    dark_palette.setColor(QPalette::ToolTipBase, baseColor.darker(120));
    dark_palette.setColor(QPalette::ToolTipText, textColor);
    dark_palette.setColor(QPalette::Text, textColor);
    dark_palette.setColor(QPalette::Button, baseColor);
    dark_palette.setColor(QPalette::ButtonText, textColor);
    dark_palette.setColor(QPalette::BrightText, Qt::red);
    dark_palette.setColor(QPalette::Link, highlightColor);
    dark_palette.setColor(QPalette::Highlight, highlightColor);
    dark_palette.setColor(QPalette::HighlightedText, Qt::white);
    dark_palette.setColor(QPalette::Active, QPalette::Button, baseColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::ButtonText,
                          disabledTextColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::WindowText,
                          disabledTextColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::Text,
                          disabledTextColor);
    dark_palette.setColor(QPalette::Disabled, QPalette::Light, baseColor);
    QApplication::setPalette(dark_palette);

    qApp->setStyleSheet(R"(
        QGroupBox { border: 1px solid #2f2f2f; border-radius: 3px; margin-top: 0.6em; padding: 0.3em; }
        QGroupBox::title { subcontrol-origin: margin; margin-left: 0em; }
        QFrame[frameShape="4"] { border: none; border-top: 1px solid #2f2f2f; background: #2f2f2f; margin: 0.5em 0; }
        QMenuBar { background-color: #262626; color: #dddddd; }
        QMenuBar::item { background: transparent; padding: 4px 10px; }
        QMenuBar::item:selected, QMenuBar::item:pressed { background: rgb(0, 136, 57); color: #ffffff; }
        QMenuBar::item:disabled { color: #666666; }
        QMenu { background-color: #262626; color: #dddddd; border: 1px solid #2f2f2f; }
        QMenu::separator { height: 1px; background: #2f2f2f; margin: 4px 8px; }
        QMenu::item { background: transparent; padding: 6px 18px; }
        QMenu::item:selected { background: rgb(0, 136, 57); color: #ffffff; }
        QMenu::item:disabled { color: #666666; }
        QLineEdit { background: #2a2a2a; color: #ffffff; selection-background-color: rgb(0, 136, 57); selection-color: #ffffff; border: 1px solid #3a3a3a; border-radius: 3px; }
    )");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Match main app theme and icon
    setDarkTheme();
    app.setWindowIcon(QIcon(":/appicon"));

    DownloadSorterUpdater updater;
    updater.checkForUpdates();

    return 0;
}
