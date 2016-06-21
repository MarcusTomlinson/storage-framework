#!/usr/bin/python3

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


def main(argv):
    bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
    provider = Provider(bus, PROVIDER_BUS_NAME, argv[1])

    roots = provider.roots()
    print("Roots:", roots)

    root_id = roots[0][0]

    upload_id, fd = provider.create_file(
        root_id, 'file name', 'text/plain', False)
    print(upload_id, fd)

    os.write(fd, b'Hello world\n' * 1000)
    os.close(fd)

    provider.finish_upload(upload_id)

if __name__ == '__main__':
    sys.exit(main(sys.argv))
