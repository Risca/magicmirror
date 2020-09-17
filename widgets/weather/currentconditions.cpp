#include "currentconditions.h"
#include "ui_currentconditions.h"

#include "settingsfactory.h"
#include "data_sources/icurrentconditionsdatasource.h"

namespace weather {

CurrentConditions::CurrentConditions(QSharedPointer<QNetworkAccessManager> &net, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CurrentConditions),
    m_dataSource(0)
{
    ui->setupUi(this);

    QSharedPointer<QSettings> settings = SettingsFactory::Create("Weather");

    if (ICurrentConditionsDataSource::Create(m_dataSource, settings, net, this)) {
        connect(m_dataSource, SIGNAL(image(QPixmap)), ui->image, SLOT(setPixmap(QPixmap)));
        connect(m_dataSource, SIGNAL(temperature(double)), this, SLOT(setTemperature(double)));
    }
}

CurrentConditions::~CurrentConditions()
{
    delete m_dataSource;
    delete ui;
}

void CurrentConditions::setTemperature(double t)
{
    ui->temperature->setText(QString::number(t, 'f', 1) + QString::fromUtf8("°"));
}

} // namespace weather
