#ifndef ICURRENTCONDITIONSDATASOURCE_H
#define ICURRENTCONDITIONSDATASOURCE_H

#include <QObject>

#include <QTime>
#include <QList>
#include <QObject>
#include <QPixmap>
#include <QSharedPointer>
#include <QTime>

class QNetworkAccessManager;
class QSettings;

namespace weather {

class ICurrentConditionsDataSource : public QObject
{
    Q_OBJECT
public:
    static bool Create(ICurrentConditionsDataSource*& obj, const QSharedPointer<QSettings> settings, QSharedPointer<QNetworkAccessManager> net, QObject* parent = 0);
    virtual ~ICurrentConditionsDataSource() = 0;

signals:
    void temperature(double);
    void humidity(double);
    void windSpeed(double);
    void skyConditions(QString);
    void sunrise(QTime);
    void sunset(QTime);
    void image(QPixmap);

protected:
    ICurrentConditionsDataSource(QObject* parent = 0) : QObject(parent) {}
    Q_DISABLE_COPY(ICurrentConditionsDataSource)
};
inline ICurrentConditionsDataSource::~ICurrentConditionsDataSource() {}

} // namespace weather

#endif // ICURRENTCONDITIONSDATASOURCE_H
