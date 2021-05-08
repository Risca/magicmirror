#pragma once

#include <QLabel>
#include <QString>
#include <QStringList>
#include <QTimer>

class QPropertyAnimation;

class Compliment : public QLabel
{
    Q_OBJECT

public:
    explicit Compliment(QWidget *parent = 0);

signals:

public slots:
    virtual void setText(const QString &text);
    void changeCompliment();

private:
    QPropertyAnimation *m_fadeIn;
    QPropertyAnimation *m_fadeOut;

    QTimer m_timer;
    QString m_newText;
    QStringList m_compliments;

private slots:
    void textFadedOut();
};
