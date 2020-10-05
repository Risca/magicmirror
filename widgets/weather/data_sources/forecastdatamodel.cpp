#include "forecastdatamodel.h"

#include <QBrush>
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QLocale>
#include <QModelIndex>
#include <QString>
#include <QVariant>


namespace weather {

namespace {

QString Capitalized(const QString& str, const QLocale& locale = QLocale())
{
    QString firstLetter = str.left(1);
    return locale.toUpper(firstLetter) + str.right(str.size() - 1);
}

QString Date(const QDateTime& dt)
{
    const qint64 HOURS = 3600;
    const QDateTime later = QDateTime::currentDateTime().addSecs(16 * HOURS);
    const QLocale locale;

    if (dt >= later) {
        QString day = locale.toString(dt.date(), "ddd");
        return Capitalized(day);
    }
    else {
        return locale.toString(dt.time(), QLocale::ShortFormat);
    }
    return QString();
}

QString Temperature(double t)
{
    return QString::number(t, 'f', 1) + QString::fromUtf8("°");
}

QString Humidity(double h)
{
    return QString::number(h, 'f', 0) + "%";
}

QString Precipitation(double p)
{
    return QString::number(p, 'f', 0) + " mm";
}

} // anonymous namespace

ForecastDataModel::ForecastDataModel(IForecastDataSource *dataSource, QObject *parent) :
    QAbstractItemModel(parent),
    m_source(dataSource)
{
    connect(m_source, SIGNAL(newForecastAvailable()), this, SLOT(UpdateDataModel()));
}

ForecastDataModel::~ForecastDataModel()
{
    delete m_source;
}

QModelIndex ForecastDataModel::index(int row, int column, const QModelIndex &) const
{
    return createIndex(row, column);
}

QModelIndex ForecastDataModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int ForecastDataModel::rowCount(const QModelIndex &parent) const
{
    // return zero for child indexes (flat hierarchy)
    if (parent.isValid())
        return 0;

    return m_source->forecast().count();
}

int ForecastDataModel::columnCount(const QModelIndex &parent) const
{
    // return zero for child indexes (flat hierarchy)
    if (parent.isValid())
        return 0;

    return 6;
}

QVariant ForecastDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int col = index.column();
    const int row = index.row();

    const QList<Data>& forecastList = m_source->forecast();

    if (row >= forecastList.size()) {
        return QVariant();
    }

    const Data forecast = forecastList.at(row);

    switch (role) {
    case Qt::DecorationRole:
        if (col == 1) {
            return forecast.icon;
        }
        break;

    case Qt::DisplayRole:
        switch (col) {
        case 0:
            return Date(forecast.dateTime);
        case 1:
            // Only icons in this column
            break;
        case 2:
            if (forecast.values.contains(weather::TEMPERATURE_HIGH)) {
                return Temperature(forecast.values[weather::TEMPERATURE_HIGH]);
            }
            return Temperature(forecast.values[weather::TEMPERATURE]);
        case 3:
            if (forecast.values.contains(weather::TEMPERATURE_LOW)) {
                return Temperature(forecast.values[weather::TEMPERATURE_LOW]);
            }
            break;
        case 4:
            if (forecast.values.contains(weather::HUMIDITY)) {
                return Humidity(forecast.values[weather::HUMIDITY]);
            }
            break;
        case 5:
            if (forecast.values.contains(weather::PRECIPITATION)) {
                return Precipitation(forecast.values[weather::PRECIPITATION]);
            }
            break;
        default:
            break;
        }
        break;

    case Qt::TextAlignmentRole:
        if (col == 4) {
            return static_cast<Qt::Alignment::Int>(Qt::AlignVCenter | Qt::AlignRight);
        }
        break;
    case Qt::ForegroundRole:
        switch (col) {
        case 2:
            if (forecast.values.contains(weather::TEMPERATURE_HIGH)) {
                return QBrush(Qt::yellow);
            }
            break;
        case 3:
            if (forecast.values.contains(weather::TEMPERATURE_LOW)) {
                return QBrush(Qt::blue);
            }
        default:
            break;
        }
        break;

    default:
        break;
    }
    return QVariant();
}

void ForecastDataModel::UpdateDataModel()
{
#if 1
    beginResetModel();
    endResetModel();
#else
    emit dataChanged(index(0, 0), index(rowCount(), columnCount()));
#endif
}

} // namespace weather
