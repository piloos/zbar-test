#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QSize>
#include <QElapsedTimer>
#include <QImage>
#include <QRect>
#include <QMargins>

#include <iostream>
#include <fstream>

#include <zbar.h>

#include <zbarQRScanner.h>

using namespace std;

static QRect get_area(const zbar::Symbol& symbol)
{
    QPoint start((*symbol.point_begin()).x, (*symbol.point_begin()).y);
    QRect rect(start.x(), start.y(), 0, 0);
    for (zbar::Symbol::PointIterator it = ++symbol.point_begin(); it != symbol.point_end(); ++it) {
        QPoint point((*it).x, (*it).y);
        if (point.x() < rect.left()) rect.setLeft(point.x());
        if (point.x() > rect.right()) rect.setRight(point.x());
        if (point.y() < rect.top()) rect.setTop(point.y());
        if (point.y() > rect.bottom()) rect.setBottom(point.y());
    }
    return rect;
}

static QImage read_file(const string& filename)
{
    QImage image;
    image.load(QString::fromStdString(filename));
    QImage image_rgb888 = image.convertToFormat(QImage::Format_RGB888);
    cout << QString("Image loaded into memory: size %1x%2, byte count: %3")
            .arg(image_rgb888.width()).arg(image_rgb888.height()).arg(image_rgb888.byteCount()).toStdString() << endl;
    return image_rgb888;
}

static void analyse_image_old_school(const QImage& qimage, int nr_cycles)
{
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
    image_scanner.set_config((zbar::zbar_symbol_type_t) 0, zbar::ZBAR_CFG_ENABLE, 0);
    image_scanner.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);

    QRect search_area;
    bool search_area_found = false;

    for (int i = 0; i < nr_cycles; ++i) {

        qint64 t_start = timer.nsecsElapsed();

        qint64 t0,t1,t2,t3,t4,t5;

        {
            t0 = timer.nsecsElapsed();

            //qimage.save(QString("original_image_%1.png").arg(i));

            float sizing_factor = 1.0;

            QImage qimage_cropped;
            if (search_area_found) {
                //crop image to search area
                qimage_cropped = qimage.copy(search_area);
            }
            else {
                if (qimage.width() > 960) {
                    qimage_cropped = qimage.scaledToWidth(960, Qt::FastTransformation);
                    sizing_factor = static_cast<float>(qimage.width()) / static_cast<float>(qimage_cropped.width());
                    cout << "Downscaled image with factor " << sizing_factor << endl;
                }
                else {
                    qimage_cropped = qimage;
                }
            }

            quint64 t_now = timer.nsecsElapsed();
            t1 = t_now - t0;
            t0 = t_now;

            // Scanner can only scan Y800 (=grayscale, 1 byte/px) or GRAY image formats
            QImage qimage_converted_and_cropped = qimage_cropped.convertToFormat(QImage::Format_Grayscale8);
            zbar::Image image(qimage_converted_and_cropped.width(), qimage_converted_and_cropped.height(), "Y800",
                                        qimage_converted_and_cropped.bits(), qimage_converted_and_cropped.byteCount());


            //QImage qimage_from_zbar((unsigned char*) image.get_data(), image.get_width(), image.get_height(), QImage::Format_Grayscale8);
            //qimage_from_zbar.save(QString("cropped_image_%1_grayscale.png").arg(i));

            t_now = timer.nsecsElapsed();
            t2 = t_now - t0;
            t0 = t_now;

            int result  = image_scanner.scan(image);
            if (result < 0) cout << "Image scanning failed!!" << endl;

            t_now = timer.nsecsElapsed();
            t3 = t_now - t0;
            t0 = t_now;

            zbar::SymbolSet results = image.get_symbols();

            t_now = timer.nsecsElapsed();
            t4 = t_now - t0;
            t0 = t_now;

            if (results.get_size() > 0) {
                zbar::SymbolIterator symbol = results.symbol_begin();
                std::string symbol_data = symbol->get_data();
                cout << "QR symbol found: " << symbol_data << endl;
                QRect symbol_area = get_area(*symbol);
                cout << QString("Symbol_area: top left (%1, %2), size: %3x%4")
                        .arg(symbol_area.left()).arg(symbol_area.top()).arg(symbol_area.width()).arg(symbol_area.height()).toStdString() << endl;
                if (!search_area_found) {
                    cout << "Applying new search area" << endl;
                    QRect symbol_area_stretched = symbol_area;
                    symbol_area_stretched.translate(symbol_area.x() * (sizing_factor - 1), symbol_area.y() * (sizing_factor - 1));
                    symbol_area_stretched.setWidth(symbol_area.width() * sizing_factor);
                    symbol_area_stretched.setHeight(symbol_area.height() * sizing_factor);
                    cout << QString("Symbol_area_resized: top left (%1, %2), size: %3x%4")
                            .arg(symbol_area_stretched.left()).arg(symbol_area_stretched.top()).arg(symbol_area_stretched.width()).arg(symbol_area_stretched.height()).toStdString() << endl;
                    QRect symbol_area_with_margins = symbol_area_stretched.marginsAdded({5, 5, 5, 5});

                    //zbar fails on widths/heights which are not 4-byte aligned
                    symbol_area_with_margins.setWidth((symbol_area_with_margins.width() / 4 + 1) * 4);
                    symbol_area_with_margins.setHeight((symbol_area_with_margins.height() / 4 + 1) * 4);
                    search_area = symbol_area_with_margins;
                    search_area_found = true;
                }
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

static void analyse_image_new_school(const QImage& qimage, int nr_cycles)
{
    QElapsedTimer timer;

    timer.start();

    qint64 t_total_min = 9223372036854775807;
    qint64 t_total_avg = 0;

    ZbarQRScanner scanner;

    for (int i = 0; i < nr_cycles; ++i) {

        qint64 t_start = timer.nsecsElapsed();

        scanner.scan_image(qimage);

        if (scanner.locked_on_QR_code_area()) {
            cout << "QR symbol found: " << scanner.last_QR_code_content_found() << endl;
        }
        else {
            cout << "No QR symbol found" << endl;
        }

        qint64 t_total = timer.nsecsElapsed() - t_start;

        if (t_total < t_total_min) t_total_min = t_total;
        t_total_avg += t_total;
    }

    t_total_avg = t_total_avg / nr_cycles;

    cout << "total min " << t_total_min / 1000 << " us, avg " << t_total_avg / 1000 << " us" << endl;
}

static void analyse_file(const string& filename, int nr_cycles)
{
    QImage qimage = read_file(filename);

    cout << endl << "Old school way of analyzing:" << endl;

    analyse_image_old_school(qimage, nr_cycles);

    cout << endl << "New school way of analyzing:" << endl;

    analyse_image_new_school(qimage, nr_cycles);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    cout << "Starting ZBAR test application" << endl;

    QCommandLineParser parser;

    QCommandLineOption file_option(QString("file"), QString("Image file to analyse (png/jpeg/...)"), QString("filename"));
    parser.addOption(file_option);

    QCommandLineOption repeat_option(QString("repeat"), QString("Repeat procedure x times"), QString("number"), QString("1"));
    parser.addOption(repeat_option);

    parser.addHelpOption();

    parser.process(app);

    string filename = parser.value(file_option).toStdString();


    int nr_cycles = parser.value(repeat_option).toInt();

    cout << "Going to analyse file '" << filename << "' "<< nr_cycles << " time(s)" << endl;

    analyse_file(filename, nr_cycles);

    cout << "ZBAR test application finished" << endl;
}
