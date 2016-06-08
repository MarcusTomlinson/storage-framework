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

class MainLoopExecutor : public QObject {
    Q_OBJECT
public:
    typedef std::function<void()> work;

    static MainLoopExecutor& instance();
    void submit(work const& closure);

    bool event(QEvent *event) override;

private:
    MainLoopExecutor();
    void execute(work const& closure) noexcept;

    Q_DISABLE_COPY(MainLoopExecutor)
};

}
}
}
}
