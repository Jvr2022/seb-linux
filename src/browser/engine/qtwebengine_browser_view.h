#pragma once

#include "browser_view.h"

#include <QPointer>

QT_BEGIN_NAMESPACE
class QWebEnginePage;
class QWebEngineProfile;
class QWebEngineView;
QT_END_NAMESPACE

class SebSession;

namespace seb::browser::engine {

class QtWebEngineBrowserPage final : public QWebEnginePage
{
public:
    QtWebEngineBrowserPage(SebSession &session, QObject *parent = nullptr);

    void setNavigationPolicy(BrowserView::NavigationPolicy policy);
    void setUploadsAllowed(bool allowed);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
    QStringList chooseFiles(
        FileSelectionMode mode,
        const QStringList &oldFiles,
        const QStringList &acceptedMimeTypes) override;

private:
    SebSession &session_;
    BrowserView::NavigationPolicy navigationPolicy_;
    bool uploadsAllowed_ = true;
};

class QtWebEngineBrowserView final : public BrowserView
{
    Q_OBJECT

public:
    QtWebEngineBrowserView(SebSession &session, QWebEngineProfile *profile, QWidget *parentWidget, QObject *parent = nullptr);
    ~QtWebEngineBrowserView() override;

    QWidget *widget() override;

    QUrl url() const override;
    QString title() const override;

    bool canGoBack() const override;
    bool canGoForward() const override;

    void load(const QUrl &url) override;
    void back() override;
    void forward() override;
    void reload() override;

    void setNavigationPolicy(NavigationPolicy policy) override;
    void setUploadsAllowed(bool allowed) override;
    void findText(const QString &text) override;
    void openDevTools() override;

    QWebEnginePage *page() const;

private:
    QPointer<QWebEngineView> view_;
    QPointer<QtWebEngineBrowserPage> page_;
};

}  // namespace seb::browser::engine

