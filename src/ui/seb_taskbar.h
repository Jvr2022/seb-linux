#pragma once

#include "../seb_settings.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QHBoxLayout;
class QTimer;
QT_END_NAMESPACE

class SebTaskbar : public QWidget
{
    Q_OBJECT

public:
    explicit SebTaskbar(const seb::SebSettings &settings, bool isMainWindow, QWidget *parent = nullptr);

    void setCurrentUrl(const QUrl &url);
    void setWindowTitleText(const QString &title);

signals:
    void backRequested();
    void findRequested();
    void forwardRequested();
    void homeRequested();
    void quitRequested();
    void reloadRequested();

private:
    void refreshClock();

    QWidget *applicationArea_ = nullptr;
    QHBoxLayout *applicationLayout_ = nullptr;
    QWidget *metaArea_ = nullptr;
    QHBoxLayout *metaLayout_ = nullptr;
    QLabel *titleLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QLabel *clockLabel_ = nullptr;
    QTimer *clockTimer_ = nullptr;
};
