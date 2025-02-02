#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QtCore/QDir>
#include <QtGui/QIcon>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
// #include <QtWidgets/QSpacerItem>
#include <QtCore/QSettings>
#include <QtWidgets/QMainWindow>

#include <QtWidgets/QStatusBar>

#include <QtWidgets/QGroupBox>

#include "DownloadSorter.h"
#include "subclass.h"

// #include <boost/format.hpp>

class Dashboard : public QMainWindow {
   public:
    Dashboard();
    virtual ~Dashboard() = default;

   private:
    QString currentDownloadFolder = QString("E:/Downloads");

    QLineEdit* pathField = new QLineEdit();
    QSettings* settings = new QSettings(QSettings::NativeFormat,
                                        QSettings::UserScope,
                                        "Yangkie",
                                        "Download Sorter");

    void initiateSort();
    void downloadFinished();

   public slots:
    void browseDownloadFolder();
    // void browseFolder();
};

#endif