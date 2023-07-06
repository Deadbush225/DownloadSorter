#include "DownloadSorter.h"

// QMap<QString, QList<QString>> fileTypesMap = {};
// QMap<QString, int> t = ;

DownloadSorter::DownloadSorter(QString& path) {
    qDebug() << "Creating an instance";
    this->fileTypesMap["Downloaded Archives"] = {"zip", "rar", "7z"};
    this->fileTypesMap["Downloaded Audios"] = {"m4a", "mp3", "wav", "aac"};
    this->fileTypesMap["Downloaded Documents"] = {
        "doc", "docx", "xls", "xlsx", "ppt", "pptx", "odt", "pdf", "rtf"};
    this->fileTypesMap["Downloaded Fonts"] = {"otf", "ttf"};
    this->fileTypesMap["Downloaded Images"] = {"png",  "jpeg", "jpg", "gif",
                                               "tiff", "psd",  "ai",  "eps"};
    this->fileTypesMap["Downloaded Programs"] = {"exe", "msi"};
    this->fileTypesMap["Downloaded Videos"] = {"mp4", "mov", "wmv", "avi",
                                               "mkv"};

    this->downloadFolder = QDir(path);
    qDebug() << "Instance created";
}

void DownloadSorter::run() {
    this->createFoldersIfDoesntExist();

    this->contents = this->downloadFolder.entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);

    this->moveContents(this->evaluateCategory());

    // emit fine("Done");
    // emit finished(this->downloadFolder.absolutePath());
}

void DownloadSorter::recalculateContents() {
    this->contents = this->downloadFolder.entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
}

int DownloadSorter::moveContents(QMap<QString, QString> filesPerCategory) {
    // QMap<QString>
    for (auto i = filesPerCategory.begin(), end = filesPerCategory.end();
         i != end; ++i) {
        // QString OriginalLocation = i.key().absoluteFilePath();
        // QString FileName = i.key();

        // QString DestinationFolder =
        // this->downloadFolder.absolutePath() + "/" + i.value();
        // QString RenamedDestination =
        // DestinationFolder + "/" + FileName;  // add here the file name
        // QString OriginalLocation =
        // this->downloadFolder.absolutePath() + "/" + FileName;

        qDebug() << "moving " << i.key() << " to " << i.value();

        LPCTSTR original = i.key().toStdWString().c_str();
        LPCTSTR destination = i.value().toStdWString().c_str();

        // QFile original = QFile(i.key());
        // QString destination = i.value();

        auto ret = MoveFile(original, destination);
        // auto ret = original.rename(destination);

        qDebug() << ret << " : moving " << original << " to " << destination;
        // qDebug() << " : moving " << original << " to " << destination;
    }

    return 0;
}

QMap<QString, QString> DownloadSorter::evaluateCategory() {
    // QList<QFileInfo> filteredContents;
    // qDebug() << "[qdebug] - evaluateCategory";
    QMap<QString, QString> filesPerCategory;

    for (qsizetype i = 0; i < this->contents.length(); i++) {
        QFileInfo content = this->contents.at(i);
        QString contentFileName = content.fileName();

        if (blacklist.contains(contentFileName)) {
            continue;  //   qDebug() << contentFileName;
        }

        // QString suffix = content.suffix();

        QString OutputFolder = this->suffixToFolder(content);
        if (OutputFolder == "*") {  // unrecognized file, don't move
            continue;
        }
        // .isEmpty()
        //    ? "/" + this->suffixToFolder(suffix)
        //    : "";

        // QString OriginalFolder = content.fileName();

        // qDebug() << contentFileName << " : " << OutputFolder;
        // qDebug() << this->contents.at(i).absoluteFilePath();

        // construction
        // QString FileName = i.key();
        QString OriginalLocation =
            this->downloadFolder.absolutePath() + "/" + contentFileName;

        QString DestinationFolder =
            this->downloadFolder.absolutePath() + OutputFolder;
        QString RenamedDestination = DestinationFolder + "/" +
                                     contentFileName;  // add here the file name

        // checking
        int counter = 1;
        qDebug() << RenamedDestination << " : "
                 << QFileInfo(RenamedDestination).exists();
        while (QFileInfo(RenamedDestination).exists()) {
            qDebug() << "[Duplicate Found] Renaming ... " << RenamedDestination;

            std::string diff =
                std::string(" (") + std::to_string(counter) + std::string(")");
            RenamedDestination += diff.c_str();
        }

        filesPerCategory[OriginalLocation] = RenamedDestination;
    }
    return filesPerCategory;
}

QString DownloadSorter::suffixToFolder(QFileInfo content) {
    // qDebug() << "[qdebug] - suffixToFolder";

    if (content.isDir()) {
        return "/Download Folders";
    }

    QString suffix = content.suffix();
    // if (suffix.isEmpty()) {
    // return "Download Folders";
    // }

    for (auto i = this->fileTypesMap.begin(), end = this->fileTypesMap.end();
         i != end; ++i) {
        // qDebug() << suffix;
        // qDebug() << i.value().contains(suffix);
        if (i.value().contains(suffix)) {
            return "/" + i.key();
        }
        // qDebug() << i.key() << " : " << i.value();
        // if ((*i).contains(suffix)) {
        // return (*i)
        // }
    }

    return "*";
}

void DownloadSorter::createFoldersIfDoesntExist() {
    for (qsizetype i = 0; i < this->blacklist.length(); i++) {
        QString folder =
            this->downloadFolder.absolutePath() + "/" + this->blacklist[i];
        QDir folder_dir = QDir(folder);

        if (!folder_dir.exists()) {
            std::filesystem::create_directory(
                folder_dir.absolutePath().toStdString());
        }
    }
}
