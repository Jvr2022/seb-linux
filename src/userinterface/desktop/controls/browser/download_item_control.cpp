#include "download_item_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::browser { DownloadItemControl::DownloadItemControl(QWidget *parent) : QWidget(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Download"), this)); } }
