#include <unity/storage/provider/internal/MainLoopExecutor.h>

#include <QCoreApplication>
#include <QEvent>

#include <stdexcept>

namespace {

class WorkEvent : public QEvent {
public:
    typedef unity::storage::provider::internal::MainLoopExecutor::work work;

    WorkEvent(work&& closure)
        : QEvent(WorkEvent::eventType()), closure_(std::move(closure))
    {
    }

    static QEvent::Type eventType()
    {
        static auto type = static_cast<QEvent::Type>(QEvent::registerEventType());
        return type;
    }

    work closure_;
};

}

namespace unity
{
namespace storage
{
namespace provider
{
namespace internal
{

MainLoopExecutor::MainLoopExecutor()
{
}

MainLoopExecutor& MainLoopExecutor::instance()
{
    static MainLoopExecutor instance;
    return instance;
}

void MainLoopExecutor::submit(work&& closure)
{
    QCoreApplication::instance()->postEvent(
        this, new WorkEvent(std::move(closure)));
}

void MainLoopExecutor::close()
{
}

bool MainLoopExecutor::closed()
{
    return false;
}

bool MainLoopExecutor::try_executing_one()
{
    return false;
}


bool MainLoopExecutor::event(QEvent *e)
{
    if (e->type() != WorkEvent::eventType())
    {
        return QObject::event(e);
    }
    auto *we = static_cast<WorkEvent*>(e);
    execute(we->closure_);
    return true;
}

void MainLoopExecutor::execute(work& closure) noexcept
{
    closure();
}

}
}
}
}
