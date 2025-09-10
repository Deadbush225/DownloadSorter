#pragma once
#include <QtCore/QJsonArray>
#include <QtCore/QObject>
#include <QtNetwork/QNetworkAccessManager>

class eUpdater : public QObject {
    Q_OBJECT

   public:
    explicit eUpdater(QObject* parent = nullptr);
    void checkForUpdates(const QString& manifestUrl,
                         const QString& releaseApiUrl,
                         const QString& installerTemplate,
                         const QString& packageName = QString());

   private:
    QString downloadFile(const QString& url);
    int compareVersions(const QString& localVersion,
                        const QString& remoteVersion);
    QString getJsonValue(const QString& json, const QString& key);
    QString chooseAssetUrl(const QJsonArray& assets,
                           const QString& packageName);
    QString detectLinuxFamily();
    bool nameMatches(const QString& name, const QString& pattern);
    void runInstaller(const QString& path);

    QString m_packageName;

    QNetworkAccessManager networkManager;
};
