#include "updater.h"
#include <QtCore/QDir>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtCore/QFileDevice>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>
#include <QtCore/QSysInfo>
#include <QtCore/QVersionNumber>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

eUpdater::eUpdater(QObject* parent) : QObject(parent) {}

void eUpdater::checkForUpdates(const QString& manifestUrl,
                               const QString& releaseApiUrl,
                               const QString& installerTemplate,
                               const QString& packageName) {
    // Store package name, with fallback
    m_packageName = packageName;
    if (m_packageName.isEmpty()) {
        m_packageName = "download-sorter";  // Default fallback
    }
    if (manifestUrl.isEmpty() && releaseApiUrl.isEmpty()) {
        QMessageBox::critical(nullptr, "Updater Error",
                              "No update source provided. Pass --manifest-url "
                              "or --release-api-url.");
        return;
    }

    QString remoteVersion;
    QString apiResponse;

    // Get remote version from manifest or GitHub API
    if (!manifestUrl.isEmpty()) {
        QString remoteManifestPath = downloadFile(manifestUrl);
        if (remoteManifestPath.isEmpty()) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Failed to download manifest file.");
            return;
        }
        QFile remoteManifestFile(remoteManifestPath);
        QString remoteManifest;
        if (remoteManifestFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            remoteManifest = remoteManifestFile.readAll();
            remoteManifestFile.close();
        }
        remoteVersion = getJsonValue(remoteManifest, "version");
    }

    // Always get GitHub API data for asset selection
    QString defaultApiUrl = releaseApiUrl;
    if (defaultApiUrl.isEmpty()) {
        defaultApiUrl =
            "https://api.github.com/repos/Deadbush225/DownloadSorter/releases/"
            "latest";
    }

    QString apiResponsePath = downloadFile(defaultApiUrl);
    if (apiResponsePath.isEmpty()) {
        QMessageBox::critical(nullptr, "Update Error",
                              "Failed to query release API.");
        return;
    }
    QFile apiFile(apiResponsePath);
    if (apiFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        apiResponse = apiFile.readAll();
        apiFile.close();
    }

    // If we didn't get version from manifest, get it from API
    if (remoteVersion.isEmpty()) {
        remoteVersion = getJsonValue(apiResponse, "tag_name");
        if (remoteVersion.startsWith('v')) {
            remoteVersion.remove(0, 1);
        }
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

    // Extract local version
    QString localVersion = getJsonValue(localManifest, "version");

    // Compare versions
    if (compareVersions(localVersion, remoteVersion) < 0) {
        if (QMessageBox::question(nullptr, "Update Available",
                                  "Update to " + remoteVersion + " now?") !=
            QMessageBox::Yes) {
            return;
        }

        // Parse API response to get assets
        QJsonDocument doc = QJsonDocument::fromJson(apiResponse.toUtf8());
        if (!doc.isObject()) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Invalid API response");
            return;
        }

        QJsonArray assets = doc.object().value("assets").toArray();
        QString assetUrl = chooseAssetUrl(assets, m_packageName);

        if (assetUrl.isEmpty()) {
            QMessageBox::critical(
                nullptr, "Update Error",
                "No suitable installer found for your platform\nDetected: " +
                    detectLinuxFamily());
            return;
        }

        // Download installer
        QString installerPath = downloadFile(assetUrl);
        if (!installerPath.isEmpty()) {
            QMessageBox::information(
                nullptr, "Debug",
                "Downloaded: " + QFileInfo(installerPath).fileName() +
                    "\nPath: " + installerPath);
            runInstaller(installerPath);
        } else {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Failed to download the installer.");
        }
    } else {
        QMessageBox::information(
            nullptr, "Up to Date",
            "You already have the latest version: " + localVersion);
    }
}

QString eUpdater::downloadFile(const QString& url) {
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "eUpdater/1.0");
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
    if (fileName.isEmpty())
        fileName = "download.bin";

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

int eUpdater::compareVersions(const QString& localVersion,
                              const QString& remoteVersion) {
    const QVersionNumber lv = QVersionNumber::fromString(localVersion);
    const QVersionNumber rv = QVersionNumber::fromString(remoteVersion);
    return QVersionNumber::compare(lv, rv);
}

QString eUpdater::getJsonValue(const QString& json, const QString& key) {
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains(key)) {
            return obj.value(key).toString();
        }
    }
    return "";
}

QString eUpdater::detectLinuxFamily() {
#if defined(Q_OS_LINUX)
    QFile f("/etc/os-release");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString data = QString::fromUtf8(f.readAll()).toLower();
        if (data.contains("debian") || data.contains("ubuntu"))
            return "deb";
        if (data.contains("arch") || data.contains("manjaro"))
            return "arch";
        if (data.contains("fedora") || data.contains("rhel") ||
            data.contains("centos") || data.contains("suse"))
            return "rpm";
    }
#endif
    return "unknown";
}

bool eUpdater::nameMatches(const QString& name, const QString& pattern) {
    return QRegularExpression(pattern,
                              QRegularExpression::CaseInsensitiveOption)
        .match(name)
        .hasMatch();
}

QString eUpdater::chooseAssetUrl(const QJsonArray& assets,
                                 const QString& packageName) {
    // Create flexible patterns based on package name
    QString pkgName = packageName.toLower();
    QString pkgNameCamel = packageName;  // Keep original case for some patterns
#ifdef Q_OS_WIN
    // Windows: look for .exe installer
    QStringList winPatterns = {
        QString(R"(%1.*\.exe$)").arg(pkgName),
        QString(R"(%1.*\.exe$)").arg(pkgNameCamel), R"(.*setup.*\.exe$)",
        R"(.*installer.*\.exe$)",
        R"(.*\.exe$)"  // Last resort: any .exe
    };

    for (const auto& pat : winPatterns) {
        for (const auto& v : assets) {
            const QJsonObject a = v.toObject();
            const QString name = a.value("name").toString();
            if (nameMatches(name, pat)) {
                return a.value("browser_download_url").toString();
            }
        }
    }
    return {};
#elif defined(Q_OS_LINUX)
    const QString fam = detectLinuxFamily();
    QStringList patterns;

    // Create architecture-flexible patterns (with and without arch suffix)
    QString archSuffix = R"((?:[-_](?:x86_64|amd64|64bit|\d))?)";

    if (fam == "deb") {
        patterns << QString(R"(%1.*%2\.deb$)").arg(pkgName, archSuffix);
    } else if (fam == "rpm") {
        patterns << QString(R"(%1.*%2\.rpm$)").arg(pkgName, archSuffix);
    } else if (fam == "arch") {
        // For Arch/Manjaro, prefer .tar.gz over .pkg.tar.gz
        patterns << QString(R"(%1.*%2\.tar\.gz$)").arg(pkgName, archSuffix);
        patterns << QString(R"(%1.*%2\.pkg\.tar\.(?:zst|gz)$)")
                        .arg(pkgName, archSuffix);
    }

    // Universal fallbacks (order matters - most preferred first)
    patterns << QString(R"(%1.*%2\.tar\.gz$)").arg(pkgName, archSuffix);
    patterns << QString(R"(%1.*%2\.AppImage$)").arg(pkgNameCamel, archSuffix);
    patterns << QString(R"(%1.*%2\.tar\.xz$)").arg(pkgName, archSuffix);
    patterns << QString(R"(%1.*%2\.zip$)").arg(pkgName, archSuffix);

    // Debug output
    qDebug() << "Looking for assets matching package:" << packageName;
    qDebug() << "Detected Linux family:" << fam;
    qDebug() << "Using patterns:" << patterns;

    for (const auto& pat : patterns) {
        for (const auto& v : assets) {
            const QJsonObject a = v.toObject();
            const QString name = a.value("name").toString();
            qDebug() << "Checking asset:" << name << "against pattern:" << pat;
            if (nameMatches(name, pat)) {
                qDebug() << "MATCH FOUND:" << name;
                return a.value("browser_download_url").toString();
            }
        }
    }

    qDebug() << "No matching assets found";
    return {};
#else
    return {};
#endif
}

void eUpdater::runInstaller(const QString& path) {
#ifdef Q_OS_WIN
    if (!QProcess::startDetached(path, {})) {
        QMessageBox::critical(nullptr, "Update Error",
                              "Failed to run installer");
    }
#elif defined(Q_OS_LINUX)
    const QString lower = path.toLower();
    QString cmd;
    QStringList args;

    if (lower.endsWith(".deb")) {
        cmd = "pkexec";
        args << "dpkg" << "-i" << path;
    } else if (lower.endsWith(".rpm")) {
        cmd = "pkexec";
        args << "rpm" << "-Uvh" << path;
    } else if (lower.contains(".pkg.tar")) {
        cmd = "pkexec";
        args << "pacman" << "-U" << "--noconfirm" << path;
        QMessageBox::information(
            nullptr, "Debug",
            "Installing Arch package with: " + cmd + " " + args.join(" "));
    } else if (lower.endsWith(".tar.gz")) {
        // Extract and run install.sh with pkexec
        const QString outDir = QDir::tempPath() + "/ds-install";
        QDir().mkpath(outDir);

        // First extract the archive
        QProcess extractProc;
        extractProc.start("tar", {"xzf", path, "-C", outDir});
        if (!extractProc.waitForFinished(30000) ||
            extractProc.exitCode() != 0) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Failed to extract archive: " +
                                      extractProc.readAllStandardError());
            return;
        }

        // Look for install.sh
        QString installScript = outDir + "/install.sh";
        if (!QFileInfo::exists(installScript)) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "No install.sh found in archive");
            return;
        }

        // Make executable and run with pkexec
        QFile(installScript)
            .setPermissions(QFile::permissions(installScript) |
                            QFileDevice::ExeOwner);

        QProcess installProc;
        installProc.start("pkexec", {"bash", installScript});
        if (!installProc.waitForStarted(3000)) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Failed to start installer");
            return;
        }

        if (!installProc.waitForFinished(60000)) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Installation timed out");
            return;
        }

        if (installProc.exitCode() == 0) {
            QMessageBox::information(nullptr, "Update Complete",
                                     "Installation finished");
        } else {
            QString stderr = installProc.readAllStandardError();
            QString stdout = installProc.readAllStandardOutput();
            QMessageBox::critical(nullptr, "Update Error",
                                  "Installation failed with code " +
                                      QString::number(installProc.exitCode()) +
                                      "\nStdout: " + stdout +
                                      "\nStderr: " + stderr);
        }
        return;
    } else if (lower.endsWith(".appimage")) {
        // Make AppImage executable and run it
        QFile(path).setPermissions(
            QFile::permissions(path) | QFileDevice::ExeOwner |
            QFileDevice::ExeGroup | QFileDevice::ExeOther);
        if (!QProcess::startDetached(path, {})) {
            QMessageBox::critical(nullptr, "Update Error",
                                  "Failed to run AppImage");
            return;
        }
        QMessageBox::information(nullptr, "Update Complete",
                                 "AppImage launched");
        return;
    } else {
        QMessageBox::critical(nullptr, "Update Error",
                              "Unknown installer format");
        return;
    }

    // For deb/rpm/pkg installations
    QProcess proc;
    proc.start(cmd, args);
    if (!proc.waitForStarted(3000)) {
        QMessageBox::critical(nullptr, "Update Error",
                              "Failed to start installer");
        return;
    }

    if (!proc.waitForFinished(60000)) {
        QMessageBox::critical(nullptr, "Update Error",
                              "Installation timed out");
        return;
    }

    if (proc.exitCode() == 0) {
        QMessageBox::information(nullptr, "Update Complete",
                                 "Installation finished");
    } else {
        QString stderr = proc.readAllStandardError();
        QString stdout = proc.readAllStandardOutput();
        QMessageBox::critical(
            nullptr, "Update Error",
            "Installer exited with code " + QString::number(proc.exitCode()) +
                "\nStdout: " + stdout + "\nStderr: " + stderr);
    }
#else
    Q_UNUSED(path);
    QMessageBox::critical(nullptr, "Update Error", "Unsupported OS");
#endif
}
