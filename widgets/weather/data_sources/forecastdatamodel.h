#ifndef FORECASTDATAMODEL_H
#define FORECASTDATAMODEL_H

#include "iforecastdatasource.h"

#include <QAbstractItemModel>
#include <QSharedPointer>

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
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    IForecastDataSource* m_source;

protected slots:
    void UpdateDataModel();
};

} // namespace weather

#endif // FORECASTDATAMODEL_H
