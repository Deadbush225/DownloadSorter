#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QList>
#include <QMap>
#include <QString>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>

#include "DownloadSorter.h"  // for SettingsData

class SettingsManager {
   public:
    // Returns path to JSON settings and ensures directory exists
    static QString configPath() {
        const QString base =
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        QDir dir(base);
        dir.mkpath("DownloadSorter");
        return dir.filePath("DownloadSorter/mappings.json");
    }

    // Default seed for first-run or corrupted/missing files
    static SettingsData defaults() {
        SettingsData d;
        d.mappings[QStringLiteral("Downloaded Archives")] = {"zip", "rar",
                                                             "7z"};
        d.mappings[QStringLiteral("Downloaded Audios")] = {"m4a", "mp3", "wav",
                                                           "aac"};
        d.mappings[QStringLiteral("Downloaded Documents")] = {
            "doc", "docx", "xls", "xlsx", "ppt", "pptx", "odt", "pdf", "rtf"};
        d.mappings[QStringLiteral("Downloaded Fonts")] = {"otf", "ttf"};
        d.mappings[QStringLiteral("Downloaded Images")] = {
            "png", "jpeg", "jpg", "gif", "tiff", "psd", "ai", "eps"};
        d.mappings[QStringLiteral("Downloaded Programs")] = {"exe", "msi"};
        d.mappings[QStringLiteral("Downloaded Videos")] = {"mp4", "mov", "wmv",
                                                           "avi", "mkv"};
        // ignorePatterns empty by default
        return d;
    }

    // Read settings; if missing/invalid/empty, seed defaults and write them
    static SettingsData read() {
        QFile f(configPath());
        if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
            auto def = defaults();
            write(def);
            return def;
        }
        const auto doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        if (!doc.isObject()) {
            auto def = defaults();
            write(def);
            return def;
        }
        SettingsData data;
        const auto obj = doc.object();

        // mappings
        const auto mappingsArr =
            obj.value(QStringLiteral("mappings")).toArray();
        for (const auto& v : mappingsArr) {
            const auto o = v.toObject();
            const QString folder = o.value(QStringLiteral("folder")).toString();
            QList<QString> exts;
            for (const auto& ev :
                 o.value(QStringLiteral("extensions")).toArray()) {
                const auto e = ev.toString().toLower();
                if (!e.isEmpty() && !exts.contains(e))
                    exts.push_back(e);
            }
            if (!folder.isEmpty() && !exts.isEmpty())
                data.mappings.insert(folder, exts);
        }

        // ignore patterns
        const auto ignoreArr =
            obj.value(QStringLiteral("ignorePatterns")).toArray();
        for (const auto& v : ignoreArr)
            data.ignorePatterns.append(v.toString());

        // seed if mappings empty
        if (data.mappings.isEmpty()) {
            data = defaults();
            write(data);
        }
        return data;
    }

    // Persist settings
    static bool write(const SettingsData& data) {
        QJsonObject obj;

        // mappings
        QJsonArray mappingsArr;
        for (auto it = data.mappings.constBegin();
             it != data.mappings.constEnd(); ++it) {
            QJsonObject o;
            o.insert(QStringLiteral("folder"), it.key());
            QJsonArray exts;
            for (const auto& e : it.value())
                exts.append(e.toLower());
            o.insert(QStringLiteral("extensions"), exts);
            mappingsArr.append(o);
        }
        obj.insert(QStringLiteral("mappings"), mappingsArr);

        // ignore patterns
        QJsonArray ignoreArr;
        for (const auto& p : data.ignorePatterns)
            ignoreArr.append(p);
        obj.insert(QStringLiteral("ignorePatterns"), ignoreArr);

        QFile f(configPath());
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return false;
        const auto bytes = QJsonDocument(obj).toJson(QJsonDocument::Indented);
        const bool ok = f.write(bytes) == bytes.size();
        f.close();
        return ok;
    }
};

#endif  // SETTINGS_MANAGER_H
