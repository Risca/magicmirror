#ifndef CALENDAR_ISOURCE_HPP
#define CALENDAR_ISOURCE_HPP

#include <QList>
#include <QObject>
#include <QSharedPointer>

class QNetworkAccessManager;
class QSettings;
class QString;

namespace calendar {

struct Event;

class ISource : public QObject {
    Q_OBJECT

public:
    static bool Create(ISource*& obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);
    virtual ~ISource() = 0;

public slots:
    virtual void sync() = 0;

signals:
    void finished(const QList<calendar::Event>&);
    void error(const QString&);

protected:
    ISource(QObject* parent) : QObject(parent) {}
    Q_DISABLE_COPY(ISource)

};
inline ISource::~ISource() {}

}
#endif // CALENDAR_ISOURCE_HPP
