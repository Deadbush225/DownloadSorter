#ifndef DOWNLOADSORTER_H
#define DOWNLOADSORTER_H

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QThread>

// learn to create and import the library (fmt)

// #include <fmt/core.h>
// #include "fmt/format.h"
// #include "fmt/format-inl.h"

#include <iostream>
#include <string>

#include <filesystem>

// Unified settings struct
struct SettingsData {
    QMap<QString, QList<QString>> mappings;
    QList<QString> ignorePatterns;
};

class DownloadSorter : public QThread {
    Q_OBJECT

   public:
    // Accept const reference to QString for flexibility
    DownloadSorter(const QString& path);

    void run();

    // Add: configure mappings at runtime
    void setFileTypesMap(const QMap<QString, QList<QString>>& map) {
        fileTypesMap = map;
    }
    const QMap<QString, QList<QString>>& getFileTypesMap() const {
        return fileTypesMap;
    }

    // New: accept ignore patterns (regex strings), compile and store
    void setIgnorePatterns(const QList<QString>& patterns) {
        ignorePatterns.clear();
        for (const QString& p : patterns) {
            const QString trimmed = p.trimmed();
            if (trimmed.isEmpty())
                continue;
            QRegularExpression re(trimmed);
            if (re.isValid())
                ignorePatterns.append(re);
            // invalid regexes are skipped silently; could log if needed
        }
    }

    // Optional helper used by sorter code to check whether a name should be
    // ignored
    bool isIgnored(const QString& name) const {
        for (const auto& re : ignorePatterns) {
            if (re.isValid() && re.match(name).hasMatch())
                return true;
        }
        return false;
    }

   signals:
    // Progress bar and status signals
    void progressRangeChanged(int minimum, int maximum);
    void progressValueChanged(int value);
    void statusMessage(const QString& message);

   private:
    QDir downloadFolder;
    QList<QFileInfo> contents;

    QList<QString> blacklist = {"Downloaded Archives",  "Downloaded Audios",
                                "Downloaded Documents", "Downloaded Fonts",
                                "Downloaded Images",    "Downloaded Programs",
                                "Downloaded Videos",    "Downloaded Folders"};

    QMap<QString, QList<QString>> fileTypesMap;
    // QMap<QFileInfo, QString> filesPerCategory;

    // added member to store compiled ignore regexes
    QList<QRegularExpression> ignorePatterns;

    void recalculateContents();
    QMap<QString, QString> evaluateCategory();
    int moveContents(QMap<QString, QString> contents);
    QString suffixToFolder(QFileInfo content);

    void createFoldersIfDoesntExist();
};

#endif  // DOWNLOADSORTER_H