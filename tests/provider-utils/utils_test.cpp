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

using namespace unity::storage::provider;

TEST(UtilsTest, convert_exception_ptr)
{
    std::string const exc_message = "exception message";
    int const exc_code = 42;

    std::exception_ptr sep = std::make_exception_ptr(
        ResourceException(exc_message, exc_code));

    boost::exception_ptr bep = internal::convert_exception_ptr(sep);
    try
    {
        boost::rethrow_exception(bep);
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_EQ(exc_message, e.error_message());
        EXPECT_EQ(exc_code, e.error_code());
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
