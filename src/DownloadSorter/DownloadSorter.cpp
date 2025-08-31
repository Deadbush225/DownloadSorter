#include "../Include/DownloadSorter/DownloadSorter.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

// Keep constructor minimal; settings (mappings/ignore) are injected by
// Dashboard
DownloadSorter::DownloadSorter(const QString& path) {
    this->downloadFolder = QDir(path);
}

void DownloadSorter::run() {
    this->createFoldersIfDoesntExist();

    // Include directories in the contents list
    this->contents = this->downloadFolder.entryInfoList(
        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    const auto plan = this->evaluateCategory();
    if (plan.isEmpty()) {
        emit statusMessage(QStringLiteral("Nothing to move."));
        emit progressRangeChanged(0, 1);
        emit progressValueChanged(1);
        return;
    }

    emit statusMessage(QStringLiteral("Moving %1 items...").arg(plan.size()));
    this->moveContents(plan);
}

void DownloadSorter::recalculateContents() {
    this->contents = this->downloadFolder.entryInfoList(
        QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
}

int DownloadSorter::moveContents(QMap<QString, QString> filesPerCategory) {
    const int total = filesPerCategory.size();
    emit progressRangeChanged(0, total);
    int done = 0;
    emit progressValueChanged(done);

    for (auto i = filesPerCategory.begin(), end = filesPerCategory.end();
         i != end; ++i) {
        const QString src = i.key();
        const QString dst = i.value();

        // Ensure destination directory exists
        const QFileInfo dstInfo(dst);
        QDir().mkpath(dstInfo.path());

        bool ok = QFile::rename(src, dst);
        if (!ok) {
            // Fallback for cross-device moves: copy then remove
            if (QFile::copy(src, dst)) {
                QFile::remove(src);
                ok = true;
            }
        }

        if (!ok) {
            qWarning() << "Failed to move" << src << "to" << dst << ":"
                       << QFile(src).errorString();
        }

        done++;
        emit progressValueChanged(done);
    }

    emit statusMessage(QStringLiteral("Done."));
    return 0;
}

QMap<QString, QString> DownloadSorter::evaluateCategory() {
    QMap<QString, QString> filesPerCategory;

    for (auto it = this->contents.begin(); it != this->contents.end(); ++it) {
        const QFileInfo content = *it;
        const QString baseName = content.completeBaseName();
        const QString suffixName = content.suffix();
        const QString contentFileName = content.fileName();

        if (blacklist.contains(contentFileName)) {
            continue;
        }

        // Ignore via regex (both files and directories)
        bool shouldIgnore = false;
        for (const QRegularExpression& regex : this->ignorePatterns) {
            if (regex.isValid() && regex.match(contentFileName).hasMatch()) {
                shouldIgnore = true;
                break;
            }
        }
        if (shouldIgnore)
            continue;

        // Directories: move to "Downloaded Folders"
        if (content.isDir()) {
            const QString outputFolder = "/Downloaded Folders";
            const QString originalLocation =
                this->downloadFolder.absolutePath() + "/" + contentFileName;
            const QString destinationFolder =
                this->downloadFolder.absolutePath() + outputFolder;
            QString renamedDestination =
                destinationFolder + "/" + contentFileName;

            int counter = 1;
            while (QFileInfo(renamedDestination).exists()) {
                const QString diff =
                    contentFileName + " (" + QString::number(counter) + ")";
                renamedDestination = destinationFolder + "/" + diff;
                counter++;
            }

            filesPerCategory[originalLocation] = renamedDestination;
            continue;
        }

        // Files
        const QString outputFolder = this->suffixToFolder(content);
        if (outputFolder == "*")  // unrecognized file, skip
            continue;

        const QString originalLocation =
            this->downloadFolder.absolutePath() + "/" + contentFileName;
        const QString destinationFolder =
            this->downloadFolder.absolutePath() + outputFolder;
        QString renamedDestination = destinationFolder + "/" + contentFileName;

        // Handle duplicates
        int counter = 1;
        while (QFileInfo(renamedDestination).exists()) {
            const std::string diff = baseName.toStdString() + " (" +
                                     std::to_string(counter) + ")." +
                                     suffixName.toStdString();
            renamedDestination =
                destinationFolder + '/' + QString::fromStdString(diff);
            counter++;
        }

        filesPerCategory[originalLocation] = renamedDestination;
    }

    return filesPerCategory;
}

QString DownloadSorter::suffixToFolder(QFileInfo content) {
    const QString suffix = content.suffix().toLower();
    for (auto it = this->fileTypesMap.begin(), end = this->fileTypesMap.end();
         it != end; ++it) {
        if (it.value().contains(suffix)) {
            return "/" + it.key();
        }
    }
    return "*";
}

void DownloadSorter::createFoldersIfDoesntExist() {
    // Create built-in folders from blacklist (if missing)
    for (qsizetype i = 0; i < this->blacklist.length(); i++) {
        const QString folder =
            this->downloadFolder.absolutePath() + "/" + this->blacklist[i];
        QDir folder_dir(folder);
        if (!folder_dir.exists()) {
            std::filesystem::create_directory(
                folder_dir.absolutePath().toStdString());
        }
    }
}
