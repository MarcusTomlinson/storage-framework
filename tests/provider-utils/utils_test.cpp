/*
 * Copyright (C) 2017 Canonical Ltd
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

#include <unity/storage/provider/Exceptions.h>
#include <unity/storage/provider/internal/utils.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <stdexcept>
#include <string>

using namespace unity::storage::provider;

// Helper function to create exceptions, with specialisations for
// types with irregular constructors.
template <typename Exception>
Exception make_exception(std::string const& message)
{
    return Exception(message);
}

template<>
NotExistsException make_exception<NotExistsException>(std::string const& message)
{
    return NotExistsException(message, "key");
}

template<>
ExistsException make_exception<ExistsException>(std::string const& message)
{
    return ExistsException(message, "item_id", "name");
}

template<>
ResourceException make_exception<ResourceException>(std::string const& message)
{
    return ResourceException(message, 0);
}

template <typename Ex>
class ConvertExceptionTest : public ::testing::Test {
};

using ExceptionTypes = ::testing::Types<
    RemoteCommsException,
    NotExistsException,
    ExistsException,
    ConflictException,
    UnauthorizedException,
    PermissionException,
    QuotaException,
    CancelledException,
    LogicException,
    InvalidArgumentException,
    ResourceException,
    UnknownException>;

TYPED_TEST_CASE(ConvertExceptionTest, ExceptionTypes);

TYPED_TEST(ConvertExceptionTest, storage_exception)
{
    using Exception = TypeParam;
    std::string const message = "exception message";

    std::exception_ptr sep = std::make_exception_ptr(
        make_exception<Exception>(message));

    boost::exception_ptr bep = internal::convert_exception_ptr(sep);
    try
    {
        boost::rethrow_exception(bep);
        FAIL();
    }
    catch (Exception const& e)
    {
        EXPECT_EQ(message, e.error_message());
    }
}

class CustomException : public std::exception
{
public:
    virtual char const* what() const noexcept override
    {
        return "custom message";
    }
};

// Exceptions raised with boost::enable_current_exception() are preserved
TEST(ConvertExceptionTest, enable_current_exception)
{
    std::exception_ptr sep = std::make_exception_ptr(
        boost::enable_current_exception(CustomException()));

    boost::exception_ptr bep = internal::convert_exception_ptr(sep);
    try
    {
        boost::rethrow_exception(bep);
        FAIL();
    }
    catch (CustomException const& e)
    {
        EXPECT_EQ("custom message", e.what());
    }
}

TEST(ConvertExceptionTest, unknown_exception)
{
    std::exception_ptr sep = std::make_exception_ptr(CustomException());

    boost::exception_ptr bep = internal::convert_exception_ptr(sep);
    try
    {
        boost::rethrow_exception(bep);
        FAIL();
    }
    catch (CustomException const& e)
    {
        // Boost can't convert exceptions that haven't been annotated
        // with enable_current_exception().
        FAIL();
    }
    catch (boost::unknown_exception const& e)
    {
        EXPECT_EQ("std::exception", std::string(e.what()));
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
