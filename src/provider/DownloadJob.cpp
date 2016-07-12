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

#include <unity/storage/provider/DownloadJob.h>
#include <unity/storage/provider/internal/DownloadJobImpl.h>

#include <QCoreApplication>

using namespace std;

namespace unity
{
namespace storage
{
namespace provider
{

DownloadJob::DownloadJob(internal::DownloadJobImpl *p)
    : p_(p)
{
    // We may be created by user code running in some other thread:
    // make sure our events are processed on the event loop thread,
    // and then let the class complete its initialisation on that
    // thread.
    p_->moveToThread(QCoreApplication::instance()->thread());
    QMetaObject::invokeMethod(p_, "complete_init", Qt::QueuedConnection);
}

DownloadJob::DownloadJob(string const& download_id)
    : DownloadJob(new internal::DownloadJobImpl(download_id))
{
}

DownloadJob::~DownloadJob()
{
    if (p_)
    {
        p_->deleteLater();
    }
}

string const& DownloadJob::download_id() const
{
    return p_->download_id();
}

int DownloadJob::write_socket() const
{
    return p_->write_socket();
}

void DownloadJob::report_complete()
{
    p_->report_complete();
}

void DownloadJob::report_error(std::exception_ptr p)
{
    p_->report_error(p);
}

}
}
}
