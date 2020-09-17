#ifndef CURRENTCONDITIONS_H
#define CURRENTCONDITIONS_H

#include <QWidget>
#include <QSharedPointer>

class QNetworkAccessManager;

namespace Ui {
class CurrentConditions;
}

namespace weather {

class ICurrentConditionsDataSource;

class CurrentConditions : public QWidget
{
    Q_OBJECT

public:
    explicit CurrentConditions(QSharedPointer<QNetworkAccessManager> &net, QWidget *parent = 0);
    ~CurrentConditions();

private:
    Ui::CurrentConditions *ui;
    ICurrentConditionsDataSource* m_dataSource;

private slots:
    void setTemperature(double t);
};

} // namespace weather

#endif // CURRENTCONDITIONS_H
