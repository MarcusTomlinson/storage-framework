#include <unity/storage/qt/client/client-api.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFutureWatcher>
#include <QSocketNotifier>

#include <cassert>

using namespace unity::storage;
using namespace unity::storage::qt::client;
using namespace std;

class Printer : public QObject
{
    Q_OBJECT

public:
    Printer(Item::SPtr const& item);
    ~Printer() = default;

public Q_SLOTS:
    void downloader_ready();
    void chunk_ready();
    void got_eof();
    void check_status();

Q_SIGNALS:
    void done();

private:
    QFutureWatcher<Downloader::SPtr> watcher_;
    Downloader::SPtr downloader_;
    QFutureWatcher<TransferState> status_watcher_;
    QByteArray data_;
};

Printer::Printer(Item::SPtr const& item)
{
    Folder::SPtr folder = dynamic_pointer_cast<Folder>(item);
    if (folder)
    {
        qDebug() << folder->name() << "(folder)";
        Q_EMIT done();
        return;
    }

    File::SPtr file = dynamic_pointer_cast<File>(item);
    assert(file);
    QObject::connect(&watcher_, &QFutureWatcher<Downloader::SPtr>::finished, this, &Printer::downloader_ready);
    watcher_.setFuture(file->create_downloader());
}

void Printer::downloader_ready()
{
    downloader_ = watcher_.future().result();
    connect(downloader_->socket().get(), &QLocalSocket::readyRead, this, &Printer::chunk_ready);
    connect(downloader_->socket().get(), &QLocalSocket::readChannelFinished, this, &Printer::got_eof);
}

void Printer::chunk_ready()
{
    char buf[64 * 1024];
    auto bytes_read = downloader_->socket()->read(buf, sizeof(buf));
    if (bytes_read == -1)
    {
        abort();
    }
    data_.append(buf, bytes_read);
}

void Printer::got_eof()
{
    connect(&status_watcher_, &QFutureWatcher<TransferState>::finished, this, &Printer::check_status);
    status_watcher_.setFuture(downloader_->finish_download());
}

void Printer::check_status()
{
    try
    {
        auto status = status_watcher_.future().result();
        if (status != TransferState::cancelled)
        {
            qDebug().nospace() << downloader_->file()->name() << " (" << data_.size() << ")";
            qDebug() << data_;
        }
    }
    catch (std::exception const& e)
    {
        qDebug() << e.what();
        abort();
    }
    Q_EMIT done();
}

class Lister : public QObject
{
    Q_OBJECT

public:
    Lister(QObject* = nullptr);
    ~Lister() = default;

public Q_SLOTS:
    void show_list_result(QFutureWatcher<QVector<Item::SPtr>>& w);

Q_SIGNALS:
    void list_finished();

public:
    QVector<shared_ptr<Printer>> printers_;
};

Lister::Lister(QObject* parent)
    : QObject(parent)
{
}

void Lister::show_list_result(QFutureWatcher<QVector<Item::SPtr>>& w)
{
    // Make a Printer for each result. The Printer shows the name of
    // each item and, if the item is a file, prints the file contents.
    try
    {
        for (auto i : w.future().result())
        {
            printers_.append(make_shared<Printer>(i));
        }
    }
    catch (std::exception const& e)
    {
        qDebug() << e.what();
        abort();
    }
}

int main(int argc, char* argv[])
{
    try
    {
        QCoreApplication app(argc, argv);

        // Initialize the runtime.
        auto runtime = Runtime::create();

        // Get the acccounts. (There is only one account for the local client implementation.)
        // We do this synchronously for simplicity.
        auto accounts = runtime->accounts().result();
        Root::SPtr root = accounts[0]->roots().result()[0];
        qDebug() << "id:" << root->native_identity();
        qDebug() << "time:" << root->last_modified_time();

        // Call list() and set up a watcher that triggers list_ready on the Lister once
        // list() has completed.
        QFutureWatcher<QVector<Item::SPtr>> watcher;
        watcher.setFuture(root->list());

        Lister lister;
        auto list_ready = [&watcher, &lister]{ lister.show_list_result(watcher); };
        QObject::connect(&watcher, &QFutureWatcher<QVector<Item::SPtr>>::finished, &lister, list_ready);

        auto item_fut = root->lookup("x");
        auto item = item_fut.result();
        auto file = dynamic_pointer_cast<File>(item);
        assert(file);

        return app.exec();
    }
    catch (std::exception const& e)
    {
        qDebug() << e.what();
        return 1;
    }
}

#include "main.moc"
