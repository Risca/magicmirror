#pragma once

#include "cal_isource.h"

#include <QNetworkRequest>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>

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
    IcsSource(const QNetworkRequest &req, QSharedPointer<QNetworkAccessManager> net, QObject* parent);

    QSharedPointer<QNetworkAccessManager> m_net;
    QNetworkReply* m_reply;
    const QNetworkRequest m_request;
    QTimer m_retryTimer;

protected slots:
    void downloadFinished();
};

} // namespace calendar
