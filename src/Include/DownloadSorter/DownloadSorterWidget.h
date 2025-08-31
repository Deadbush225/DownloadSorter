#pragma once
#include <QList>
#include <QMap>
#include <QString>
#include <QWidget>

class QPushButton;
class QProgressBar;
// Forward-declare global class (no namespace)
class DownloadSorter;

class DownloadSorterWidget : public QWidget {
    Q_OBJECT
   public:
    explicit DownloadSorterWidget(DownloadSorter* sorter,
                                  QWidget* parent = nullptr);

   signals:
    void mappingsChanged(const QMap<QString, QList<QString>>&);

   private:
    DownloadSorter* sorter = nullptr;
    QPushButton* configureBtn = nullptr;
    QProgressBar* progressBar = nullptr;

    void loadInitialMappings();
    void connectProgressSignals();
    void openSettings();
    void applyMappings(const QMap<QString, QList<QString>>& map);
};
