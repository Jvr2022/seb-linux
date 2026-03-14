#pragma once

#include "BrowserWindowContext.h"
#include "contracts/IBrowserWindow.h"

#include <QObject>

class BrowserWindow;

namespace seb::browser {

class BrowserWindowPort : public QObject, public contracts::IBrowserWindow
{
    Q_OBJECT

public:
    explicit BrowserWindowPort(::BrowserWindow *window, QObject *parent = nullptr);

    QString handle() const override;
    QString iconPath() const override;
    QString title() const override;
    void activate() override;
    bool isMainWindow() const override;
    QString url() const override;
    BrowserWindowContext context() const;

private:
    ::BrowserWindow *window_ = nullptr;
};

}  // namespace seb::browser
