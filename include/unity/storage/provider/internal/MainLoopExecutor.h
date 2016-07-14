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

#include <boost/thread/executor.hpp>
#include <QObject>

#include <functional>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

class MainLoopExecutor : public QObject, public boost::executors::executor {
    Q_OBJECT
public:
    static MainLoopExecutor& instance();

    void submit(work&& closure) override;
    void close() override;
    bool closed() override;
    bool try_executing_one() override;

    bool event(QEvent *event) override;

private:
    MainLoopExecutor();
    void execute(work& closure) noexcept;

    Q_DISABLE_COPY(MainLoopExecutor)
};

}
}
}
}
