#include "DownloadSorter/DownloadSorterWidget.h"

#include "DownloadSorter/DownloadSorter.h"
#include "DownloadSorter/DownloadSorterConfig.h"
#include "DownloadSorter/SettingsDialog.h"

#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

DownloadSorterWidget::DownloadSorterWidget(DownloadSorter* sorter,
                                           QWidget* parent)
    : QWidget(parent), sorter(sorter) {
    auto* layout = new QVBoxLayout(this);
    configureBtn = new QPushButton(QStringLiteral("Configure Rules..."), this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 1);
    progressBar->setValue(0);

    layout->addWidget(configureBtn);
    layout->addWidget(progressBar);
    setLayout(layout);

    connect(configureBtn, &QPushButton::clicked, this,
            &DownloadSorterWidget::openSettings);
    connectProgressSignals();
    loadInitialMappings();
}

void DownloadSorterWidget::loadInitialMappings() {
    const auto map = DownloadSorterConfig::loadMappings();
    if (sorter && !map.isEmpty()) {
        sorter->setFileTypesMap(map);
        emit mappingsChanged(map);
    }
}

void DownloadSorterWidget::connectProgressSignals() {
    if (!sorter)
        return;
    connect(sorter, &DownloadSorter::progressRangeChanged, progressBar,
            &QProgressBar::setRange);
    connect(sorter, &DownloadSorter::progressValueChanged, progressBar,
            &QProgressBar::setValue);
    // Optional: show text if you emit statusMessage
    // connect(sorter, &DownloadSorter::statusMessage, progressBar,
    // &QProgressBar::setFormat);
}

void DownloadSorterWidget::openSettings() {
    if (!sorter)
        return;
    bool ok = false;
    auto current = sorter->getFileTypesMap();
    auto map = SettingsDialog::getMappings(
        this,
        current.isEmpty() ? DownloadSorterConfig::loadMappings() : current,
        &ok);
    if (!ok)
        return;
    applyMappings(map);
}

void DownloadSorterWidget::applyMappings(
    const QMap<QString, QList<QString>>& map) {
    if (!sorter)
        return;
    sorter->setFileTypesMap(map);
    DownloadSorterConfig::saveMappings(map);
    emit mappingsChanged(map);
}
