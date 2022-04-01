#ifndef FADINGQLABEL_H
#define FADINGQLABEL_H

#include <QLabel>
#include <QTimer>
#include <QWidget>

class QPropertyAnimation;

class FadingQLabel : public QLabel
{
    Q_OBJECT
public:
    FadingQLabel(int msec_in, int msec_out, int period = 0, QWidget *parent = 0);
    ~FadingQLabel();

public slots:
    void fadeOut();
    void fadeIn();

signals:
    void fadedOut();
    void fadedIn();

private:
    QPropertyAnimation *m_fadeIn;
    QPropertyAnimation *m_fadeOut;
    QTimer m_timer;
};

#endif // FADINGQLABEL_H
