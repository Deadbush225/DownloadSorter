#include "../Include/DownloadSorter/SettingsDialog.h"
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include "../Include/DownloadSorter/SettingsManager.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");

    QVBoxLayout* layout = new QVBoxLayout(this);

    // Mappings section
    QGroupBox* mappingsGroup = new QGroupBox("File Type Mappings");
    QVBoxLayout* mappingsLayout = new QVBoxLayout(mappingsGroup);
    mappingsTable = new QTableWidget();
    mappingsTable->setColumnCount(2);
    mappingsTable->setHorizontalHeaderLabels({"Folder", "Extensions"});
    mappingsLayout->addWidget(mappingsTable);
    QHBoxLayout* mappingsBtnLayout = new QHBoxLayout();
    addMappingBtn = new QPushButton("Add");
    removeMappingBtn = new QPushButton("Remove");
    mappingsBtnLayout->addWidget(addMappingBtn);
    mappingsBtnLayout->addWidget(removeMappingBtn);
    mappingsLayout->addLayout(mappingsBtnLayout);
    layout->addWidget(mappingsGroup);

    // Ignore patterns section
    QGroupBox* ignoreGroup = new QGroupBox("Ignore Patterns (Regex)");
    QVBoxLayout* ignoreLayout = new QVBoxLayout(ignoreGroup);
    ignoreList = new QListWidget();
    // Allow common edit triggers (double-click, edit key, typing)
    ignoreList->setEditTriggers(QAbstractItemView::DoubleClicked |
                                QAbstractItemView::EditKeyPressed |
                                QAbstractItemView::AnyKeyPressed);
    ignoreLayout->addWidget(ignoreList);
    QHBoxLayout* ignoreBtnLayout = new QHBoxLayout();
    addIgnoreBtn = new QPushButton("Add");
    removeIgnoreBtn = new QPushButton("Remove");
    ignoreBtnLayout->addWidget(addIgnoreBtn);
    ignoreBtnLayout->addWidget(removeIgnoreBtn);
    ignoreLayout->addLayout(ignoreBtnLayout);
    layout->addWidget(ignoreGroup);

    // Buttons
    QDialogButtonBox* buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(addMappingBtn, &QPushButton::clicked, this,
            &SettingsDialog::addMapping);
    connect(removeMappingBtn, &QPushButton::clicked, this,
            &SettingsDialog::removeMapping);
    connect(addIgnoreBtn, &QPushButton::clicked, this,
            &SettingsDialog::addIgnorePattern);
    connect(removeIgnoreBtn, &QPushButton::clicked, this,
            &SettingsDialog::removeIgnorePattern);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::setMappings(
    const QMap<QString, QList<QString>>& mappings) {
    mappingsTable->setRowCount(mappings.size());
    int row = 0;
    for (auto it = mappings.begin(); it != mappings.end(); ++it) {
        mappingsTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        mappingsTable->setItem(row, 1,
                               new QTableWidgetItem(it.value().join(", ")));
        row++;
    }
}

QMap<QString, QList<QString>> SettingsDialog::getMappings() const {
    QMap<QString, QList<QString>> map;
    for (int row = 0; row < mappingsTable->rowCount(); ++row) {
        QString folder = mappingsTable->item(row, 0)->text();
        QString extsStr = mappingsTable->item(row, 1)->text();
        QList<QString> exts = extsStr.split(", ", Qt::SkipEmptyParts);
        if (!folder.isEmpty() && !exts.isEmpty()) {
            map[folder] = exts;
        }
    }
    return map;
}

void SettingsDialog::setIgnorePatterns(const QList<QString>& patterns) {
    ignoreList->clear();
    for (const QString& pattern : patterns) {
        // create editable items for existing patterns
        QListWidgetItem* it = new QListWidgetItem(pattern);
        it->setFlags(it->flags() | Qt::ItemIsEditable);
        ignoreList->addItem(it);
    }
}

QList<QString> SettingsDialog::getIgnorePatterns() const {
    QList<QString> patterns;
    for (int i = 0; i < ignoreList->count(); ++i) {
        QString pattern = ignoreList->item(i)->text();
        if (!pattern.isEmpty()) {
            patterns.append(pattern);
        }
    }
    return patterns;
}

bool SettingsDialog::getSettings(QWidget* parent,
                                 QMap<QString, QList<QString>>& mappings,
                                 QList<QString>& ignorePatterns) {
    SettingsDialog dialog(parent);
    dialog.setMappings(mappings);
    dialog.setIgnorePatterns(ignorePatterns);
    if (dialog.exec() == QDialog::Accepted) {
        mappings = dialog.getMappings();
        qDebug() << mappings;
        ignorePatterns = dialog.getIgnorePatterns();
        qDebug() << ignorePatterns;
        return true;
    }
    return false;
}

bool SettingsDialog::editSettings(QWidget* parent) {
    // Read current settings (seeds defaults if needed)
    SettingsData data = SettingsManager::read();

    SettingsDialog dialog(parent);
    dialog.setMappings(data.mappings);
    dialog.setIgnorePatterns(data.ignorePatterns);
    if (dialog.exec() == QDialog::Accepted) {
        data.mappings = dialog.getMappings();
        data.ignorePatterns = dialog.getIgnorePatterns();
        return SettingsManager::write(data);
    }
    return false;
}

void SettingsDialog::addMapping() {
    int row = mappingsTable->rowCount();
    mappingsTable->insertRow(row);
    mappingsTable->setItem(row, 0, new QTableWidgetItem("New Folder"));
    mappingsTable->setItem(row, 1, new QTableWidgetItem("ext1, ext2"));
}

void SettingsDialog::removeMapping() {
    int row = mappingsTable->currentRow();
    if (row >= 0) {
        mappingsTable->removeRow(row);
    }
}

void SettingsDialog::addIgnorePattern() {
    // create an editable item and start inline editing immediately
    QListWidgetItem* item = new QListWidgetItem("New Regex");
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ignoreList->addItem(item);
    ignoreList->setCurrentItem(item);
    ignoreList->editItem(item);
}

void SettingsDialog::removeIgnorePattern() {
    int row = ignoreList->currentRow();
    if (row >= 0) {
        delete ignoreList->takeItem(row);
    }
}
