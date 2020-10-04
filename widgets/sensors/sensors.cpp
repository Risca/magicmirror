#include "sensors.h"
#include "ui_sensors.h"

#include "data_sources/sensor_isource.h"
#include "data_sources/sensordata.h"

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
        obj = new Sensors(src, parent);
    }

    return !!obj;
}

Sensors::Sensors(ISource* dataSource, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Sensors),
    m_ds(dataSource)
{
    ui->setupUi(this);

    ui->sensorList->sortByColumn(0, Qt::AscendingOrder);

    connect(m_ds, SIGNAL(sensorDataUpdated(const QList<SensorData>&)),
            this, SLOT(SetSensorData(const QList<SensorData>&)));
}

Sensors::~Sensors()
{
    delete ui;
}

void Sensors::SetSensorData(const QList<SensorData> &list)
{
    const QDateTime yesterday = QDateTime::currentDateTime().addDays(-1);
    bool sorting = ui->sensorList->isSortingEnabled();

    ui->sensorList->clearContents();
    ui->sensorList->setRowCount(list.count());

    ui->sensorList->setSortingEnabled(false);

    for (int row = 0; row < list.count(); ++row) {
        const SensorData& d = list[row];
        ui->sensorList->setItem(row, 0, new QTableWidgetItem(d.name));

        QTableWidgetItem* value = new QTableWidgetItem(d.value);
        value->setTextAlignment(Qt::AlignRight);
        if (d.lastUpdated < yesterday) {
            value->setForeground(Qt::red);
        }
        ui->sensorList->setItem(row, 1, value);
    }
    ui->sensorList->setSortingEnabled(sorting);
    ui->sensorList->resizeColumnToContents(1);
}

} // namespace sensors
