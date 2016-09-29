import QtQuick 2.0
import Ubuntu.StorageFramework 0.1

Item {
  id: root
  width: 100
  height: 100

  property var accountsjob: null;

  Runtime {
    id: runtime

    Component.onCompleted: root.accountsjob = runtime.accounts()
  }

  Connections {
    target: root.accountsjob

    onStatusChanged: {
      console.log("AccountsJob status changed to " + status);

      if (status == AccountsJob.Finished) {
        var accounts = root.accountsjob.accounts;
        for (var i = 0; i < accounts.length; i++) {
          console.log("Account " + i + " owner = " + accounts[i].owner);
        }
      }
    }
  }
}
