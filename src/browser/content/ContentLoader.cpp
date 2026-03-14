#include "ContentLoader.h"

#include <QFile>

namespace seb::browser::content {

QString ContentLoader::loadApi(const QString &browserExamKey, const QString &configurationKey, const QString &version)
{
    if (api_.isEmpty()) {
        api_ = load(QStringLiteral(":/browser/content/Api.js"));
    }

    QString js = api_;
    js.replace(QStringLiteral("%%_BEK_%%"), browserExamKey);
    js.replace(QStringLiteral("%%_CK_%%"), configurationKey);
    js.replace(QStringLiteral("%%_VERSION_%%"), version);
    return js;
}

QString ContentLoader::loadBlockedContent() const
{
    QString html = load(QStringLiteral(":/browser/content/BlockedContent.html"));
    html.replace(QStringLiteral("%%MESSAGE%%"), QStringLiteral("This content has been blocked by Safe Exam Browser."));
    return html;
}

QString ContentLoader::loadBlockedPage() const
{
    QString html = load(QStringLiteral(":/browser/content/BlockedPage.html"));
    html.replace(QStringLiteral("%%BACK_BUTTON%%"), QStringLiteral("Back"));
    html.replace(QStringLiteral("%%MESSAGE%%"), QStringLiteral("This page has been blocked by Safe Exam Browser."));
    html.replace(QStringLiteral("%%TITLE%%"), QStringLiteral("Blocked Page"));
    return html;
}

QString ContentLoader::loadClipboard()
{
    if (clipboard_.isEmpty()) {
        clipboard_ = load(QStringLiteral(":/browser/content/Clipboard.js"));
    }

    return clipboard_;
}

QString ContentLoader::loadPageZoom()
{
    if (pageZoom_.isEmpty()) {
        pageZoom_ = load(QStringLiteral(":/browser/content/PageZoom.js"));
    }

    return pageZoom_;
}

QString ContentLoader::load(const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

}  // namespace seb::browser::content
