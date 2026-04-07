#pragma once

#include <QObject>
#include <QString>

namespace seb::browser::engine {
class BrowserView;
}

namespace seb::browser {

class BrowserControl : public QObject
{
    Q_OBJECT

public:
    explicit BrowserControl(seb::browser::engine::BrowserView *view, QObject *parent = nullptr);

    QString address() const;
    bool canNavigateBackwards() const;
    bool canNavigateForwards() const;
    void navigateTo(const QString &address);
    void navigateBackwards();
    void navigateForwards();
    void reload();

private:
    seb::browser::engine::BrowserView *view_ = nullptr;
};

}  // namespace seb::browser
