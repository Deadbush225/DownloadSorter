#pragma once

#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVariant>

class SettingsDialog : public QDialog {
    Q_OBJECT
   public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    // Utility to show dialog and return result
    static QMap<QString, QList<QString>> getMappings(
        QWidget* parent,
        const QMap<QString, QList<QString>>& initial,
        bool* ok = nullptr);

    void setMappings(const QMap<QString, QList<QString>>& map);
    QMap<QString, QList<QString>> mappings() const;

   private slots:
    void onAddRow();
    void onRemoveRow();
    void onBrowseFolder();
    void onAccept();

   private:
    QTableWidget* table = nullptr;
    QPushButton* addBtn = nullptr;
    QPushButton* removeBtn = nullptr;
    QPushButton* browseBtn = nullptr;

    static QStringList parseExtensions(const QString& text);
    void setupUi();
    void addRow(const QString& folder, const QStringList& exts);
};

// ===== Inline implementations to resolve linker errors =====

inline SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setupUi();
}

inline void SettingsDialog::setupUi() {
    setWindowTitle(QStringLiteral("Download Sorter Rules"));
    auto* layout = new QVBoxLayout(this);

    auto* info =
        new QLabel(QStringLiteral("Edit relative folder and associated "
                                  "extensions (comma- or space-separated)."),
                   this);
    info->setWordWrap(true);
    layout->addWidget(info);

    table = new QTableWidget(this);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(
        {QStringLiteral("Folder"), QStringLiteral("Extensions")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(table);

    auto* buttonsRow = new QHBoxLayout();
    addBtn = new QPushButton(QStringLiteral("Add"), this);
    removeBtn = new QPushButton(QStringLiteral("Remove"), this);
    browseBtn = new QPushButton(QStringLiteral("Browse Folder..."), this);
    buttonsRow->addWidget(addBtn);
    buttonsRow->addWidget(removeBtn);
    buttonsRow->addStretch();
    buttonsRow->addWidget(browseBtn);
    layout->addLayout(buttonsRow);

    auto* actionRow = new QHBoxLayout();
    auto* cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    auto* okBtn = new QPushButton(QStringLiteral("Save"), this);
    actionRow->addStretch();
    actionRow->addWidget(cancelBtn);
    actionRow->addWidget(okBtn);
    layout->addLayout(actionRow);

    connect(addBtn, &QPushButton::clicked, this, &SettingsDialog::onAddRow);
    connect(removeBtn, &QPushButton::clicked, this,
            &SettingsDialog::onRemoveRow);
    connect(browseBtn, &QPushButton::clicked, this,
            &SettingsDialog::onBrowseFolder);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, this, &SettingsDialog::onAccept);
}

inline void SettingsDialog::addRow(const QString& folder,
                                   const QStringList& exts) {
    const int row = table->rowCount();
    table->insertRow(row);
    auto* folderItem = new QTableWidgetItem(folder);
    auto* extsItem = new QTableWidgetItem(exts.join(", "));
    table->setItem(row, 0, folderItem);
    table->setItem(row, 1, extsItem);
}

inline QStringList SettingsDialog::parseExtensions(const QString& text) {
    QString t = text;
    t.replace(';', ' ').replace(',', ' ');
    QStringList parts = t.split(' ', Qt::SkipEmptyParts);
    for (QString& s : parts)
        s = s.trimmed().toLower();
    parts.removeAll(QString());
    parts.removeDuplicates();
    return parts;
}

inline void SettingsDialog::setMappings(
    const QMap<QString, QList<QString>>& map) {
    table->setRowCount(0);
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        addRow(it.key(), it.value());
    }
}

inline QMap<QString, QList<QString>> SettingsDialog::mappings() const {
    QMap<QString, QList<QString>> out;
    for (int r = 0; r < table->rowCount(); ++r) {
        const QString folder =
            table->item(r, 0) ? table->item(r, 0)->text().trimmed() : QString();
        const QString extsRaw =
            table->item(r, 1) ? table->item(r, 1)->text() : QString();
        if (folder.isEmpty())
            continue;
        const QStringList exts = parseExtensions(extsRaw);
        if (exts.isEmpty())
            continue;
        out.insert(folder, exts);
    }
    return out;
}

inline QMap<QString, QList<QString>> SettingsDialog::getMappings(
    QWidget* parent,
    const QMap<QString, QList<QString>>& initial,
    bool* ok) {
    SettingsDialog dlg(parent);
    dlg.setMappings(initial);
    const int rc = dlg.exec();
    if (ok)
        *ok = (rc == QDialog::Accepted);
    if (rc == QDialog::Accepted) {
        return dlg.mappings();
    }
    return initial;
}

inline void SettingsDialog::onAddRow() {
    addRow(QStringLiteral("New Folder"), {});
}

inline void SettingsDialog::onRemoveRow() {
    int row = table->currentRow();
    if (row >= 0)
        table->removeRow(row);
}

inline void SettingsDialog::onBrowseFolder() {
    int row = table->currentRow();
    if (row < 0)
        return;
    const QString dir = QFileDialog::getExistingDirectory(
        this, QStringLiteral(
                  "Select (relative) folder under your download directory"));
    if (dir.isEmpty())
        return;
    QString name = dir;
    QDir d(dir);
    if (d.isAbsolute())
        name = d.dirName();
    if (!table->item(row, 0))
        table->setItem(row, 0, new QTableWidgetItem());
    table->item(row, 0)->setText(name);
}

inline void SettingsDialog::onAccept() {
    accept();
}
