#ifndef FORECAST_H
#define FORECAST_H

#include <QWidget>
#include <QSharedPointer>

class QAbstractItemModel;
class QNetworkAccessManager;

namespace Ui {
class Forecast;
}

namespace weather {

class Forecast : public QWidget
{
    Q_OBJECT

public:
    explicit Forecast(QSharedPointer<QNetworkAccessManager> &net, QWidget *parent = NULL);
    ~Forecast();

private:
    Ui::Forecast *ui;

    QAbstractItemModel* m_forecastModel;
};

} // namespace weather

#endif // FORECAST_H
