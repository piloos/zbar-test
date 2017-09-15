#ifndef ZBARQRSCANNER_H
#define ZBARQRSCANNER_H

#include "QRScanner.h"

#include <zbar.h>

struct QRImage {
    QImage image {};
    float scale_factor {1.0};
    bool partial_image {false};
    zbar::Symbol symbol {};
};

class ZbarQRScanner : virtual public QRScanner
{
public:
    explicit ZbarQRScanner();
    virtual std::string last_QR_code_content_found() const final;
    virtual bool locked_on_QR_code_area() const final;
    virtual bool scan_image(const QImage& image) final;

private:
    static const int SCAN_INTERVAL = 10; ///< if not locked, only scan 10% of images

    zbar::ImageScanner image_scanner_ {};

    bool locked_ {false};
    unsigned image_counter_ {0};
    QRect QR_code_area_ {};

    std::string QR_code_content_ {};

    void clear_results();
    void update_results(const QRImage&);
    bool should_skip_image();
    const QRImage crop_and_convert_image(const QImage&) const;

    static QRect get_area_around_symbol(const QRImage&);
};

#endif // ZBARQRSCANNER_H
