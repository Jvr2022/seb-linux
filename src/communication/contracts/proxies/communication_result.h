#pragma once

namespace seb::communication::contracts::proxies {

class CommunicationResult
{
public:
    explicit CommunicationResult(bool ok)
        : success(ok) {}

    bool success = false;
};

template <typename T>
class TypedCommunicationResult : public CommunicationResult
{
public:
    TypedCommunicationResult(bool ok, T responseValue)
        : CommunicationResult(ok), value(std::move(responseValue)) {}

    T value;
};

}  // namespace seb::communication::contracts::proxies
