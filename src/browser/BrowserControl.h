#pragma once

#include "contracts/i_webview.h"
#include <QObject>
#include <QString>

namespace seb::browser {

class BrowserControl : public QObject
{
    Q_OBJECT

public:
    explicit BrowserControl(contracts::IWebView *view, QObject *parent = nullptr);

    QString address() const;
    bool canNavigateBackwards() const;
    bool canNavigateForwards() const;
    void navigateTo(const QString &address);
    void navigateBackwards();
    void navigateForwards();
    void reload();

private:
    contracts::IWebView *view_ = nullptr;
};

}  // namespace seb::browser
