#pragma once

#include <QObject>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace seb::shell::taskbar::platform {

struct KeyboardLayoutState
{
    QString currentShortName;
    QString currentLongName;
    QStringList availableLayouts;
};

class KeyboardController : public QObject
{
    Q_OBJECT

public:
    explicit KeyboardController(QObject *parent = nullptr);

    const KeyboardLayoutState &state() const;
    void refresh();

signals:
    void stateChanged();

private:
    KeyboardLayoutState state_;
    QTimer *timer_ = nullptr;
};

}  // namespace seb::shell::taskbar::platform
