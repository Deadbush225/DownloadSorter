#include "Dashboard.h"
// #include "subclass.h"

Dashboard::Dashboard() {
    // ++ Retrieving settings

    QString retrieved_path = this->settings->value("Download Path").toString();
    if (retrieved_path.isEmpty()) {
        this->currentDownloadFolder = QString("Not Set");
    } else {
        this->currentDownloadFolder = retrieved_path;
    }

    //

    QVBoxLayout* mainlayout = new ModQVBoxLayout();

    // QGroupBox* groupbox = new QGroupBox("Test");
    // groupbox->setCheckable(true);

    /* Browser */
    QVBoxLayout* browserlayout = new ModQVBoxLayout();

    ModQLabel* label = new ModQLabel("Select Path:");

    QHBoxLayout* searcherlayout = new ModQHBoxLayout();

    // browserlayout->addWidget(label);

    // QLineEdit* pathField = new QLineEdit();

    QIcon* search_icon = new QIcon(":/icons/search.png");
    QPushButton* search_btn = new QPushButton(*search_icon, "");
    QObject::connect(search_btn, QPushButton::clicked, this,
                     Dashboard::browseDownloadFolder);
    // QPushButton* search_btn = new QPushButton("");

    QPushButton* arrangeButton = new QPushButton("Begin Sort");
    QObject::connect(arrangeButton, &QPushButton::clicked, this,
                     &Dashboard::initiateSort);

    arrangeButton->setFixedHeight(40);

    this->pathField->setText(this->currentDownloadFolder);

    searcherlayout->addWidget(this->pathField);
    searcherlayout->addWidget(search_btn);

    browserlayout->addWidget(label);
    browserlayout->addLayout(searcherlayout);
    // browserlayout->setSpacing(0);

    mainlayout->addLayout(browserlayout);

    mainlayout->addSpacing(10);
    mainlayout->addStretch(2);
    mainlayout->addWidget(arrangeButton);

    QWidget* central_widget = new QWidget();

    this->setCentralWidget(central_widget);
    this->centralWidget()->setLayout(mainlayout);

    this->statusBar()->show();

    this->setMinimumWidth(450);

    // DownloadSorter* ds = new DownloadSorter("E:/Downloads");
}

void Dashboard::initiateSort() {
    DownloadSorter* ds = new DownloadSorter(this->currentDownloadFolder);
    QObject::connect(ds, &DownloadSorter::finished, this, &Dashboard::downloadFinished);

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

    this->currentDownloadFolder = downloadFolder;

    this->pathField->setText(this->currentDownloadFolder);

    this->settings->setValue("Download Path", this->currentDownloadFolder);

    // return downloadFolder;
    QString message = QString("Set '%1' as the current download folder")
                          .arg(this->currentDownloadFolder);

    this->statusBar()->showMessage(message, 5000);
}
