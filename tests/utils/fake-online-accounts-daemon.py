#!/usr/bin/python3
# A fake version of the OnlineAccounts D-Bus service.

import os
from gi.repository import Gio, GLib

BUS_NAME = "com.ubuntu.OnlineAccounts.Manager"
OBJECT_PATH = "/com/ubuntu/OnlineAccounts/Manager"
OA_IFACE = "com.ubuntu.OnlineAccounts.Manager"

INTERFACE_XML = os.path.join(os.path.dirname(__file__),
                             "com.ubuntu.OnlineAccounts.Manager.xml")

AUTH_OAUTH1 = 1
AUTH_OAUTH2 = 2
AUTH_PASSWORD = 3
AUTH_SASL = 4

class OAuth1:
    method = AUTH_OAUTH1
    def __init__(self, consumer_key, consumer_secret, token, token_secret, signature_method='HMAC-SHA1'):
        self.consumer_key = consumer_key
        self.consumer_secret = consumer_secret
        self.token = token
        self.token_secret = token_secret
        self.signature_method = signature_method

    def serialise(self):
        return {
            'ConsumerKey': GLib.Variant("s", self.consumer_key),
            'ConsumerSecret': GLib.Variant("s", self.consumer_secret),
            'Token': GLib.Variant("s", self.token),
            'TokenSecret': GLib.Variant("s", self.token_secret),
            'SignatureMethod': GLib.Variant("s", self.signature_method),
        }

class OAuth2:
    method = AUTH_OAUTH2
    def __init__(self, access_token, expires_in=0, granted_scopes=[]):
        self.access_token = access_token
        self.expires_in = expires_in
        self.granted_scopes = granted_scopes

    def serialise(self):
        return {
            'AccessToken': GLib.Variant("s", self.access_token),
            'ExpiresIn': GLib.Variant("i", self.expires_in),
            'GrantedScopes': GLib.Variant("as", self.granted_scopes),
        }

class Password:
    method = AUTH_PASSWORD
    def __init__(self, username, password):
        self.username = username
        self.password = password

    def serialise(self):
        return {
            'Username': GLib.Variant("s", self.username),
            'Password': GLib.Variant("s", self.password),
        }

class Account:
    def __init__(self, account_id, display_name, service_id, credentials):
        self.account_id = account_id
        self.display_name = display_name
        self.service_id = service_id
        self.credentials = credentials

    def serialise(self):
        return (self.account_id, {
            'displayName': GLib.Variant("s", self.display_name),
            'serviceId': GLib.Variant("s", self.service_id),
            'authMethod': GLib.Variant("i", self.credentials.method),
            })

class Manager:
    def __init__(self, connection, object_path, accounts):
        self.connection = connection

        with open(INTERFACE_XML, 'r') as fp:
            introspection_data = Gio.DBusNodeInfo.new_for_xml(fp.read())
        connection.register_object(
            OBJECT_PATH, introspection_data.interfaces[0],
            self._on_method_call,
            self._on_get_property,
            self._on_set_property)
        self.accounts = accounts

    def _on_method_call(self, connection, sender, object_path, interface_name,
                       method_name, parameters, invocation):
        if method_name.startswith('_'):
            invocation.return_error_literal(
                Gio.DBusError.quark(),
                Gio.DBusError.UNKNOWN_METHOD,
                "Method not found")
            return
        try:
            method = getattr(self, method_name)
        except KeyError:
            invocation.return_error_literal(
                Gio.DBusError.quark(),
                Gio.DBusError.UNKNOWN_METHOD,
                "Method not found")
            return

        try:
            result = method(*parameters)
        except Exception as e:
            invocation.return_error_literal(
                Gio.DBusError.quark(),
                Gio.DBusError.FAILED,
                str(e))
            return
        if isinstance(result, tuple):
            result = GLib.Variant.new_tuple(*result)
        else:
            result = GLib.Variant.new_tuple(result)
        invocation.return_value(result)

    def _on_get_property(self, connection, sender, object_path, interface, key):
        pass

    def _on_set_property(self, connection, sender, object_path, interface_name, key, value):
        pass

    def GetAccounts(self, filters):
        print("GetAccounts %r" % filters)
        return GLib.Variant("a(ua{sv})", [a.serialise() for a in self.accounts])

    def Authenticate(self, account_id, service_id, interactive, invalidate, parameters):
        for account in self.accounts:
            if account.account_id == account_id and account.service_id == service_id:
                return GLib.Variant("a{sv}", account.credentials.serialise())
        else:
            raise KeyError(repr((account_id, service_id)))

    def RequestAccess(self, service_id, parameters):
        for account in self.accounts:
            if account.service_id == service_id:
                return (GLib.Variant("(ua{sv})", account.serialise()),
                        GLib.Variant("a{sv}", account.credentials.serialise()))
        else:
            raise KeyError(service_id)

class Server:
    def __init__(self, accounts):
        self.accounts = accounts
        self.manager = None
        self.main_loop = GLib.MainLoop()
        self.owner_id = Gio.bus_own_name(Gio.BusType.SESSION, BUS_NAME,
                                         Gio.BusNameOwnerFlags.NONE,
                                         self.on_bus_acquired,
                                         self.on_name_acquired,
                                         self.on_name_lost)

    def run(self):
        try:
            self.main_loop.run()
        except KeyboardInterrupt:
            pass

    def on_bus_acquired(self, connection, name):
        self.manager = Manager(connection, OBJECT_PATH, accounts)

    def on_name_acquired(self, connection, name):
        pass

    def on_name_lost(self, connection, name):
        self.main_loop.quit()

if __name__ == '__main__':
    accounts = [
        Account(1, 'OAuth1 account', 'oauth1-service',
                OAuth1('consumer_key', 'consumer_secret', 'token', 'token_secret')),
        Account(2, 'OAuth2 account', 'oauth2-service',
                OAuth2('access_token', 0, ['scope1', 'scope2'])),
        Account(3, 'Password account', 'password-service',
                Password('user', 'pass')),
    ]
    server = Server(accounts)
    server.run()
