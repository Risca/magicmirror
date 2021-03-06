#include "forecast.h"
#include "ui_forecast.h"

#include "data_sources/forecastdatamodel.h"
#include "data_sources/iforecastdatasource.h"

#include "utils/effects.h"
#include "utils/settingsfactory.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QSharedPointer>

namespace weather {

Forecast::Forecast(QSharedPointer<QNetworkAccessManager>& net, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Forecast)
{
    ui->setupUi(this);

    QSharedPointer<QSettings> settings = SettingsFactory::Create("Weather");

    IForecastDataSource* source;
    if (IForecastDataSource::Create(source, settings, net, this)) {
        m_forecastModel = new ForecastDataModel(source, ui->tableView);
        ui->tableView->setModel(m_forecastModel);
        ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tableView->resizeColumnsToContents();
        ui->tableView->resizeRowsToContents();
    }
    utils::ApplyFade(ui->tableView);
}

Forecast::~Forecast()
{
    delete ui;
    delete m_forecastModel;
}

} // namespace weather
