#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

class FolderCustomizerUpdater : public QObject {
    Q_OBJECT

   private:
    const QString appName = "Folder Customizer";
    const QString appVersion = "0.0.8";
    const QString remoteManifestUrl =
        "https://raw.githubusercontent.com/Deadbush225/Folder-Customizer/main/"
        "manifest.json";
    const QString latestInstallerUrl =
        "https://github.com/Deadbush225/Folder-Customizer/releases/latest/"
        "download/FolderCustomizerSetup-x64.exe";

    QNetworkAccessManager networkManager;

   public:
    FolderCustomizerUpdater(QObject* parent = nullptr) : QObject(parent) {}

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

            // Download installer
            QString installerPath = downloadFile(latestInstallerUrl);
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

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    FolderCustomizerUpdater updater;
    updater.checkForUpdates();

    return 0;  // Not using app.exec() as we don't need event loop after update
               // check
}
