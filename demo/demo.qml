/*
 * Copyright (C) 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 *              Michi Henning <michi.henning@canonical.com>
 */

import QtQuick 2.0
import Ubuntu.StorageFramework 0.1 as SF

// From Build dir, run with:
//    qmlscene -I plugins $srcdir/demo/demo.qml

Item {
  id: root
  width: 100
  height: 100

  property var accountsjob: null;

  SF.Runtime {
    id: runtime

    Component.onCompleted: root.accountsjob = runtime.accounts()
  }

  Connections {
    target: root.accountsjob

    onStatusChanged: {
      console.log("AccountsJob status changed to " + status);

      if (status == SF.AccountsJob.Finished) {
        var accounts = root.accountsjob.accounts;
        console.log("Got accounts " + accounts);
        for (var i = 0; i < accounts.length; i++) {
          console.log("Account " + i + ": busName =     " + accounts[i].busName());
          console.log("Account " + i + ": objectPath =  " + accounts[i].objectPath());
          console.log("Account " + i + ": displayName = " + accounts[i].displayName);
        }
      }
    }
  }
}
