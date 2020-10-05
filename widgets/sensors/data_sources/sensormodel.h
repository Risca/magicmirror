#pragma once

#include "sensordata.h"

#include <QAbstractItemModel>
#include <QList>

class QModelIndex;

namespace sensors {

class ISource;

class SensorModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    SensorModel(ISource* dataSource, QObject* parent = 0);
    virtual ~SensorModel();

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    ISource* m_source;

    QList<SensorData> m_data;

protected slots:
    void addData(const QList<SensorData>& data);
};

} // namespace sensors
