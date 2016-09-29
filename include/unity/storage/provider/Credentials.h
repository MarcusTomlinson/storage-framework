/*
 * Copyright (C) 2016 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/storage/visibility.h>

#include <boost/blank.hpp>
#include <boost/variant.hpp>

#include <string>

namespace unity
{
namespace storage
{
namespace provider
{

struct UNITY_STORAGE_EXPORT NoCredentials
{
};

struct UNITY_STORAGE_EXPORT OAuth1Credentials
{
    std::string consumer_key;
    std::string consumer_secret;
    std::string token;
    std::string token_secret;
};

struct UNITY_STORAGE_EXPORT OAuth2Credentials
{
    std::string access_token;
};

struct UNITY_STORAGE_EXPORT PasswordCredentials
{
    std::string username;
    std::string password;
    std::string host;
};

typedef boost::variant<boost::blank,OAuth1Credentials,OAuth2Credentials,PasswordCredentials> Credentials;

}
}
}
