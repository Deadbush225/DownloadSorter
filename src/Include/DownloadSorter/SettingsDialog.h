#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QList>
#include <QMap>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QListWidget;
class QTableWidget;

class SettingsDialog : public QDialog {
    Q_OBJECT

   public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

    void setMappings(const QMap<QString, QList<QString>>& mappings);
    QMap<QString, QList<QString>> getMappings() const;
    void setIgnorePatterns(const QList<QString>& patterns);
    QList<QString> getIgnorePatterns() const;

    static bool getSettings(QWidget* parent,
                            QMap<QString, QList<QString>>& mappings,
                            QList<QString>& ignorePatterns);
    static bool editSettings(QWidget* parent);

   private slots:
    void addMapping();
    void removeMapping();
    void addIgnorePattern();
    void removeIgnorePattern();

   private:
    QTableWidget* mappingsTable;
    QListWidget* ignoreList;
    QPushButton* addMappingBtn;
    QPushButton* removeMappingBtn;
    QPushButton* addIgnoreBtn;
    QPushButton* removeIgnoreBtn;
};

#endif  // SETTINGSDIALOG_H
