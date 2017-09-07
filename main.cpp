#include <QCoreApplication>
#include <QTimer>

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    cout << "Starting ZBAR test application" << endl;

    QTimer::singleShot(0, &app, [](){ cout << "Main thread started" << endl; });
    QTimer::singleShot(1000, &app, &QCoreApplication::quit);

    app.exec();

    cout << "ZBAR test application finished" << endl;
}
