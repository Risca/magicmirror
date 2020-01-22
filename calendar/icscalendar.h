#ifndef ICSCALENDAR_H
#define ICSCALENDAR_H

#include "calendarinterface.h"

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class IcsCalendar : public CalendarInterface
{
    Q_OBJECT

public:
    IcsCalendar(const QUrl& url, QSharedPointer<QNetworkAccessManager> net, QObject* parent);

public slots:
    void sync();

protected:
    Q_DISABLE_COPY(IcsCalendar)

    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply* m_reply;
    const QUrl m_url;

protected slots:
    void downloadFinished();


};

#endif // ICSCALENDAR_H
