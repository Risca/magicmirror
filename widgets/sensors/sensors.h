#pragma once

#include <QList>
#include <QSharedPointer>
#include <QWidget>

class QNetworkAccessManager;

namespace Ui {
class Sensors;
}

namespace sensors {

class ISource;
class SensorData;

class Sensors : public QWidget
{
    Q_OBJECT

public:
    static bool Create(Sensors*& obj, QSharedPointer<QNetworkAccessManager>& net, QWidget* parent = 0);
    ~Sensors();

protected slots:
    void SetSensorData(const QList<SensorData>& list);

private:
    explicit Sensors(ISource* dataSource, QWidget *parent = 0);

    Ui::Sensors *ui;
    ISource* m_ds;
};

} // namespace sensors
