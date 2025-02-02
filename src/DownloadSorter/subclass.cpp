#include "../Include/DownloadSorter/subclass.h"

/* +++++ ModQLabel +++++ */
ModQLabel::ModQLabel(QString str) : QLabel(str) {
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    this->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

/* +++++ ModQVBoxLayout +++++ */
ModQVBoxLayout::ModQVBoxLayout() {
    // this->setContentsMargins(10, 5, 10, 5);
    // this->setContentsMargins(0, 0, 0, 0);
    this->setSpacing(3);
    // this->setMargins(5);
}

/* +++++ ModQHBoxLayout +++++ */
ModQHBoxLayout::ModQHBoxLayout() {
    // this->setContentsMargins(0, 0, 0, 0);
    this->setSpacing(5);
}
