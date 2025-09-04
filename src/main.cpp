#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>

#include "./Include/DownloadSorter/Dashboard.h"
#include "./Include/DownloadSorter/DownloadSorter.h"

static QString readVersionFromManifest() {
#ifdef APP_VERSION
    return QString::fromUtf8(APP_VERSION);
#else
    return QStringLiteral("0.0.0");
#endif
}

int main(int argc, char* argv[]) {
    QApplication* app = new QApplication(argc, argv);
    // Set app-wide icon (used by taskbar/dock)

    app->setWindowIcon(QIcon(":/appicon"));

    // Set application version from manifest (with compile-time fallback)
    QCoreApplication::setApplicationVersion(readVersionFromManifest());

    // DownloadSorter* ds = new DownloadSorter("E:/Downloads");

    /*
        create a class that manages the deletion of the files.
    */

    // QCoreApplication::setApplicationName("Download Sorter");

    Dashboard dashboard;
    dashboard.show();

    return app->exec();
}