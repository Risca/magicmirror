#include "sensors.h"
#include "ui_sensors.h"

#include "data_sources/sensor_isource.h"
#include "data_sources/sensormodel.h"

#include "utils/effects.h"
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
    else {
        obj = nullptr;
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
    ui->sensorList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->sensorList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->sensorList->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    utils::ApplyFade(ui->sensorList);
}

Sensors::~Sensors()
{
    delete ui;
    delete m_model;
}

} // namespace sensors
