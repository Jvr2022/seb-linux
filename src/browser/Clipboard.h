#pragma once

#include <QObject>
#include <QString>

namespace seb::browser {

class Clipboard : public QObject
{
    Q_OBJECT

public:
    explicit Clipboard(QObject *parent = nullptr);

    QString content() const;
    void update(const QString &content);

signals:
    void changed(const QString &content);

private:
    QString content_;
};

}  // namespace seb::browser
