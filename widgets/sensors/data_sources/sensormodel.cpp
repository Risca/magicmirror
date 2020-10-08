#include "sensormodel.h"

#include "sensor_isource.h"
#include "utils/sensordata.h"

#include <QBrush>
#include <QDebug>
#include <QIcon>
#include <QModelIndex>

#include <algorithm>

namespace sensors {

namespace {

QString Temperature(double t)
{
    return QString::number(t, 'f', 1) + QString::fromUtf8("°");
}

} // anonymous namespace

enum Column {
    COL_NAME,
    COL_ICON,
    COL_TEMPERATURE,
    COL_LAST = COL_TEMPERATURE,
    COL_COUNT,
};

SensorModel::SensorModel(ISource *dataSource, QObject *parent) :
    QAbstractItemModel(parent),
    m_source(dataSource)
{
    m_source->setParent(this);

    connect(m_source, SIGNAL(sensorDataUpdated(const QList<utils::SensorData>&)),
            this, SLOT(addData(const QList<utils::SensorData>&)));
}

SensorModel::~SensorModel()
{
    delete m_source;
}

QModelIndex SensorModel::index(int row, int column, const QModelIndex &) const
{
    return createIndex(row, column);
}

QModelIndex SensorModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int SensorModel::rowCount(const QModelIndex &parent) const
{
    // return zero for child indexes (flat hierarchy)
    if (parent.isValid())
        return 0;

    return m_data.size();
}

int SensorModel::columnCount(const QModelIndex &parent) const
{
    // return zero for child indexes (flat hierarchy)
    if (parent.isValid())
        return 0;

    return COL_COUNT;
}

QVariant SensorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int col = index.column();
    const int row = index.row();

    if (row >= m_data.size()) {
        return QVariant();
    }

    const utils::SensorData& d = m_data.at(row);

    switch (role) {
    case Qt::DecorationRole:
        // icons
        if (col == COL_ICON) {
            const QDateTime yesterday = QDateTime::currentDateTime().addDays(-1);
            if (d.timestamp < yesterday) {
                return QIcon(":/sensors/icons/low_battery.svg");
            }
        }
        break;

    case Qt::DisplayRole:
        switch (col) {
        case COL_NAME:
            return d.source;
        case COL_TEMPERATURE:
            return Temperature(d.values[utils::TEMPERATURE]);
        default:
            break;
        }
        break;

    case Qt::TextAlignmentRole:
        break;
    case Qt::ForegroundRole:
        if (col == COL_TEMPERATURE) {
            const QDateTime yesterday = QDateTime::currentDateTime().addDays(-1);
            if (d.timestamp < yesterday) {
                return QBrush(Qt::red);
            }
        }
        break;
    default:
        break;
    }
    return QVariant();
}

void SensorModel::addData(const QList<utils::SensorData> &data)
{
    beginResetModel();
    m_data = data;
    std::sort(m_data.begin(), m_data.end(), utils::SensorData::SortbySource());
    endResetModel();
}

} // namespace sensors
