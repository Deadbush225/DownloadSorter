#ifndef DOWNLOADSORTER_H
#define DOWNLOADSORTER_H

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QThread>
// #include <QtCore/QFileInfoList>

// learn to create and import the library (fmt)

// #include <fmt/core.h>
// #include "fmt/format.h"
// #include "fmt/format-inl.h"

#include <iostream>
#include <string>

#include <filesystem>

#include <windows.h>

class DownloadSorter : public QThread {
    Q_OBJECT

    //    signals:
    //     void fine(QString message);

   public:
    DownloadSorter(QString& path);

    void run();

    // Add: configure mappings at runtime
    void setFileTypesMap(const QMap<QString, QList<QString>>& map) {
        fileTypesMap = map;
    }
    const QMap<QString, QList<QString>>& getFileTypesMap() const {
        return fileTypesMap;
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
                                "Downloaded Videos",    "IDM Roaming",
                                "Telegram Desktop",     "Tixati",
                                "Download Folders"};

    QMap<QString, QList<QString>> fileTypesMap;
    // QMap<QFileInfo, QString> filesPerCategory;

    void recalculateContents();
    QMap<QString, QString> evaluateCategory();
    int moveContents(QMap<QString, QString> contents);
    QString suffixToFolder(QFileInfo content);

    void createFoldersIfDoesntExist();
};

#endif