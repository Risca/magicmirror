#include "sensormodel.h"

#include "sensor_isource.h"
#include "sensordata.h"

#include <QBrush>
#include <QDebug>
#include <QIcon>
#include <QModelIndex>

#include <algorithm>

namespace sensors {

enum Column {
    COL_NAME,
    COL_VALUE,
    COL_LAST = COL_VALUE,
    COL_COUNT,
};

SensorModel::SensorModel(ISource *dataSource, QObject *parent) :
    QAbstractItemModel(parent),
    m_source(dataSource)
{
    m_source->setParent(this);

    connect(m_source, SIGNAL(sensorDataUpdated(const QList<SensorData>&)),
            this, SLOT(addData(const QList<SensorData>&)));
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

    const SensorData d = m_data.at(row);

    switch (role) {
    case Qt::DecorationRole:
        // icons
        if (col == COL_VALUE) {
            const QDateTime yesterday = QDateTime::currentDateTime().addDays(-1);
            if (d.lastUpdated < yesterday) {
                return QIcon(":/sensors/icons/low_battery.svg");
            }
        }
        break;

    case Qt::DisplayRole:
        switch (col) {
        case COL_NAME:
            return d.name;
        case COL_VALUE:
            return d.value;
        default:
            break;
        }
        break;

    case Qt::TextAlignmentRole:
        if (col == COL_LAST) {
            return static_cast<Qt::Alignment::Int>(Qt::AlignVCenter | Qt::AlignRight);
        }
        break;
    case Qt::ForegroundRole:
        if (col == COL_VALUE) {
            const QDateTime yesterday = QDateTime::currentDateTime().addDays(-1);
            if (d.lastUpdated < yesterday) {
                return QBrush(Qt::red);
            }
        }
        break;
    default:
        break;
    }
    return QVariant();
}

void SensorModel::addData(const QList<SensorData> &data)
{
    beginResetModel();
    m_data = data;
    std::sort(m_data.begin(), m_data.end());
    endResetModel();
}

} // namespace sensors
