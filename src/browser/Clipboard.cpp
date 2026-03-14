#include "Clipboard.h"

namespace seb::browser {

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
{
}

QString Clipboard::content() const
{
    return content_;
}

void Clipboard::update(const QString &content)
{
    if (content_ == content) {
        return;
    }
    content_ = content;
    emit changed(content_);
}

}  // namespace seb::browser
