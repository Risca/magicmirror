#include "departurewidget.h"

#include "qsettings.h"
#include "singledeparturewidget.h"
#include "data_sources/resrobotdepartures.h"
#include "utils/settingsfactory.h"

#include <algorithm>
#include <QDebug>
#include <QVBoxLayout>

namespace departure {

bool DepartureWidget::Create(DepartureWidget *&obj, QSharedPointer<QNetworkAccessManager> &net, QWidget *parent)
{
    QList<ISource*> dataSources;
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Departures");
    foreach (const QString& source, settings->value("sources").toStringList()) {
        qDebug() << __PRETTY_FUNCTION__ << "Trying to create source:" << source;
        if (source == "resrobot") {
            ISource* obj;
            if (ResrobotDepartures::Create(obj, net, parent)) {
                dataSources.push_back(obj);
            }
        }
    }
    if (dataSources.empty()) {
        return false;
    }
    obj = new DepartureWidget(dataSources, parent);
    return true;
}

DepartureWidget::DepartureWidget(const QList<ISource *> &sources, QWidget *parent) :
    QWidget(parent)
{
    this->setLayout(new QVBoxLayout);
    foreach (ISource *source, sources) {
        connect(source, &ISource::newDepartureTimes, this, &DepartureWidget::updateDepartures);
        source->requestNewDepartures();
    }
}

void DepartureWidget::updateDepartures(QList<Departure> times)
{
    qDebug() << __PRETTY_FUNCTION__ << "updating" << times.size() << "times";
    if (!times.empty()) {
        SingleDepartureWidget *w;
        const QDateTime now = QDateTime::currentDateTime();
        std::sort(times.begin(), times.end());
        int id = times.first().stopID;
        qDebug() << __PRETTY_FUNCTION__ << id;
        if (m_widgets.contains(id)) {
            w = dynamic_cast<SingleDepartureWidget*>(m_widgets[id]);
        }
        else {
            qDebug() << __PRETTY_FUNCTION__ << "adding new widget for" << times.first().stopName;
            w = new SingleDepartureWidget(this);
            m_widgets[id] = w;
            this->layout()->addWidget(w);
        }
        w->SetDepartureData(times.first().stopName,
                            now.secsTo(times[0].time) / 60,
                            times.count() == 1 ? -1 : now.secsTo(times[1].time) / 60);
    }
}

} // namespace departure
