#pragma once

#include <QString>
#include <QStringList>

#include <functional>

QT_BEGIN_NAMESPACE
class QByteArray;
QT_END_NAMESPACE

namespace seb::settingsinternal {

bool looksLikeHtml(const QByteArray &raw);
bool startsWithXml(const QByteArray &data);
QByteArray unwrapSebContainer(
    QByteArray raw,
    QString *error,
    QStringList *warnings,
    const std::function<QString (bool)> &passwordProvider);

}  // namespace seb::settingsinternal
