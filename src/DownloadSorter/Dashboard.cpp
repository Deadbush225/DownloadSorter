#include "../Include/DownloadSorter/Dashboard.h"
#include "../Include/DownloadSorter/DownloadSorter.h"
#include "../Include/DownloadSorter/SettingsDialog.h"
#include "../Include/DownloadSorter/SettingsManager.h"

#include <QAction>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QStandardPaths>

#include <QApplication>
#include <QPalette>

void setDarkTheme() {
    QApplication::setStyle("Fusion");

    QPalette* dark_palette = new QPalette();
    QColor baseColor(31, 31, 31);
    QColor textColor(Qt::white);
    // Use a green accent
    QColor highlightColor(0, 136, 57);  // rgb(0, 146, 61)
    QColor disabledTextColor(Qt::darkGray);

    dark_palette->setColor(QPalette::Window, baseColor);
    dark_palette->setColor(QPalette::WindowText, textColor);
    dark_palette->setColor(QPalette::Base,
                           baseColor.darker(160));  // slightly darker
    dark_palette->setColor(QPalette::AlternateBase, baseColor);
    dark_palette->setColor(QPalette::ToolTipBase, baseColor.darker(120));
    dark_palette->setColor(QPalette::ToolTipText, textColor);
    dark_palette->setColor(QPalette::Text, textColor);
    dark_palette->setColor(QPalette::Button, baseColor);
    dark_palette->setColor(QPalette::ButtonText, textColor);
    dark_palette->setColor(QPalette::BrightText, Qt::red);
    dark_palette->setColor(QPalette::Link, highlightColor);
    dark_palette->setColor(QPalette::Highlight, highlightColor);
    // Ensure selected/hovered text stays readable (white on green)
    dark_palette->setColor(QPalette::HighlightedText, Qt::white);
    dark_palette->setColor(QPalette::Active, QPalette::Button, baseColor);
    dark_palette->setColor(QPalette::Disabled, QPalette::ButtonText,
                           disabledTextColor);
    dark_palette->setColor(QPalette::Disabled, QPalette::WindowText,
                           disabledTextColor);
    dark_palette->setColor(QPalette::Disabled, QPalette::Text,
                           disabledTextColor);
    dark_palette->setColor(QPalette::Disabled, QPalette::Light, baseColor);
    QApplication::setPalette(*dark_palette);

    qApp->setStyleSheet(R"(
            QGroupBox { 
    border: 1px solid #2f2f2f;
    border-radius: 3px;
    margin-top: 0.6em; 
    padding: 0.3em;
}

QGroupBox::title {
    subcontrol-origin: margin;
    margin-left: 0em;
}

QFrame[frameShape="4"] { /* QFrame::HLine */
    border: none;
    border-top: 1px solid #2f2f2f;
    background: #2f2f2f;
    margin: 0.5em 0;
}

/* Menu bar */
QMenuBar {
    background-color: #262626;
    color: #dddddd;
}
QMenuBar::item {
    background: transparent;
    padding: 4px 10px;
}
QMenuBar::item:selected,
QMenuBar::item:pressed {
    background: rgb(0, 136, 57); /* green accent */
    color: #ffffff;      /* keep text readable */
}
QMenuBar::item:disabled {
    color: #666666;
}

/* Menus and submenus */
QMenu {
    background-color: #262626;
    color: #dddddd;
    border: 1px solid #2f2f2f;
}
QMenu::separator {
    height: 1px;
    background: #2f2f2f;
    margin: 4px 8px;
}
QMenu::item {
    background: transparent;
    padding: 6px 18px;
}
QMenu::item:selected {
    background: rgb(0, 136, 57); /* green accent */
    color: #ffffff;      /* force white text on hover/selection */
}
QMenu::item:disabled {
    color: #666666;
}

/* Inputs */
QLineEdit {
    background: #2a2a2a;
    color: #ffffff;
    selection-background-color:rgb(0, 136, 57);
    selection-color: #ffffff;
    border: 1px solid #3a3a3a;
    border-radius: 3px;
}
)");
}

Dashboard::Dashboard() {
    // ++ Retrieving settings
    setDarkTheme();

    this->setWindowIcon(QIcon(":/appicon"));

    QString retrieved_path = this->settings->value("Download Path").toString();
    if (retrieved_path.isEmpty()) {
        this->currentDownloadFolder =
            QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    } else {
        this->currentDownloadFolder = retrieved_path;
    }

    // Menu with "Configure Rules..." action
    this->rulesMenu = this->menuBar()->addMenu("&Rules");
    this->configureRulesAction =
        this->rulesMenu->addAction("Configure Rules...");
    QObject::connect(this->configureRulesAction, &QAction::triggered, this,
                     [this]() { this->openRulesConfigurator(); });

    // Help menu and actions
    this->helpMenu = this->menuBar()->addMenu("&Help");
    this->checkUpdatesAction =
        this->helpMenu->addAction("Check for &Updates...");
    QObject::connect(this->checkUpdatesAction, &QAction::triggered, this,
                     &Dashboard::checkForUpdates);
    this->helpMenu->addSeparator();
    this->aboutAction = this->helpMenu->addAction("&About Download Sorter");
    QObject::connect(this->aboutAction, &QAction::triggered, this,
                     &Dashboard::showAbout);

    // Status-bar progress bar (hidden by default)
    this->progressBar = new QProgressBar(this);
    this->progressBar->setMaximumWidth(120);
    this->progressBar->setMaximumHeight(15);
    this->progressBar->setTextVisible(false);
    this->statusBar()->addPermanentWidget(this->progressBar);
    // this->progressBar->hide();

    QVBoxLayout* mainlayout = new ModQVBoxLayout();

    // QGroupBox* groupbox = new QGroupBox("Test");
    // groupbox->setCheckable(true);

    /* Browser */
    QVBoxLayout* browserlayout = new ModQVBoxLayout();

    ModQLabel* label = new ModQLabel("Select Path:");

    QHBoxLayout* searcherlayout = new ModQHBoxLayout();

    // browserlayout->addWidget(label);

    // QLineEdit* pathField = new QLineEdit();

    QIcon* search_icon = new QIcon(":/search.png");
    QPushButton* search_btn = new QPushButton(*search_icon, "");
    QObject::connect(search_btn, &QPushButton::clicked, this,
                     &Dashboard::browseDownloadFolder);

    QPushButton* arrangeButton = new QPushButton("Begin Sort");
    QObject::connect(arrangeButton, &QPushButton::clicked, this,
                     &Dashboard::initiateSort);

    arrangeButton->setFixedHeight(40);

    this->pathField->setText(this->currentDownloadFolder);

    searcherlayout->addWidget(this->pathField);
    searcherlayout->addWidget(search_btn);

    browserlayout->addWidget(label);
    browserlayout->addLayout(searcherlayout);

    mainlayout->addLayout(browserlayout);

    mainlayout->addSpacing(10);
    mainlayout->addStretch(2);
    mainlayout->addWidget(arrangeButton);

    QWidget* central_widget = new QWidget();

    this->setCentralWidget(central_widget);
    this->centralWidget()->setLayout(mainlayout);

    this->statusBar()->show();

    this->setMinimumWidth(450);
}

void Dashboard::initiateSort() {
    if (this->progressBar) {
        this->progressBar->hide();
        this->progressBar->setRange(0, 100);
        this->progressBar->setValue(0);
    }

    auto* ds = new DownloadSorter(this->currentDownloadFolder);

    // Get settings from SettingsManager
    const SettingsData settings = SettingsManager::read();
    ds->setFileTypesMap(settings.mappings);
    ds->setIgnorePatterns(settings.ignorePatterns);

    // Wire progress to status bar progress bar (use qualified
    // pointer-to-member)
    QObject::connect(ds, &DownloadSorter::progressRangeChanged, this,
                     [this](int min, int max) {
                         if (this->progressBar) {
                             this->progressBar->setRange(min, max);
                             this->progressBar->show();
                         }
                     });
    QObject::connect(ds, &DownloadSorter::progressValueChanged, this,
                     [this](int value) {
                         if (this->progressBar) {
                             this->progressBar->setValue(value);
                         }
                     });
    QObject::connect(
        ds, &DownloadSorter::statusMessage, this,
        [this](const QString& m) { this->statusBar()->showMessage(m); });
    QObject::connect(ds, &DownloadSorter::finished, this,
                     &Dashboard::downloadFinished);
    QObject::connect(ds, &QThread::finished, ds, &QObject::deleteLater);

    this->onSortStarted();
    ds->start();
}

void Dashboard::downloadFinished() {
    this->statusBar()->showMessage(
        QString("Finished: '%1'").arg(this->currentDownloadFolder), 5000);
}

void Dashboard::browseDownloadFolder() {
    QString downloadFolder = QFileDialog::getExistingDirectory(
        this, "Select Download Directory", "C:/Downloads",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (downloadFolder.isEmpty()) {
        this->statusBar()->showMessage("Operation Canceled.", 5000);
        return;
    }

    this->currentDownloadFolder = downloadFolder;

    this->pathField->setText(this->currentDownloadFolder);

    this->settings->setValue("Download Path", this->currentDownloadFolder);

    QString message = QString("Set '%1' as the current download folder")
                          .arg(this->currentDownloadFolder);

    this->statusBar()->showMessage(message, 5000);
}

// Show indeterminate progress while sorting (until range is known)
void Dashboard::onSortStarted() {
    if (this->progressBar) {
        this->progressBar->setRange(0, 0);  // indeterminate
        this->progressBar->show();
    }
    this->statusBar()->showMessage("Sorting...");
}

// Open settings dialog for rules from the menu action
void Dashboard::openRulesConfigurator() {
    // One-call helper: loads, shows, and persists on accept
    if (SettingsDialog::editSettings(this)) {
        this->statusBar()->showMessage("Settings saved.", 3000);
    } else {
        this->statusBar()->showMessage("Settings unchanged.", 3000);
    }
}
