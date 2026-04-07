#pragma once

#include <QObject>
#include <QtGlobal>
#include <QUrl>
#include <functional>

QT_BEGIN_NAMESPACE
class QWidget;
class QAuthenticator;
QT_END_NAMESPACE

namespace seb::browser::contracts {

class IWebView : public QObject
{
    Q_OBJECT

public:
    explicit IWebView(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IWebView() = default;

    /*!
     * \brief Returns the actual Qt widget that can be inserted into the UI layout.
     * For WebEngine this is the QWebEngineView. For WebKitGTK this is a QWindow container widget.
     */
    virtual QWidget *widget() const = 0;

    virtual QUrl url() const = 0;
    virtual void setUrl(const QUrl &url) = 0;

    virtual void back() = 0;
    virtual void forward() = 0;
    virtual void reload() = 0;
    virtual void openDevTools() = 0;
    virtual bool canGoBack() const = 0;
    virtual bool canGoForward() const = 0;

    virtual void findText(const QString &text) = 0;

    /*!
     * \brief Allows injecting a delegate to approve/block top-level navigations.
     */
    using NavigationRequestDelegate = std::function<bool(const QUrl &url, bool isMainFrame)>;
    virtual void setNavigationRequestDelegate(NavigationRequestDelegate delegate) = 0;

signals:
    void urlChanged(const QUrl &url);
    void titleChanged(const QString &title);
    void newWindowRequested(const QUrl &targetUrl, bool &handled);
    void proxyAuthenticationRequired(const QUrl &url, QAuthenticator *authenticator, const QString &proxyHost);
};

}  // namespace seb::browser::contracts
