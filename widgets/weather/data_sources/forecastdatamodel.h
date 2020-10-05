#pragma once

#include "iforecastdatasource.h"

#include <QAbstractItemModel>

class QModelIndex;

namespace weather {

class ForecastDataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ForecastDataModel(IForecastDataSource* dataSource, QObject* parent = NULL);
    virtual ~ForecastDataModel();

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

protected:
    IForecastDataSource* m_source;

protected slots:
    void UpdateDataModel();
};

} // namespace weather
