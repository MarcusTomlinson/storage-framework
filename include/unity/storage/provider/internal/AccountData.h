#pragma once

#include <unity/storage/provider/Credentials.h>
#include <unity/storage/provider/visibility.h>

#include <OnlineAccounts/Account>
#include <OnlineAccounts/PendingCallWatcher>
#include <QObject>
#include <QDBusConnection>

#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

class ProviderBase;

namespace internal
{

class CredentialsCache;
class PendingJobs;

class AccountData : public QObject
{
    Q_OBJECT
public:
    AccountData(std::unique_ptr<ProviderBase>&& provider,
                std::shared_ptr<CredentialsCache> const& credentials,
                QDBusConnection const& bus,
                OnlineAccounts::Account* account,
                QObject* parent=nullptr);
    virtual ~AccountData();

    void authenticate(bool interactive);
    bool has_credentials();
    Credentials const& credentials();

    ProviderBase& provider();
    CredentialsCache& dbus_creds();
    PendingJobs& jobs();

Q_SIGNALS:
    void authenticated();

private Q_SLOTS:
    void on_authenticated();

private:
    std::unique_ptr<ProviderBase> const provider_;
    std::shared_ptr<CredentialsCache> const dbus_creds_;
    std::shared_ptr<PendingJobs> const jobs_;

    OnlineAccounts::Account* const account_;
    std::unique_ptr<OnlineAccounts::PendingCallWatcher> auth_watcher_;
    bool authenticating_interactively_ = false;

    bool credentials_valid_ = false;
    Credentials credentials_;

    Q_DISABLE_COPY(AccountData)
};

}
}
}
}
