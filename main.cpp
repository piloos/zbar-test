#include <QCoreApplication>
#include <QTimer>
#include <unified-logger/logger.h>
#include <unified-logger/consolehandler.h>
#include <unified-logger/threadsafehandler.h>
#include <unified-logger/predefinedfilters.h>

static void init_loggers()
{
    std::unique_ptr<lol::LogHandler> consolehandler(new lol::Handlers::ConsoleHandler);
    // This is a multithreaded application, we don't want the output from multiple threads mixed together !
    consolehandler.reset(new lol::Handlers::ThreadSafeHandler(std::move(consolehandler)));
    lol::logger().addHandler(std::move(consolehandler));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    init_loggers();

    lol::logger().notice("Starting ZBAR test application");

    QTimer::singleShot(0, &app, [](){ lol::logger().notice("Main thread started"); });
    QTimer::singleShot(1000, &app, &QCoreApplication::quit);

    app.exec();

    lol::logger().notice("ZBAR test application finished");
}
