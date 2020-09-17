#ifndef CALENDAR_ICSSOURCE_H
#define CALENDAR_ICSSOURCE_H

#include "isource.hpp"

#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <QUrl>

class QNetworkReply;
class QNetworkAccessManager;
class QSettings;

namespace calendar {

class IcsSource : public ISource
{
    Q_OBJECT

public:
    static bool Create(ISource*& obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);
    virtual ~IcsSource() {}

public slots:
    virtual void sync();

protected:
    Q_DISABLE_COPY(IcsSource)

    IcsSource(const QUrl& url, QSharedPointer<QNetworkAccessManager> net, QObject* parent);
    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply* m_reply;
    const QUrl m_url;
    QTimer m_retryTimer;

protected slots:
    void downloadFinished();
};

} // namespace calendar

#endif // CALENDAR_ICSSOURCE_H
