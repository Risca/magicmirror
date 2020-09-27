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
        connect(m_dataSource, SIGNAL(image(QPixmap)), this, SLOT(setPixmap(QPixmap)));
        connect(m_dataSource, SIGNAL(temperature(double)), this, SLOT(setTemperature(double)));
        connect(m_dataSource, SIGNAL(skyConditions(QString)), ui->label, SLOT(setText(QString)));
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

void CurrentConditions::setPixmap(const QPixmap &pixmap)
{
    // scale image to same 3/2 the size of text
    QFontInfo fi = ui->temperature->fontInfo();
    int height = fi.pixelSize() * pixmap.devicePixelRatio() * 3 / 2;
    ui->image->setPixmap(pixmap.scaledToHeight(height));
}

} // namespace weather
