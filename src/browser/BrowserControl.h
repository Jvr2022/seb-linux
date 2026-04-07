#pragma once

#include "webengine_compat.h"

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QWebEngineView;
QT_END_NAMESPACE

namespace seb::browser {

class BrowserControl : public QObject
{
    Q_OBJECT

public:
    explicit BrowserControl(QWebEngineView *view, QObject *parent = nullptr);

    QString address() const;
    bool canNavigateBackwards() const;
    bool canNavigateForwards() const;
    void navigateTo(const QString &address);
    void navigateBackwards();
    void navigateForwards();
    void reload();

private:
    QWebEngineView *view_ = nullptr;
};

}  // namespace seb::browser
