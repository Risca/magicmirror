#include "sensors.h"
#include "ui_sensors.h"

#include "data_sources/sensor_isource.h"
#include "data_sources/sensormodel.h"

#include "utils/settingsfactory.h"

#include <QDebug>
#include <QTableWidgetItem>

namespace sensors {

bool Sensors::Create(Sensors *&obj, QSharedPointer<QNetworkAccessManager> &net, QWidget *parent)
{
    ISource* src;
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Sensors");

    obj = 0;

    if (ISource::Create(src, settings, net)) {
        obj = new Sensors(new SensorModel(src), parent);
    }

    return !!obj;
}

Sensors::Sensors(SensorModel *model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Sensors),
    m_model(model)
{
    ui->setupUi(this);

    m_model->setParent(this);

    ui->sensorList->setModel(m_model);
    ui->sensorList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->sensorList->horizontalHeader()->setSectionResizeMode(model->columnCount() - 1, QHeaderView::ResizeToContents);
    ui->sensorList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

Sensors::~Sensors()
{
    delete ui;
    delete m_model;
}

} // namespace sensors
