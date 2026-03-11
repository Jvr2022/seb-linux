#pragma once

#include "../seb_settings.h"

#include <QString>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

namespace seb::browser {

class KeyGenerator
{
public:
    explicit KeyGenerator(const seb::SebSettings &settings);

    QByteArray configurationKeyHash(const QUrl &url) const;
    QByteArray requestHash(const QUrl &url) const;

private:
    QString browserExamKey() const;
    QString buildVersion() const;
    QString codeSignatureHash() const;
    QString executableHash() const;
    QString normalizeUrl(const QUrl &url) const;

    seb::SebSettings settings_;
    mutable QString browserExamKey_;
    mutable QString executableHash_;
};

}  // namespace seb::browser
