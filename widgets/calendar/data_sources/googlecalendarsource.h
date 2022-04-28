#pragma once

#include "cal_isource.h"

#include <QColor>
#include <QList>
#include <QMap>
#include <QNetworkReply>
#include <QStringList>
#include <QTimer>

class O2GoogleDevice;
class O2Requestor;
class QJsonDocument;
class QJsonObject;
class QSettings;

namespace calendar {

struct Event;

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
    void onRefreshFinished(QNetworkReply::NetworkError error);

protected:
    Q_DISABLE_COPY(GoogleCalendarSource)
    GoogleCalendarSource(O2GoogleDevice *o2, const QStringList& calendars, QSharedPointer<QNetworkAccessManager> net, QObject *parent);

    void getColors();
    void getCalendarInfo();
    void getEvents(const QString& calendar);
    bool isAccessTokenValid();

    void addEvents(const QJsonDocument &jdoc, const QString &calendarId);
    QColor getEventColor(const QJsonObject &event, const QString &calendarId);

    QSharedPointer<QNetworkAccessManager> m_net;
    O2GoogleDevice *m_o2;
    O2Requestor *m_requestor;
    bool m_linkingInProgress;
    const QStringList m_ids;
    int m_colorRequest;
    int m_calendarInfoRequest;
    QMap<int, QColor> m_calendarColorsByColorId;
    QMap<QString, QColor> m_calendarColorsByCalendarId;
    QMap<int, QColor> m_eventColors;
    QMap<int, QString> m_currentRequests;
    QList<calendar::Event> m_events;
    QTimer m_retryTimer;
    QTimer m_refreshTimer;
};

} // namespace calendar
