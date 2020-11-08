#pragma once

#include <QSplashScreen>

class QMouseEvent;
class QPainter;
class QSize;
class QString;

namespace utils {

class QrCodePopup : public QSplashScreen
{
    Q_OBJECT

public:
    explicit QrCodePopup(const QSize& size, const QString& qr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
    void mousePressEvent(QMouseEvent *event);
    void drawContents(QPainter *painter);

protected slots:
    void onMessageChanged(const QString &message);
};

} // namespace utils
