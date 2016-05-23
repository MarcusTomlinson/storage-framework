#include <unity/storage/qt/client/client-api.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFutureWatcher>

using namespace unity::storage::qt::client;
using namespace std;

class Receiver : public QObject
{
    Q_OBJECT

public:
    Receiver(QObject* = nullptr);
    ~Receiver() = default;

public Q_SLOTS:
    void show_list_result(QFutureWatcher<QVector<Item::SPtr>>& w);

Q_SIGNALS:
    void list_finished();
};

Receiver::Receiver(QObject* parent)
    : QObject(parent)
{
}

void Receiver::show_list_result(QFutureWatcher<QVector<Item::SPtr>>& w)
{
    auto fut = w.future();
    try
    {
        auto results = fut.result();
        qDebug() << results.size() << "results";
    }
    catch (std::exception const& e)
    {
        qDebug() << e.what();
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Receiver rcv;

        auto runtime = Runtime::create();
        auto accounts = runtime->accounts().result();
        Root::SPtr root = accounts[0]->roots().result()[0];
        qDebug() << "id:" << root->native_identity();
        qDebug() << "time:" << root->last_modified_time();

        auto fut = root->list();
        QFutureWatcher<QVector<Item::SPtr>> watcher;
        watcher.setFuture(fut);

        auto list_ready = [&watcher, &rcv]{ rcv.show_list_result(watcher); };
        QObject::connect(&watcher, &QFutureWatcher<QVector<Item::SPtr>>::finished, &rcv, list_ready);

        QCoreApplication app(argc, argv);
        return app.exec();
    }
    catch (std::exception const& e)
    {
        qDebug() << e.what();
        return 1;
    }
}

#include "main.moc"
