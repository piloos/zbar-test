#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QSize>
#include <QElapsedTimer>
#include <QImage>

#include <iostream>
#include <fstream>

#include <zbar.h>

using namespace std;

pair<char*, int> read_file(string filename)
{
    ifstream file(filename, ios::binary | ios::ate);
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    char* data = new char[size];
    file.read(data, size);
    file.close();

    return pair<char*, int>(data, (int) size);
}

static void analyse_file(string filename, QSize framesize, int nr_cycles)
{
    pair<char*, int> file_info = read_file(filename);
    char* data = file_info.first;
    int size = file_info.second;

    int expected_size = framesize.width() * framesize.height() * 3;
    if (expected_size != size) {
        cout << endl << "WARNING: the detected file size is " << size
             << " bytes, while we were expecting " << expected_size << " bytes according to the specified framesize." << endl << endl;
    }

    QElapsedTimer timer;

    timer.start();

    qint64 t1_min = 9223372036854775807;
    qint64 t1_avg = 0;
    qint64 t2_min = 9223372036854775807;
    qint64 t2_avg = 0;
    qint64 t3_min = 9223372036854775807;
    qint64 t3_avg = 0;
    qint64 t4_min = 9223372036854775807;
    qint64 t4_avg = 0;
    qint64 t5_min = 9223372036854775807;
    qint64 t5_avg = 0;
    qint64 t_total_min = 9223372036854775807;
    qint64 t_total_avg = 0;


    zbar::ImageScanner image_scanner;

    // only look for QR codes: disable all + enable QR
    image_scanner.set_config((zbar::zbar_symbol_type_e) 0, zbar::ZBAR_CFG_ENABLE, 0);
    image_scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);

    for (int i = 0; i < nr_cycles; ++i) {

        qint64 t_start = timer.nsecsElapsed();

        qint64 t0,t1,t2,t3,t4,t5;

        {
            t0 = timer.nsecsElapsed();

            QImage qimage((unsigned char*) data, framesize.width(), framesize.height(), QImage::Format_RGB888);

            quint64 t_now = timer.nsecsElapsed();
            t1 = t_now - t0;
            t0 = t_now;

            // Scanner can only convert Y800 or GRAY image formats
            QImage qimage_converted = qimage.convertToFormat(QImage::Format_Grayscale8);
            zbar::Image image_converted(qimage_converted.width(), qimage_converted.height(), "Y800", qimage_converted.bits(), qimage_converted.byteCount());

            t_now = timer.nsecsElapsed();
            t2 = t_now - t0;
            t0 = t_now;

            int result  = image_scanner.scan(image_converted);
            if (result < 0) cout << "Image scanning failed!!" << endl;

            t_now = timer.nsecsElapsed();
            t3 = t_now - t0;
            t0 = t_now;

            zbar::SymbolSet results = image_converted.get_symbols();

            t_now = timer.nsecsElapsed();
            t4 = t_now - t0;
            t0 = t_now;

            if (results.get_size() > 0) {
                std::string symbol_data = results.symbol_begin()->get_data();
                cout << "QR symbol found: " << symbol_data << endl;
            }
            else {
                cout << "No QR symbol found" << endl;
            }

            t_now = timer.nsecsElapsed();
            t5 = t_now - t0;
        }

        qint64 t_total = timer.nsecsElapsed() - t_start;

        if (t_total < t_total_min) t_total_min = t_total;
        t_total_avg += t_total;

        if (t1 < t1_min) t1_min = t1;
        t1_avg += t1;
        if (t2 < t2_min) t2_min = t2;
        t2_avg += t2;
        if (t3 < t3_min) t3_min = t3;
        t3_avg += t3;
        if (t4 < t4_min) t4_min = t4;
        t4_avg += t4;
        if (t5 < t5_min) t5_min = t5;
        t5_avg += t5;
    }

    t1_avg = t1_avg / nr_cycles;
    t2_avg = t2_avg / nr_cycles;
    t3_avg = t3_avg / nr_cycles;
    t4_avg = t4_avg / nr_cycles;
    t5_avg = t5_avg / nr_cycles;
    t_total_avg = t_total_avg / nr_cycles;

    cout << "t1 min " << t1_min / 1000 << " us, avg " << t1_avg / 1000 << " us" << endl;
    cout << "t2 min " << t2_min / 1000 << " us, avg " << t2_avg / 1000 << " us" << endl;
    cout << "t3 min " << t3_min / 1000 << " us, avg " << t3_avg / 1000 << " us" << endl;
    cout << "t4 min " << t4_min / 1000 << " us, avg " << t4_avg / 1000 << " us" << endl;
    cout << "t5 min " << t5_min / 1000 << " us, avg " << t5_avg / 1000 << " us" << endl;
    cout << "total min " << t_total_min / 1000 << " us, avg " << t_total_avg / 1000 << " us" << endl;

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

    QCommandLineOption repeat_option(QString("repeat"), QString("Repeat procedure x times"), QString("number"), QString("1"));
    parser.addOption(repeat_option);

    parser.addHelpOption();

    parser.process(app);

    string filename = parser.value(file_option).toStdString();

    QStringList l = parser.value(size_option).split("x");
    QSize framesize(l.at(0).toInt(), l.at(1).toInt());

    int nr_cycles = parser.value(repeat_option).toInt();

    cout << "Going to analyse file '" << filename << "' "<< nr_cycles << " time(s)" << endl;

    analyse_file(filename, framesize, nr_cycles);

    cout << "ZBAR test application finished" << endl;
}
