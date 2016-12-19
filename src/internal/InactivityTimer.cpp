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
 * Authors: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/storage/internal/InactivityTimer.h>

#include <QDebug>

#include <cassert>

namespace unity
{
namespace storage
{
namespace internal
{

InactivityTimer::InactivityTimer(int timeout_ms)
    : timeout_ms_(timeout_ms)
    , num_requests_(0)
{
    assert(timeout_ms_ >= 0);

    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, &InactivityTimer::timeout);
}

InactivityTimer::~InactivityTimer() = default;

void InactivityTimer::request_started()
{
    assert(num_requests_ >= 0);

    if (num_requests_++ == 0)
    {
        timer_.stop();
    }
}

void InactivityTimer::request_finished()
{
    assert(num_requests_ > 0);

    if (--num_requests_ == 0)
    {
        timer_.start(timeout_ms_);
    }
}

} // namespace internal
} // namespace storage
} // namespace unity
