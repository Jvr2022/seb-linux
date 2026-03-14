#pragma once

#include <QString>

namespace seb::browser::content {

class ContentLoader
{
public:
    QString loadApi(const QString &browserExamKey, const QString &configurationKey, const QString &version);
    QString loadBlockedContent() const;
    QString loadBlockedPage() const;
    QString loadClipboard();
    QString loadPageZoom();

private:
    static QString load(const QString &resourcePath);

    QString api_;
    QString clipboard_;
    QString pageZoom_;
};

}  // namespace seb::browser::content
