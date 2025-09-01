#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>

#include "./Include/DownloadSorter/Dashboard.h"
#include "./Include/DownloadSorter/DownloadSorter.h"

int main(int argc, char* argv[]) {
    QApplication* app = new QApplication(argc, argv);
    // Set app-wide icon (used by taskbar/dock)

    app->setWindowIcon(QIcon(":/appicon"));

    // DownloadSorter* ds = new DownloadSorter("E:/Downloads");

    /*
        create a class that manages the deletion of the files.
    */

    // QCoreApplication::setApplicationName("Download Sorter");

    Dashboard dashboard;
    dashboard.show();

    return app->exec();
}