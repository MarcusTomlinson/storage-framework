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

#include <unity/storage/provider/internal/UploadJobImpl.h>

#include <QLocalSocket>
#include <QTemporaryFile>

#include <memory>
#include <string>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

class TempfileUploadJobImpl : public UploadJobImpl
{
    Q_OBJECT
public:
    explicit TempfileUploadJobImpl(std::string const& upload_id);
    virtual ~TempfileUploadJobImpl();

    void complete_init() override;

    std::string file_name() const;

private Q_SLOTS:
    void on_ready_read();
    void on_read_channel_finished();

private:
    std::unique_ptr<QLocalSocket> reader_;
    std::unique_ptr<QTemporaryFile> tmpfile_;

    Q_DISABLE_COPY(TempfileUploadJobImpl)
};

}
}
}
}
