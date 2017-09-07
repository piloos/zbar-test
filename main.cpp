#include <QCoreApplication>
#include <QTimer>

#include <QCommandLineParser>

#include <iostream>
#include <fstream>

#include <zbar.h>

using namespace std;

zbar::Image read_file(string filename)
{
    ifstream file(filename, ios::binary | ios::ate);
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    char* data = new char[size];
    file.read(data, size);
    file.close();

    zbar::Image image(800, 600, "RGB3", data, size);

    cout << "Image size is " << image.get_data_length() << " bytes" << endl;

    return image;
}

static void analyse_file(string filename)
{
    zbar::Image image = read_file(filename);

    zbar::Image image_converted = image.convert("YU12");

    cout << "Converted to YU12, the image is only " << image_converted.get_data_length() << " bytes" << endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    cout << "Starting ZBAR test application" << endl;

    QCommandLineParser parser;

    QCommandLineOption file_option(QString("file"), QString("Raw image file to analyse."), QString("filename"));
    parser.addOption(file_option);

    parser.addHelpOption();

    parser.process(app);

    string filename = parser.value(file_option).toStdString();

    cout << "Going to analyse file " << filename << endl;

    analyse_file(filename);

    QTimer::singleShot(0, &app, [](){ cout << "Main thread started" << endl; });
    QTimer::singleShot(1000, &app, &QCoreApplication::quit);

    app.exec();

    cout << "ZBAR test application finished" << endl;
}
