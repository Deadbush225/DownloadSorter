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

// New UI pieces for menu and progress bar
#include <QtWidgets/QProgressBar>
class QAction;
class QMenu;

#include "subclass.h"

// #include <boost/format.hpp>

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtWidgets/QMessageBox>

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

    // Menu action and a status-bar progress bar
    QMenu* rulesMenu = nullptr;
    QAction* configureRulesAction = nullptr;
    QProgressBar* progressBar = nullptr;

    // Help menu
    QMenu* helpMenu = nullptr;
    QAction* checkUpdatesAction = nullptr;
    QAction* aboutAction = nullptr;

    // Helpers
    void onSortStarted();
    void openRulesConfigurator();
    void checkForUpdates();
    void showAbout();

   public slots:
    void browseDownloadFolder();
};

#endif

// Inline implementations to ensure the Help -> Check for Updates starts
// Updater.exe
inline void Dashboard::checkForUpdates() {
    QString updaterPath = QCoreApplication::applicationDirPath() + "/Updater";
#ifdef Q_OS_WIN
    updaterPath += ".exe";
#endif
    if (QFileInfo::exists(updaterPath)) {
        if (!QProcess::startDetached(updaterPath, {})) {
            QMessageBox::warning(this, "Update Check",
                                 "Failed to start the Updater.");
        }
    } else {
        QMessageBox::information(
            this, "Updater Not Found",
            "Updater was not found next to the application.\n"
            "Please download the latest release from GitHub.");
    }
}

inline void Dashboard::showAbout() {
    const QString text = QStringLiteral(
                             "Download Sorter\n"
                             "Organize your downloads automatically\n\n"
                             "Version: %1\n"
                             "Built with Qt %2")
                             .arg(QCoreApplication::applicationVersion(),
                                  QString::fromLatin1(QT_VERSION_STR));
    QMessageBox::about(this, "About Download Sorter", text);
}