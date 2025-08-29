#ifndef SUBCLASS_H
#define SUBCLASS_H

#include <QtCore/QString>
#include <QtWidgets/QLabel>

#include <QtCore/Qt>
#include <QtWidgets/QSizePolicy>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

class ModQLabel : public QLabel {
   public:
    ModQLabel(QString str);
};

class ModQVBoxLayout : public QVBoxLayout {
   public:
    ModQVBoxLayout();
};

class ModQHBoxLayout : public QHBoxLayout {
   public:
    ModQHBoxLayout();
};

#endif