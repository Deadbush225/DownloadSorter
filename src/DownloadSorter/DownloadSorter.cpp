#include "../Include/DownloadSorter/DownloadSorter.h"
#include <strsafe.h>
#include <windows.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

// Persist helpers for mappings
namespace {
QString mappingsConfigPath() {
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(base);
    dir.mkpath("DownloadSorter");
    return dir.filePath("DownloadSorter/mappings.json");
}

static QJsonDocument readDoc() {
    QFile f(mappingsConfigPath());
    if (!f.exists()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("defaults"), QJsonArray());
        obj.insert(QStringLiteral("custom"), QJsonArray());
        return QJsonDocument(obj);
    }
    if (!f.open(QIODevice::ReadOnly)) {
        QJsonObject obj;
        obj.insert(QStringLiteral("defaults"), QJsonArray());
        obj.insert(QStringLiteral("custom"), QJsonArray());
        return QJsonDocument(obj);
    }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("defaults"), QJsonArray());
        obj.insert(QStringLiteral("custom"), QJsonArray());
        return QJsonDocument(obj);
    }
    return doc;
}

static bool writeDoc(const QJsonDocument& doc) {
    QFile f(mappingsConfigPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const auto bytes = doc.toJson(QJsonDocument::Indented);
    const bool ok = f.write(bytes) == bytes.size();
    f.close();
    return ok;
}

static QMap<QString, QList<QString>> readSection(const char* key) {
    QMap<QString, QList<QString>> map;
    const auto doc = readDoc();
    const auto obj = doc.object();
    const auto arr = obj.value(QLatin1String(key)).toArray();
    for (const auto& v : arr) {
        const auto o = v.toObject();
        const QString folder = o.value(QStringLiteral("folder")).toString();
        QList<QString> exts;
        for (const auto& ev : o.value(QStringLiteral("extensions")).toArray()) {
            const QString e = ev.toString().toLower();
            if (!e.isEmpty() && !exts.contains(e))
                exts.push_back(e);
        }
        if (!folder.isEmpty() && !exts.isEmpty())
            map.insert(folder, exts);
    }
    return map;
}

static void writeSection(const char* key,
                         const QMap<QString, QList<QString>>& map) {
    auto doc = readDoc();
    auto obj = doc.object();
    QJsonArray arr;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QJsonObject o;
        o.insert(QStringLiteral("folder"), it.key());
        QJsonArray exts;
        for (const auto& e : it.value())
            exts.push_back(e.toLower());
        o.insert(QStringLiteral("extensions"), exts);
        arr.push_back(o);
    }
    obj.insert(QLatin1String(key), arr);
    writeDoc(QJsonDocument(obj));
}

// Defaults used only when settings are empty
static QMap<QString, QList<QString>> defaultSeed() {
    QMap<QString, QList<QString>> m;
    m[QStringLiteral("Downloaded Archives")] = {"zip", "rar", "7z"};
    m[QStringLiteral("Downloaded Audios")] = {"m4a", "mp3", "wav", "aac"};
    m[QStringLiteral("Downloaded Documents")] = {
        "doc", "docx", "xls", "xlsx", "ppt", "pptx", "odt", "pdf", "rtf"};
    m[QStringLiteral("Downloaded Fonts")] = {"otf", "ttf"};
    m[QStringLiteral("Downloaded Images")] = {"png",  "jpeg", "jpg", "gif",
                                              "tiff", "psd",  "ai",  "eps"};
    m[QStringLiteral("Downloaded Programs")] = {"exe", "msi"};
    m[QStringLiteral("Downloaded Videos")] = {"mp4", "mov", "wmv", "avi",
                                              "mkv"};
    return m;
}

// Active rules helpers (single array format)
static QMap<QString, QList<QString>> parseArrayDoc(const QJsonDocument& doc) {
    QMap<QString, QList<QString>> map;
    if (!doc.isArray())
        return map;
    for (const auto& v : doc.array()) {
        const auto obj = v.toObject();
        const QString folder = obj.value(QStringLiteral("folder")).toString();
        QList<QString> exts;
        for (const auto& ev :
             obj.value(QStringLiteral("extensions")).toArray()) {
            const QString e = ev.toString().toLower();
            if (!e.isEmpty() && !exts.contains(e))
                exts.push_back(e);
        }
        if (!folder.isEmpty() && !exts.isEmpty())
            map.insert(folder, exts);
    }
    return map;
}

static bool saveActiveMappings(const QMap<QString, QList<QString>>& map) {
    QJsonArray arr;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QJsonObject o;
        o.insert(QStringLiteral("folder"), it.key());
        QJsonArray exts;
        for (const auto& e : it.value())
            exts.push_back(e.toLower());
        o.insert(QStringLiteral("extensions"), exts);
        arr.push_back(o);
    }
    QFile f(mappingsConfigPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const auto bytes = QJsonDocument(arr).toJson(QJsonDocument::Indented);
    const bool ok = f.write(bytes) == bytes.size();
    f.close();
    return ok;
}

static QMap<QString, QList<QString>> loadActiveMappings() {
    QFile f(mappingsConfigPath());
    if (!f.exists()) {
        auto seed = defaultSeed();
        saveActiveMappings(seed);
        return seed;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        auto seed = defaultSeed();
        saveActiveMappings(seed);
        return seed;
    }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (doc.isArray()) {
        const auto map = parseArrayDoc(doc);
        if (!map.isEmpty())
            return map;
        auto seed = defaultSeed();
        saveActiveMappings(seed);
        return seed;
    }

    if (doc.isObject()) {
        // Legacy: choose custom if present, otherwise defaults; convert to
        // active
        const auto custom = readSection("custom");
        const auto defs = readSection("defaults");
        const auto chosen = custom.isEmpty() ? defs : custom;
        if (!chosen.isEmpty()) {
            saveActiveMappings(chosen);
            return chosen;
        }
    }

    auto seed = defaultSeed();
    saveActiveMappings(seed);
    return seed;
}
}  // namespace

void ErrorExit(LPCTSTR lpszFunction) {
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpMsgBuf, 0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(
        LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) *
            sizeof(TCHAR));
    StringCchPrintf(
        (LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

DownloadSorter::DownloadSorter(QString& path) {
    qDebug() << "Creating an instance";

    this->downloadFolder = QDir(path);

    // Load active rules; seed defaults only if settings are empty
    this->fileTypesMap = loadActiveMappings();

    qDebug() << "Instance created";
}

void DownloadSorter::run() {
    this->createFoldersIfDoesntExist();

    // Only list files; do not include directories
    this->contents =
        this->downloadFolder.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    const auto plan = this->evaluateCategory();
    if (plan.isEmpty()) {
        emit statusMessage(QStringLiteral("Nothing to move."));
        // Nudge the progress bar to finished instantly
        emit progressRangeChanged(0, 1);
        emit progressValueChanged(1);
        return;
    }

    emit statusMessage(QStringLiteral("Moving %1 files...").arg(plan.size()));
    this->moveContents(plan);
}

void DownloadSorter::recalculateContents() {
    // Only list files; do not include directories
    this->contents =
        this->downloadFolder.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
}

// int DownloadSorter::moveContents(QMap<QString, QString> filesPerCategory) {
//     for (auto i = filesPerCategory.begin(), end = filesPerCategory.end();
//          i != end; ++i) {

//         qDebug() << "moving " << i.key() << " to " << i.value();

//         std::wstring originalPath = i.key().toStdWString();
//         std::wstring destinationPath = i.value().toStdWString();
//         LPCTSTR original = originalPath.c_str();
//         LPCTSTR destination = destinationPath.c_str();

//         qDebug() << "Original path: " <<
//         QString::fromStdWString(originalPath); qDebug() << "Destination path:
//         " << QString::fromStdWString(destinationPath);

//         // if (_waccess(originalPath.c_str(), 0) != 0) {
//         //     qDebug() << "File does not exist: " <<
//         QString::fromStdWString(originalPath);
//         //     continue;
//         // }

//         if (!MoveFile(original, destination)) {
//             DWORD error = GetLastError();
//             qDebug() << "MoveFile failed with error code: " << error;
//             // Handle specific errors if needed
//         } else {
//             qDebug() << "Successfully moved " << original << " to " <<
//             destination;
//         }
//     }

//     return 0;
// }

int DownloadSorter::moveContents(QMap<QString, QString> filesPerCategory) {
    const int total = filesPerCategory.size();
    emit progressRangeChanged(0, total);
    int done = 0;
    emit progressValueChanged(done);

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

        auto ret = MoveFile(original, destination);
        qDebug() << ret << " : moving " << original << " to " << destination;

        done++;
        emit progressValueChanged(done);
    }

    emit statusMessage(QStringLiteral("Done."));
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

            std::string diff = baseName.toStdString() + std::string(" (") +
                               std::to_string(counter) + std::string(").") +
                               suffixName.toStdString();
            RenamedDestination = DestinationFolder + '/' + diff.c_str();
            counter++;
        }

        filesPerCategory[OriginalLocation] = RenamedDestination;
    }
    return filesPerCategory;
}

QString DownloadSorter::suffixToFolder(QFileInfo content) {
    // Do not handle directories here
    if (content.isDir()) {
        return "*";
    }

    // Normalize suffix to lower for matching
    QString suffix = content.suffix().toLower();
    for (auto i = this->fileTypesMap.begin(), end = this->fileTypesMap.end();
         i != end; ++i) {
        if (i.value().contains(suffix)) {
            return "/" + i.key();
        }
    }
    return "*";
}

void DownloadSorter::createFoldersIfDoesntExist() {
    // Create built-in folders from blacklist (if missing)
    for (qsizetype i = 0; i < this->blacklist.length(); i++) {
        QString folder =
            this->downloadFolder.absolutePath() + "/" + this->blacklist[i];
        QDir folder_dir = QDir(folder);

        if (!folder_dir.exists()) {
            std::filesystem::create_directory(
                folder_dir.absolutePath().toStdString());
        }
    }

    // Ensure custom/default rule folders exist at the download root.
    // If they exist under "Download Folders", move them back to root.
    for (auto it = this->fileTypesMap.begin(); it != this->fileTypesMap.end();
         ++it) {
        const QString rootFolder =
            this->downloadFolder.absolutePath() + "/" + it.key();
        QDir rootDir(rootFolder);
        if (rootDir.exists())
            continue;

        const QString nestedFolder = this->downloadFolder.absolutePath() +
                                     "/Download Folders/" + it.key();
        QDir nestedDir(nestedFolder);
        if (nestedDir.exists()) {
            try {
                std::filesystem::rename(nestedDir.absolutePath().toStdString(),
                                        rootDir.absolutePath().toStdString());
            } catch (const std::exception&) {
                if (!rootDir.exists()) {
                    std::filesystem::create_directory(
                        rootDir.absolutePath().toStdString());
                }
            }
        } else {
            std::filesystem::create_directory(
                rootDir.absolutePath().toStdString());
        }
    }
}
