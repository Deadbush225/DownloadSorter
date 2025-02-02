#include <QtCore/QString>
#include <QtWidgets/QApplication>
#include <iostream>

#include "./Include/DownloadSorter/Dashboard.h"
#include "./Include/DownloadSorter/DownloadSorter.h"

int main(int argc, char* argv[]) {
    QApplication* app = new QApplication(argc, argv);

    // DownloadSorter* ds = new DownloadSorter("E:/Downloads");

    /*
        create a class that manages the deletion of the files.
    */

    // QCoreApplication::setApplicationName("Download Sorter");

    Dashboard dashboard;
    // Dashboard* dashboard = new Dashboard();
    dashboard.show();

    // dashboard->setStyleSheet("*{border:1px solid black;}");

    return app->exec();

    // std::cout << "Press any key to exit";
    // std::string line;
    // std::cin >> line;
    // delete app;
    // delete ds;
}