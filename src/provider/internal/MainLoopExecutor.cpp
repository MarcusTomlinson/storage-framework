#include <unity/storage/provider/internal/MainLoopExecutor.h>

#include <QCoreApplication>
#include <QEvent>

#include <stdexcept>

namespace {

class WorkEvent : public QEvent {
public:
    WorkEvent(unity::storage::provider::internal::MainLoopExecutor::work const& closure)
        : QEvent(WorkEvent::eventType()), closure_(closure)
    {
    }

    static QEvent::Type eventType()
    {
        static auto type = static_cast<QEvent::Type>(QEvent::registerEventType());
        return type;
    }

    unity::storage::provider::internal::MainLoopExecutor::work const closure_;
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

void MainLoopExecutor::submit(work const& closure)
{
    QCoreApplication::instance()->postEvent(this, new WorkEvent(closure));
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

void MainLoopExecutor::execute(work const& closure) noexcept
{
    closure();
}

}
}
}
}
