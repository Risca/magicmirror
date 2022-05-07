#pragma once

#include <QWidget>

namespace Ui {
class SingleDepartureWidget;
}

class SingleDepartureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SingleDepartureWidget(QWidget *parent = 0);
    ~SingleDepartureWidget();

    void SetDepartureData(const QString& stop, int next, int thereafter);

private:
    Ui::SingleDepartureWidget *ui;
};
