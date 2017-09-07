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


    // Scanner can only convert Y800 or GRAY image formats
    zbar::Image image_converted = image.convert("Y800");
    cout << "Converted to Y800, the image size is " << image_converted.get_data_length() << " bytes" << endl;

    zbar::ImageScanner image_scanner;

    image_scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);

    zbar::SymbolSet results_before = image_scanner.get_results();
    zbar::SymbolSet image_results_before = image_converted.get_symbols();

    cout << "Image scanner results before scanning contains " << results_before.get_size() << " elements" << endl;
    cout << "Image contains " << image_results_before.get_size() << " symbols before scanning" << endl;

    int result = image_scanner.scan(image_converted);

    cout << "Result of scanning: " << result << endl;

    zbar::SymbolSet results_after = image_scanner.get_results();
    zbar::SymbolSet image_results_after = image_converted.get_symbols();

    cout << "Image scanner results after scanning contains " << results_after.get_size() << " elements" << endl;
    cout << "Image contains " << image_results_after.get_size() << " symbols after scanning" << endl;


    for (zbar::SymbolIterator it = image_results_after.symbol_begin(); it != image_results_after.symbol_end(); ++it) {
        cout << "  Symbol found: " << it->get_data() << endl;
    }

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

    //QTimer::singleShot(0, &app, [](){ cout << "Main thread started" << endl; });
    //QTimer::singleShot(1000, &app, &QCoreApplication::quit);

    //app.exec();

    cout << "ZBAR test application finished" << endl;
}
