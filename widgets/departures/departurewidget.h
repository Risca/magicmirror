#pragma once

#include "data_sources/departure.h"
#include "data_sources/departure_isource.h"

#include <QList>
#include <QMap>
#include <QWidget>

class QNetworkAccessManager;

namespace departure {

class DepartureWidget : public QWidget
{
    Q_OBJECT
public:
    static bool Create(DepartureWidget* &obj, QSharedPointer<QNetworkAccessManager> &net, QWidget *parent = 0);

public slots:
    void updateDepartures(QList<departure::Departure> times);

protected:
    explicit DepartureWidget(const QList<ISource*> &sources, QWidget *parent = 0);
    QMap<int, QWidget*> m_widgets;
};

} // namespace departure
