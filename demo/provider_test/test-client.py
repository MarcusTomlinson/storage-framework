#!/usr/bin/python3

#
# Copyright (C) 2016 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: James Henstridge <james.henstridge@canonical.com>
#

import os
import sys

from gi.repository import Gio, GLib

PROVIDER_BUS_NAME = 'com.canonical.StorageFramework.Provider.ProviderTest'
PROVIDER_IFACE = 'com.canonical.StorageFramework.Provider'

class Provider:
    def __init__(self, bus, bus_name, object_path):
        self._proxy = Gio.DBusProxy.new_sync(
            bus, Gio.DBusProxyFlags.NONE, None,
            bus_name, object_path, PROVIDER_IFACE, None)

    def roots(self):
        return self._proxy.Roots('()')

    def list(self, item_id, page_token):
        return self._proxy.List('(ss)', item_id, page_token)

    def lookup(self, parent_id, name):
        return self._proxy.Lookup('(ss)', parent_id, name)

    def metadata(self, item_id):
        return self._proxy.Metadata('(s)', item_id)

    def create_folder(self, parent_id, name):
        return self._proxy.CreateFolder('(ss)', parent_id, name)

    def create_file(self, parent_id, name, content_type, allow_overwrite):
        args = GLib.Variant('(sssb)', (parent_id, name, content_type, allow_overwrite))
        result, fd_list = self._proxy.call_with_unix_fd_list_sync(
            'CreateFile', args, 0, -1)
        upload_id, fd_idx = result.unpack()
        assert fd_list.get_length() == 1
        assert fd_idx == 0
        return upload_id, fd_list.steal_fds()[fd_idx]

    def update(self, item_id, old_etag=''):
        args = GLib.Variant('(ss)', (item_id, old_etag))
        result, fd_list = self._proxy.call_with_unix_fd_list_sync(
            'Update', args, 0, -1)
        upload_id, fd_idx = result.unpack()
        assert fd_list.get_length() == 1
        assert fd_idx == 0
        return upload_id, fd_list.steal_fds()[fd_idx]

    def cancel_upload(self, upload_id):
        self._proxy.CancelUpload('(s)', upload_id)

    def finish_upload(self, upload_id):
        return self._proxy.FinishUpload('(s)', upload_id)

    def download(self, item_id):
        args = GLib.Variant('(s)', (item_id,))
        result, fd_list = self._proxy.call_with_unix_fd_list_sync(
            'Download', args, 0, -1)
        download_id, fd_idx = result.unpack()
        assert fd_list.get_length() == 1
        assert fd_idx == 0
        return download_id, fd_list.steal_fds()[fd_idx]

    def finish_download(self, download_id):
        return self._proxy.FinishDownload('(s)', download_id)

    def delete(self, item_id):
        self._proxy.Delete('(s)', item_id)

    def move(self, item_id, new_parent_id, new_name):
        return self._proxy.Move('(sss)', item_id, new_parent_id, new_name)

    def copy(self, item_id, new_parent_id, new_name):
        return self._proxy.Copy('(sss)', item_id, new_parent_id, new_name)


def main(argv):
    bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
    provider = Provider(bus, PROVIDER_BUS_NAME, argv[1])

    print("Getting roots...", end='', flush=True)
    roots = provider.roots()
    print(roots)
    print()

    root_id = roots[0][0]

    print("Listing %r..." % root_id, end='', flush=True)
    children, next_token = provider.list(root_id, '')
    print(children)
    print()

    child_id = children[0][0]
    child_name = children[0][2]
    print("Looking up %r under %r..." % (child_name, root_id), end='', flush=True)
    items = provider.lookup(root_id, child_name)
    print(items)
    print()

    print("Getting metadata for %r..." % child_id, end='', flush=True)
    item = provider.metadata(child_id)
    print(item)
    print()

    print("Creating folder...", end='', flush=True)
    provider.create_folder(root_id, 'Some folder')
    print("done")
    print()

    print("Preparing to upload file...", end='', flush=True)
    upload_id, fd = provider.create_file(
        root_id, 'file name', 'text/plain', False)
    print(upload_id, fd)

    os.write(fd, b'Hello world\n' * 1000)
    os.close(fd)

    provider.finish_upload(upload_id)
    print("Completed upload")
    print()

    print("Preparing to download file...", end='', flush=True)
    download_id, fd = provider.download("some-id")
    print(download_id, fd)
    with os.fdopen(fd) as fp:
        contents = fp.read()
    provider.finish_download(download_id)
    print("Contents: %r" % contents)

    print("Moving file...", end='', flush=True)
    item = provider.move(child_id, root_id, "New Name")
    print(item)
    print()

    print("Copying file...", end='', flush=True)
    item = provider.copy(child_id, root_id, "Copy name")
    print(item)
    print()

if __name__ == '__main__':
    sys.exit(main(sys.argv))
