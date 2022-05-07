#pragma once

#include "departure_isource.h"

#include <QDateTime>
#include <QList>
#include <QNetworkRequest>
#include <QObject>
#include <QPair>
#include <QSharedPointer>
#include <QSignalMapper>
#include <QString>
#include <QTimer>

class QNetworkAccessManager;
class QNetworkReply;

namespace departure {

class ResrobotDepartures : public ISource
{
    Q_OBJECT

public:
    static bool Create(ISource* &obj, QSharedPointer<QNetworkAccessManager> &net, QObject *parent = 0);
    virtual ~ResrobotDepartures() {}

public slots:
    void requestNewDepartures();

protected:
    explicit ResrobotDepartures(const QList<QNetworkRequest>& stops, QSharedPointer<QNetworkAccessManager> &net, QObject *parent = 0);
    void requestDepartures(const QNetworkRequest& request);

    const QList<QNetworkRequest> m_stops;
    QSharedPointer<QNetworkAccessManager> m_net;
    QSignalMapper m_signalMapper;
    QTimer m_timer;

protected slots:
    void requestFinished(QObject *replyObject);
};

} // namespace departure
