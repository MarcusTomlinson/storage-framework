#pragma once

#include <boost/thread/executor.hpp>
#include <QObject>

#include <functional>

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

class MainLoopExecutor : public QObject, public boost::executors::executor {
    Q_OBJECT
public:
    static MainLoopExecutor& instance();

    void submit(work&& closure) override;
    void close() override;
    bool closed() override;
    bool try_executing_one() override;

    bool event(QEvent *event) override;

private:
    MainLoopExecutor();
    void execute(work& closure) noexcept;

    Q_DISABLE_COPY(MainLoopExecutor)
};

}
}
}
}
