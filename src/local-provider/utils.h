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
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <boost/filesystem.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QFileDevice>
#include <QLocalSocket>
#include <QString>
#pragma GCC diagnostic pop

#include <string>

constexpr char const* TMPFILE_PREFIX = ".storage-framework";

int64_t get_mtime_nsecs(std::string const& method, std::string const& path);
bool is_reserved_path(boost::filesystem::path const& path);
boost::filesystem::path sanitize(std::string const& method, std::string const& name);

[[ noreturn ]]
void throw_exception(std::string const& method, boost::filesystem::filesystem_error const& e);

[[ noreturn ]]
void throw_exception(std::string const& method, std::string const& msg, QFileDevice::FileError e);

[[ noreturn ]]
void throw_exception(std::string const& method, std::string const& msg, QLocalSocket::LocalSocketError e);
