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
          console.log("Account " + i + ": id =          " + accounts[i].id);
          console.log("Account " + i + ": serviceId =   " + accounts[i].serviceId);
          console.log("Account " + i + ": displayName = " + accounts[i].displayName);
        }
      }
    }
  }
}
