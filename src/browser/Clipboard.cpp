#include "Clipboard.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

namespace seb::browser {

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
{
    timer_.setInterval(50);
    connect(&timer_, &QTimer::timeout, this, &Clipboard::clearSystemClipboard);
    connect(QApplication::clipboard(), &QClipboard::changed, this, [this](QClipboard::Mode) {
        if (active_ && policy_ != seb::settings::security::ClipboardPolicy::Allow) clearSystemClipboard();
    });
}

Clipboard::~Clipboard()
{
    terminate();
}

void Clipboard::initialize(seb::settings::security::ClipboardPolicy policy)
{
    terminate();
    policy_ = policy;
    active_ = true;
    update(QString());
    clearSystemClipboard();
    if (policy != seb::settings::security::ClipboardPolicy::Allow) timer_.start();
}

void Clipboard::terminate()
{
    if (!active_) return;
    timer_.stop();
    clearSystemClipboard();
    active_ = false;
    update(QString());
}

void Clipboard::clearSystemClipboard()
{
    if (clearing_ || !active_) return;
    clearing_ = true;
    auto *clipboard = QApplication::clipboard();
    const auto clear = [clipboard](QClipboard::Mode mode) {
        const auto *data = clipboard->mimeData(mode);
        if (data && !data->formats().isEmpty()) clipboard->clear(mode);
    };
    clear(QClipboard::Clipboard);
    if (clipboard->supportsSelection()) clear(QClipboard::Selection);
    if (clipboard->supportsFindBuffer()) clear(QClipboard::FindBuffer);
    clearing_ = false;
}

QString Clipboard::content() const
{
    return content_;
}

void Clipboard::update(const QString &content, const QString &sourceId)
{
    if (content_ == content) {
        return;
    }
    content_ = content;
    emit changed(content_, sourceId);
}

}  // namespace seb::browser
