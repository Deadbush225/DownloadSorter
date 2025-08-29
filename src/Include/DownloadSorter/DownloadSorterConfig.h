#pragma once
#include <QList>
#include <QMap>
#include <QString>

namespace DownloadSorterConfig {
QMap<QString, QList<QString>> loadMappings();
bool saveMappings(const QMap<QString, QList<QString>>& map);
}  // namespace DownloadSorterConfig
