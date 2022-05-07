#include "singledeparturewidget.h"
#include "ui_singledeparturewidget.h"

SingleDepartureWidget::SingleDepartureWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleDepartureWidget)
{
    ui->setupUi(this);
}

SingleDepartureWidget::~SingleDepartureWidget()
{
    delete ui;
}

void SingleDepartureWidget::SetDepartureData(const QString &stop, int next, int thereafter)
{
    ui->stop->setText(stop);
    ui->next->display(next);
    if (thereafter < 0) {
        ui->thereafter->display("-");
    }
    else {
        ui->thereafter->display(thereafter);
    }
}
