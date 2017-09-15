#include "zbarQRScanner.h"

ZbarQRScanner::ZbarQRScanner()
{
    // only look for QR codes: disable all + enable QR
    image_scanner_.set_config((zbar::zbar_symbol_type_t) 0, zbar::ZBAR_CFG_ENABLE, 0);
    image_scanner_.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);
}

std::string ZbarQRScanner::last_QR_code_content_found() const
{
    return QR_code_content_;
}

bool ZbarQRScanner::locked_on_QR_code_area() const
{
    return locked_;
}

void ZbarQRScanner::clear_results()
{
    locked_ = false;
    QR_code_content_ = "";
    QR_code_area_ = {};
}

void ZbarQRScanner::update_results(const QRImage& image)
{
    QR_code_content_ = image.symbol.get_data();
    if (!image.partial_image)
        QR_code_area_ = get_area_around_symbol(image);
    locked_ = true;
}

bool ZbarQRScanner::should_skip_image()
{
    if (!locked_ && (image_counter_ % SCAN_INTERVAL != 0)) {
        image_counter_++;
        return true;
    }
    image_counter_++;
    return false;
}

const QRImage ZbarQRScanner::crop_and_convert_image(const QImage& image) const
{
    QRImage qr_image;
    if(locked_) {
        qr_image.image = image.copy(QR_code_area_).convertToFormat(QImage::Format_Grayscale8);
        qr_image.partial_image = true;
        qr_image.scale_factor = 1.0;
    }
    else {
        if (image.width() > 960) {
            qr_image.image = image.scaledToWidth(960, Qt::FastTransformation).convertToFormat(QImage::Format_Grayscale8);
            qr_image.scale_factor = static_cast<float>(image.width()) / static_cast<float>(qr_image.image.width());
        }
        else {
            qr_image.image = image.convertToFormat(QImage::Format_Grayscale8);
            qr_image.scale_factor = 1.0;
        }
        qr_image.partial_image = false;
    }
    return qr_image;
}

bool ZbarQRScanner::scan_image(const QImage &image)
{
    if (should_skip_image())
        return false;

    QRImage im = crop_and_convert_image(image);

    zbar::Image zbar_image(im.image.width(), im.image.height(), "Y800", im.image.bits(), im.image.byteCount());
    int scan_result  = image_scanner_.scan(zbar_image);
    if (scan_result < 0) {
        //scanning failed
        clear_results();
        return false;
    }
    else if (scan_result == 0) {
        //no results
        clear_results();
        return true;
    }
    else {
        im.symbol = *(zbar_image.get_symbols().symbol_begin());
        update_results(im);
        return true;
    }
}

QRect ZbarQRScanner::get_area_around_symbol(const QRImage& image)
{
    zbar::Symbol symbol = image.symbol;
    QPoint start((*symbol.point_begin()).x, (*symbol.point_begin()).y);
    QRect rect(start.x(), start.y(), 0, 0);
    for (zbar::Symbol::PointIterator it = ++symbol.point_begin(); it != symbol.point_end(); ++it) {
        QPoint point((*it).x, (*it).y);
        if (point.x() < rect.left()) rect.setLeft(point.x());
        if (point.x() > rect.right()) rect.setRight(point.x());
        if (point.y() < rect.top()) rect.setTop(point.y());
        if (point.y() > rect.bottom()) rect.setBottom(point.y());
    }

    QRect rect_stretched = rect;
    rect_stretched.translate(rect.x() * (image.scale_factor - 1), rect.y() * (image.scale_factor - 1));
    rect_stretched.setWidth(rect.width() * image.scale_factor);
    rect_stretched.setHeight(rect.height() * image.scale_factor);

    QRect rect_with_margins = rect_stretched.marginsAdded({5, 5, 5, 5});
    rect_with_margins.setWidth((rect_with_margins.width() / 4 + 1) * 4);
    rect_with_margins.setHeight((rect_with_margins.height() / 4 + 1) * 4);
    return rect_with_margins;
}
