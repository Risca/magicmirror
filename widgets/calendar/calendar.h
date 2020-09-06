#ifndef CALENDAR_H
#define CALENDAR_H

#include <QList>
#include <QLocale>
#include <QSharedPointer>
#include <QWidget>

class QNetworkAccessManager;

namespace Ui { class Calendar; }

namespace calendar {

class ISource;
struct Event;

class Calendar : public QWidget
{
    Q_OBJECT

public:
    static bool Create(Calendar*& cal, QSharedPointer<QNetworkAccessManager> net, QWidget *parent = 0);
    virtual ~Calendar();

protected:
    virtual void resizeEvent(QResizeEvent* event);

private:
    Calendar(ISource* dataSource, QWidget *parent = NULL);
    Ui::Calendar *ui;
    ISource* m_source;
    QLocale m_locale;

    void StartMidnightTimer();

private slots:
    void NewEventList(const QList<calendar::Event>&);
    void changeDay();
};

} // namespace calendar

#endif // CALENDAR_H
