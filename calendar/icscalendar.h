#ifndef ICSCALENDAR_H
#define ICSCALENDAR_H

#include "calendarinterface.h"

#include <QObject>
#include <QTimer>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class IcsCalendar : public CalendarInterface
{
    Q_OBJECT

public:
    static bool Create(CalendarInterface*& cal, QSharedPointer<QNetworkAccessManager> net, QObject* parent);
    virtual ~IcsCalendar();

public slots:
    void sync();

protected:
    Q_DISABLE_COPY(IcsCalendar)

    IcsCalendar(const QUrl& url, QSharedPointer<QNetworkAccessManager> net, QObject* parent);
    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply* m_reply;
    const QUrl m_url;
    QTimer m_retryTimer;

protected slots:
    void downloadFinished();


};

#endif // ICSCALENDAR_H
