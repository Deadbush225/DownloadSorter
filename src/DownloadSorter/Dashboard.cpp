#include "../Include/DownloadSorter/Dashboard.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include "../Include/DownloadSorter/SettingsDialog.h"

// Persist helpers for mappings (legacy helpers kept but not used directly)
namespace {
QString mappingsConfigPath() {
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(base);
    dir.mkpath("DownloadSorter");
    return dir.filePath("DownloadSorter/mappings.json");
}

QJsonDocument readDoc() {
    QFile f(mappingsConfigPath());
    if (!f.exists()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("defaults"), QJsonArray());
        obj.insert(QStringLiteral("custom"), QJsonArray());
        return QJsonDocument(obj);
    }
    if (!f.open(QIODevice::ReadOnly)) {
        QJsonObject obj;
        obj.insert(QStringLiteral("defaults"), QJsonArray());
        obj.insert(QStringLiteral("custom"), QJsonArray());
        return QJsonDocument(obj);
    }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) {
        QJsonObject obj;
        obj.insert(QStringLiteral("defaults"), QJsonArray());
        obj.insert(QStringLiteral("custom"), QJsonArray());
        return QJsonDocument(obj);
    }
    return doc;
}

bool writeDoc(const QJsonDocument& doc) {
    QFile f(mappingsConfigPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const auto bytes = doc.toJson(QJsonDocument::Indented);
    const bool ok = f.write(bytes) == bytes.size();
    f.close();
    return ok;
}

QMap<QString, QList<QString>> readSection(const char* key) {
    QMap<QString, QList<QString>> map;
    const auto doc = readDoc();
    const auto obj = doc.object();
    const auto arr = obj.value(QLatin1String(key)).toArray();
    for (const auto& v : arr) {
        const auto o = v.toObject();
        const QString folder = o.value(QStringLiteral("folder")).toString();
        QList<QString> exts;
        for (const auto& ev : o.value(QStringLiteral("extensions")).toArray()) {
            const auto s = ev.toString().toLower();
            if (!s.isEmpty() && !exts.contains(s))
                exts.push_back(s);
        }
        if (!folder.isEmpty() && !exts.isEmpty())
            map.insert(folder, exts);
    }
    return map;
}

bool writeSection(const char* key, const QMap<QString, QList<QString>>& map) {
    auto doc = readDoc();
    auto obj = doc.object();
    QJsonArray arr;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QJsonObject o;
        o.insert(QStringLiteral("folder"), it.key());
        QJsonArray exts;
        for (const auto& e : it.value())
            exts.push_back(e.toLower());
        o.insert(QStringLiteral("extensions"), exts);
        arr.push_back(o);
    }
    obj.insert(QLatin1String(key), arr);
    return writeDoc(QJsonDocument(obj));
}

// Defaults to seed if settings are empty (used once)
static QMap<QString, QList<QString>> defaultSeed() {
    QMap<QString, QList<QString>> m;
    m[QStringLiteral("Downloaded Archives")] = {"zip", "rar", "7z"};
    m[QStringLiteral("Downloaded Audios")] = {"m4a", "mp3", "wav", "aac"};
    m[QStringLiteral("Downloaded Documents")] = {
        "doc", "docx", "xls", "xlsx", "ppt", "pptx", "odt", "pdf", "rtf"};
    m[QStringLiteral("Downloaded Fonts")] = {"otf", "ttf"};
    m[QStringLiteral("Downloaded Images")] = {"png",  "jpeg", "jpg", "gif",
                                              "tiff", "psd",  "ai",  "eps"};
    m[QStringLiteral("Downloaded Programs")] = {"exe", "msi"};
    m[QStringLiteral("Downloaded Videos")] = {"mp4", "mov", "wmv", "avi",
                                              "mkv"};
    return m;
}

// Active rules: single array format (the one we edit and use)
static QMap<QString, QList<QString>> parseArrayDoc(const QJsonDocument& doc) {
    QMap<QString, QList<QString>> map;
    if (!doc.isArray())
        return map;
    const auto arr = doc.array();
    for (const auto& v : arr) {
        const auto obj = v.toObject();
        const QString folder = obj.value(QStringLiteral("folder")).toString();
        QList<QString> exts;
        for (const auto& ev :
             obj.value(QStringLiteral("extensions")).toArray()) {
            const auto s = ev.toString().toLower();
            if (!s.isEmpty() && !exts.contains(s))
                exts.push_back(s);
        }
        if (!folder.isEmpty() && !exts.isEmpty())
            map.insert(folder, exts);
    }
    return map;
}

static bool saveActiveMappings(const QMap<QString, QList<QString>>& map) {
    QJsonArray arr;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QJsonObject obj;
        obj.insert(QStringLiteral("folder"), it.key());
        QJsonArray exts;
        for (const auto& e : it.value())
            exts.push_back(e.toLower());
        obj.insert(QStringLiteral("extensions"), exts);
        arr.push_back(obj);
    }
    QFile f(mappingsConfigPath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    const auto bytes = QJsonDocument(arr).toJson(QJsonDocument::Indented);
    const bool ok = f.write(bytes) == bytes.size();
    f.close();
    return ok;
}

static QMap<QString, QList<QString>> loadActiveMappings() {
    QFile f(mappingsConfigPath());
    if (!f.exists()) {
        auto seed = defaultSeed();
        saveActiveMappings(seed);
        return seed;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        auto seed = defaultSeed();
        saveActiveMappings(seed);
        return seed;
    }
    const auto doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    // If already array -> use it
    if (doc.isArray()) {
        const auto map = parseArrayDoc(doc);
        if (!map.isEmpty())
            return map;
        // Empty array -> seed
        auto seed = defaultSeed();
        saveActiveMappings(seed);
        return seed;
    }

    // Legacy object (defaults/custom) -> convert once to active array
    if (doc.isObject()) {
        const auto custom = readSection("custom");
        const auto defs = readSection("defaults");
        const auto chosen = custom.isEmpty() ? defs : custom;
        if (!chosen.isEmpty()) {
            saveActiveMappings(chosen);
            return chosen;
        }
    }

    // Fallback: seed
    auto seed = defaultSeed();
    saveActiveMappings(seed);
    return seed;
}
}  // namespace

Dashboard::Dashboard() {
    // ++ Retrieving settings

    QString retrieved_path = this->settings->value("Download Path").toString();
    if (retrieved_path.isEmpty()) {
        this->currentDownloadFolder = QString("Not Set");
    } else {
        this->currentDownloadFolder = retrieved_path;
    }

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
    QObject::connect(search_btn, QPushButton::clicked, this,
                     Dashboard::browseDownloadFolder);
    // QPushButton* search_btn = new QPushButton("");

    QPushButton* arrangeButton = new QPushButton("Begin Sort");
    QObject::connect(arrangeButton, &QPushButton::clicked, this,
                     &Dashboard::initiateSort);

    // Single "Edit Rules..." button uses active rules (defaults seeded only if
    // empty)
    QPushButton* editRulesBtn = new QPushButton("Edit Rules...");
    QObject::connect(editRulesBtn, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        auto current = loadActiveMappings();  // ensures defaults if empty and
                                              // converts legacy
        auto updated = SettingsDialog::getMappings(this, current, &ok);
        if (ok) {
            if (saveActiveMappings(updated)) {
                this->statusBar()->showMessage("Rules saved.", 3000);
            } else {
                this->statusBar()->showMessage("Failed to save rules.", 3000);
            }
        }
    });

    arrangeButton->setFixedHeight(40);

    this->pathField->setText(this->currentDownloadFolder);

    searcherlayout->addWidget(this->pathField);
    searcherlayout->addWidget(search_btn);

    browserlayout->addWidget(label);
    browserlayout->addLayout(searcherlayout);
    // Place a single edit button under the path chooser
    browserlayout->addWidget(editRulesBtn);
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
    QObject::connect(ds, &DownloadSorter::finished, this,
                     &Dashboard::downloadFinished);

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
