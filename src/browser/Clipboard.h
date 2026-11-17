#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include "settings/Security/ClipboardPolicy.h"

namespace seb::browser {

class Clipboard : public QObject
{
    Q_OBJECT

public:
    explicit Clipboard(QObject *parent = nullptr);

    ~Clipboard() override;
    void initialize(seb::settings::security::ClipboardPolicy policy);
    void terminate();
    QString content() const;
    void update(const QString &content, const QString &sourceId = {});

signals:
    void changed(const QString &content, const QString &sourceId);

private:
    void clearSystemClipboard();
    QString content_;
    QTimer timer_;
    seb::settings::security::ClipboardPolicy policy_ = seb::settings::security::ClipboardPolicy::Allow;
    bool active_ = false;
    bool clearing_ = false;
};

}  // namespace seb::browser
