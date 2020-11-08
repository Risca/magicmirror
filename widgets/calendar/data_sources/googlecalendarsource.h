#pragma once

#include "cal_isource.h"

#include <QNetworkReply>
#include <QStringList>

class O2GoogleDevice;
class O2Requestor;
class QSettings;

namespace calendar {

class GoogleCalendarSource : public ISource
{
    Q_OBJECT

public:
    static bool Create(ISource*& obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);
    virtual ~GoogleCalendarSource();

public slots:
    virtual void sync();

protected slots:
    void onLinkedChanged();
    void onLinkingFailed();
    void onLinkingSucceeded();
    void onVerificationCodeAndUrl(const QUrl &url, const QString &code);
    void onFinished(int id, QNetworkReply::NetworkError error, const QByteArray &data);

protected:
    Q_DISABLE_COPY(GoogleCalendarSource)
    GoogleCalendarSource(O2GoogleDevice *o2, const QStringList& calendars, QSharedPointer<QNetworkAccessManager> net, QObject *parent);

    void getEvents();
    void deleteRequestor();

    QSharedPointer<QNetworkAccessManager> m_net;
    O2GoogleDevice *m_o2;
    O2Requestor *m_requestor;
    QStringList m_ids;
    int m_requestId;
};

} // namespace calendar
