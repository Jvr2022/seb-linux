#pragma once

#include <QByteArray>
#include <QUrl>

namespace seb::browser::contracts {

enum class ResourceType {
    MainFrame,
    NavigationPreloadMainFrame,
    SubFrame,
    Other
};

class IRequest {
public:
    virtual ~IRequest() = default;

    virtual QUrl requestUrl() const = 0;
    virtual QUrl firstPartyUrl() const = 0;
    virtual ResourceType resourceType() const = 0;

    virtual void block(bool shouldBlock) = 0;
    virtual void redirect(const QUrl &url) = 0;
    virtual void setHttpHeader(const QByteArray &name, const QByteArray &value) = 0;
};

class IRequestInterceptor {
public:
    virtual ~IRequestInterceptor() = default;

    virtual void interceptRequest(IRequest &request) = 0;
};

}  // namespace seb::browser::contracts
