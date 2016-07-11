#pragma once

#include <QFuture>
#include <QFutureInterface>

namespace unity
{
namespace storage
{
namespace qt
{
namespace client
{
namespace internal
{

template<typename T = void>
QFuture<T> make_ready_future()
{
    QFutureInterface<void> qf;
    qf.reportFinished();
    return qf.future();
}

template<typename T = void>
QFuture<T> make_ready_future(QFutureInterface<T> qf)
{
    qf.reportFinished();
    return qf.future();
}

template<typename T>
QFuture<T> make_ready_future(T const& val)
{
    QFutureInterface<T> qf;
    qf.reportResult(val);
    qf.reportFinished();
    return qf.future();
}

template<typename T>
QFuture<T> make_ready_future(QFutureInterface<T> qf, T const& val)
{
    qf.reportResult(val);
    qf.reportFinished();
    return qf.future();
}

template<typename E>
QFuture<void> make_exceptional_future(E const& ex)
{
    QFutureInterface<void> qf;
    qf.reportException(ex);
    qf.reportFinished();
    return qf.future();
}

template<typename T, typename E>
QFuture<T> make_exceptional_future(E const& ex)
{
    QFutureInterface<T> qf;
    qf.reportException(ex);
    qf.reportFinished();
    return qf.future();
}

template<typename T, typename E>
QFuture<T> make_exceptional_future(QFutureInterface<T> qf, E const& ex)
{
    qf.reportException(ex);
    qf.reportFinished();
    return qf.future();
}

}  // namespace internal
}  // namespace client
}  // namespace qt
}  // namespace storage
}  // namespace unity
