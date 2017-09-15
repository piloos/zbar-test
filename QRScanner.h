#ifndef QRSCANNER_H
#define QRSCANNER_H

#include <string>
#include <QImage>

class QRScanner
{
public:
    virtual ~QRScanner() {}
    virtual std::string last_QR_code_content_found() const = 0;
    virtual bool locked_on_QR_code_area() const = 0;
    virtual bool scan_image(const QImage& image) = 0;
};


#endif // QRSCANNER_H
