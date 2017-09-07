#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QSize>

#include <iostream>
#include <fstream>

#include <zbar.h>

using namespace std;

zbar::Image read_file(string filename, QSize framesize)
{
    ifstream file(filename, ios::binary | ios::ate);
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    char* data = new char[size];
    file.read(data, size);
    file.close();

    int expected_size = framesize.width() * framesize.height() * 3;
    if (expected_size != size) {
        cout << endl << "WARNING: the detected file size is " << size
             << " bytes, while we were expecting " << expected_size << " bytes according to the specified framesize." << endl << endl;
    }

    zbar::Image image(framesize.width(), framesize.height(), "RGB3", data, size);

    cout << "Image size is " << image.get_data_length() << " bytes" << endl;

    return image;
}

static void analyse_file(string filename, QSize framesize)
{
    zbar::Image image = read_file(filename, framesize);

    // Scanner can only convert Y800 or GRAY image formats
    zbar::Image image_converted = image.convert("Y800");
    cout << "Converted to Y800, the image size is " << image_converted.get_data_length() << " bytes" << endl;

    zbar::ImageScanner image_scanner;

    image_scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);

    int result = image_scanner.scan(image_converted);

    cout << "# Scanning results: " << result << endl;

    zbar::SymbolSet results = image_converted.get_symbols();

    cout << "Image contains " << results.get_size() << " symbols after scanning" << endl;

    for (zbar::SymbolIterator it = results.symbol_begin(); it != results.symbol_end(); ++it) {
        cout << "  Symbol found: " << it->get_data() << endl;
    }

}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    cout << "Starting ZBAR test application" << endl;

    QCommandLineParser parser;

    QCommandLineOption file_option(QString("file"), QString("Raw image file (packed 3-byte RGB data) to analyse."), QString("filename"));
    parser.addOption(file_option);

    QCommandLineOption size_option(QString("framesize"), QString("Size of the image"), QString("<width>x<height>"), QString("800x600"));
    parser.addOption(size_option);

    parser.addHelpOption();

    parser.process(app);

    string filename = parser.value(file_option).toStdString();

    QStringList l = parser.value(size_option).split("x");
    QSize framesize(l.at(0).toInt(), l.at(1).toInt());

    cout << "Going to analyse file " << filename << endl;

    analyse_file(filename, framesize);

    cout << "ZBAR test application finished" << endl;
}
