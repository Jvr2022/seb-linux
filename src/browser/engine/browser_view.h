#pragma once

#include <functional>

#include <QObject>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace seb::browser::engine {

class BrowserView : public QObject
{
    Q_OBJECT

public:
    using NavigationPolicy = std::function<bool(const QUrl &url, bool isMainFrame)>;
    using FileChooserPolicy = std::function<bool()>;

    explicit BrowserView(QObject *parent = nullptr) : QObject(parent) {}
    ~BrowserView() override = default;

    virtual QWidget *widget() = 0;

    virtual QUrl url() const = 0;
    virtual QString title() const = 0;

    virtual bool canGoBack() const = 0;
    virtual bool canGoForward() const = 0;

    virtual void load(const QUrl &url) = 0;
    virtual void back() = 0;
    virtual void forward() = 0;
    virtual void reload() = 0;

    virtual void setNavigationPolicy(NavigationPolicy policy) = 0;
    virtual void setUploadsAllowed(bool allowed) = 0;

    // Find-in-page. Passing empty string should clear highlights.
    virtual void findText(const QString &text) = 0;

    // Optional; backends which cannot support devtools should no-op.
    virtual void openDevTools() = 0;

signals:
    void urlChanged(const QUrl &url);
    void titleChanged(const QString &title);
    void newWindowRequested(const QUrl &targetUrl, const QUrl &openerUrl);
};

}  // namespace seb::browser::engine

