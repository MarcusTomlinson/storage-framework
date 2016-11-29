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

#pragma once

#include <unity/storage/internal/InactivityTimer.h>

#include <cassert>
#include <memory>

namespace unity
{
namespace storage
{
namespace internal
{

class InactivityTimer;

class ActivityNotifier
{
public:
    ActivityNotifier(std::shared_ptr<InactivityTimer> const& timer)
        : timer_(timer)
    {
        assert(timer);

        timer_->request_started();
    }

    ~ActivityNotifier()
    {
        timer_->request_finished();
    }

private:
    std::shared_ptr<InactivityTimer> timer_;
};

}  // namespace internal
}  // namespace storage
}  // namespace unity
