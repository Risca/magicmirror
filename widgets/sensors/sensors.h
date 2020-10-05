#pragma once

#include <QList>
#include <QSharedPointer>
#include <QWidget>

class QNetworkAccessManager;

namespace Ui {
class Sensors;
}

namespace sensors {

class SensorModel;

class Sensors : public QWidget
{
    Q_OBJECT

public:
    static bool Create(Sensors*& obj, QSharedPointer<QNetworkAccessManager>& net, QWidget* parent = 0);
    ~Sensors();

private:
    explicit Sensors(SensorModel* model, QWidget *parent = 0);

    Ui::Sensors *ui;
    SensorModel* m_model;
};

} // namespace sensors
