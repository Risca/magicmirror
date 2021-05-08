#include "qrcodepopup.h"

#include <QApplication>
#include <QFontMetrics>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

#include <qrencode.h>

namespace utils {

namespace {

QPixmap CreateQrCode(const QString& s, const QSize& size)
{
    QRcode *qr = QRcode_encodeString(s.toLocal8Bit().data(), 0, QR_ECLEVEL_Q, QR_MODE_8, 1);
    if (!qr) {
        return QPixmap();
    }

    // Allow room for 1 pixel white frame
    QImage im(qr->width + 2, qr->width + 2, QImage::Format_Mono);
    im.fill(1); // Set all pixels white
    for (int y = 0; y < qr->width; y++) {
        for (int x = 0; x < qr->width; x++) {
            if (qr->data[y * qr->width + x] & 0x01) {
                // Set pixel black
                im.setPixel(x + 1, y + 1, 0);
            }
        }
    }

    im = im.scaled(size, Qt::KeepAspectRatio);

    return QPixmap::fromImage(im, Qt::MonoOnly);
}

} // anonymous namespace

QrCodePopup::QrCodePopup(const QSize &size, const QString &qr, Qt::WindowFlags f) :
    QSplashScreen(CreateQrCode(qr, size), f | Qt::WindowStaysOnTopHint)
{
    // Need to set parent for the popup to show in fullscreen app
    this->setParent(QApplication::activeWindow());
    connect(this, &QrCodePopup::messageChanged, this, &QrCodePopup::onMessageChanged);
}

void QrCodePopup::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

// reimplemented to respect stylesheet colors
void QrCodePopup::drawContents(QPainter *painter)
{
    QRect r = rect().adjusted(5, 5, -5, -5);
    painter->drawText(r, Qt::AlignHCenter | Qt::AlignBottom, this->message());
}

// Resize popup to add space below the pixmap for writing the message
void QrCodePopup::onMessageChanged(const QString &message)
{
    QFontMetrics fm(this->fontMetrics());
    int height = fm.height() * (1 + message.count('\n')) + 5;
    this->resize(this->rect().size() + QSize(0, height));
}

} // namespace utils
