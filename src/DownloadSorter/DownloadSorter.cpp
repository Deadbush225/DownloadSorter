#include "DownloadSorter.h"
#include <windows.h>
#include <strsafe.h>

void ErrorExit(LPCTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                                       (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"),
                    lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

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

// int DownloadSorter::moveContents(QMap<QString, QString> filesPerCategory) {
//     for (auto i = filesPerCategory.begin(), end = filesPerCategory.end();
//          i != end; ++i) {

//         qDebug() << "moving " << i.key() << " to " << i.value();

//         std::wstring originalPath = i.key().toStdWString();
//         std::wstring destinationPath = i.value().toStdWString();
//         LPCTSTR original = originalPath.c_str();
//         LPCTSTR destination = destinationPath.c_str();

//         qDebug() << "Original path: " << QString::fromStdWString(originalPath);
//         qDebug() << "Destination path: " << QString::fromStdWString(destinationPath);

//         // if (_waccess(originalPath.c_str(), 0) != 0) {
//         //     qDebug() << "File does not exist: " << QString::fromStdWString(originalPath);
//         //     continue;
//         // }

//         if (!MoveFile(original, destination)) {
//             DWORD error = GetLastError();
//             qDebug() << "MoveFile failed with error code: " << error;
//             // Handle specific errors if needed
//         } else {
//             qDebug() << "Successfully moved " << original << " to " << destination;
//         }
//     }

//     return 0;
// }

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

        std::wstring originalPath = i.key().toStdWString();
        LPCTSTR original = originalPath.c_str();
        std::wstring destinationPath = i.value().toStdWString();
        LPCTSTR destination = destinationPath.c_str();

        // MessageBox(NULL, original, TEXT("Error"), MB_OK);

        // QFile original = QFile(i.key());
        // QString destination = i.value();

        auto ret = MoveFile(original, destination);
        // ErrorExit(TEXT("MoveFile"));
        // auto ret = original.rename(destination);

        qDebug() << ret << " : moving " << original << " to " << destination;
        // qDebug() << "ERROR" << e;
        // qDebug() << " : moving " << original << " to " << destination;
    }

    return 0;
}

QMap<QString, QString> DownloadSorter::evaluateCategory() {
    // QList<QFileInfo> filteredContents;
    // qDebug() << "[qdebug] - evaluateCategory";
    QMap<QString, QString> filesPerCategory;

    for (auto i = this->contents.begin(); i != this->contents.end(); i++) {
        // QFileInfo content = this->contents.at(i);
        QFileInfo content = *i;
        QString baseName = content.completeBaseName();
        QString suffixName = content.suffix();
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

            std::string diff = baseName.toStdString() +
                std::string(" (") + std::to_string(counter) + std::string(").") + suffixName.toStdString();
            RenamedDestination = DestinationFolder + '/' + diff.c_str();
            counter++;
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
