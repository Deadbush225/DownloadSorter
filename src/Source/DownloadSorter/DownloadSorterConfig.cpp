#include "DownloadSorter/DownloadSorterConfig.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

static QString configFilePath() {
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(base);
    dir.mkpath("DownloadSorter");
    dir.cd("DownloadSorter");
    return dir.filePath("mappings.json");
}

QMap<QString, QList<QString>> DownloadSorterConfig::loadMappings() {
    QMap<QString, QList<QString>> map;
    QFile f(configFilePath());
    if (!f.exists())
        return map;
    if (!f.open(QIODevice::ReadOnly))
        return map;

    const auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isArray())
        return map;

    const auto arr = doc.array();
    for (const auto& v : arr) {
        const auto obj = v.toObject();
        const QString folder = obj.value("folder").toString();
        const auto extsArr = obj.value("extensions").toArray();
        QList<QString> exts;
        exts.reserve(extsArr.size());
        for (const auto& ev : extsArr)
            exts.push_back(ev.toString().toLower());
        if (!folder.isEmpty() && !exts.isEmpty())
            map.insert(folder, exts);
    }
    return map;
}

bool DownloadSorterConfig::saveMappings(
    const QMap<QString, QList<QString>>& map) {
    QJsonArray arr;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QJsonObject obj;
        obj.insert("folder", it.key());
        QJsonArray exts;
        for (const auto& e : it.value())
            exts.push_back(e.toLower());
        obj.insert("extensions", exts);
        arr.push_back(obj);
    }
    QJsonDocument doc(arr);
    QFile f(configFilePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const auto bytes = doc.toJson(QJsonDocument::Indented);
    const bool ok = f.write(bytes) == bytes.size();
    f.close();
    return ok;
}
