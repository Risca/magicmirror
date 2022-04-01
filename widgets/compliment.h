#pragma once

#include "fadingqlabel.h"
#include <QString>
#include <QStringList>
#include <QTimer>

class Compliment : public FadingQLabel
{
    Q_OBJECT

public:
    explicit Compliment(QWidget *parent = 0);

signals:

public slots:
    virtual void setText(const QString &text);
    void changeCompliment();

private:
    QString m_newText;
    QStringList m_compliments;
};
