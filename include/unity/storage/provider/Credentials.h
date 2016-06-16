#pragma once

#include <unity/storage/provider/visibility.h>

#include <boost/blank.hpp>
#include <boost/variant.hpp>

#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

struct STORAGE_PROVIDER_EXPORT NoCredentials
{
};

struct STORAGE_PROVIDER_EXPORT OAuth1Credentials
{
    std::string consumer_key;
    std::string consumer_secret;
    std::string token;
    std::string token_secret;
};

struct STORAGE_PROVIDER_EXPORT OAuth2Credentials
{
    std::string access_token;
};

struct STORAGE_PROVIDER_EXPORT PasswordCredentials
{
    std::string username;
    std::string password;
};

typedef boost::variant<boost::blank,OAuth1Credentials,OAuth2Credentials,PasswordCredentials> Credentials;

}
}
}
