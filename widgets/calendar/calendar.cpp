#include "calendar.h"
#include "ui_calendar.h"

#include "data_sources/event.hpp"
#include "data_sources/cal_isource.h"

#include "utils/effects.h"
#include "utils/settingsfactory.h"

#include <QApplication>
#include <QAbstractTableModel>
#include <QBrush>
#include <QDebug>
#include <QFontMetrics>
#include <QLocale>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QTableView>
#include <QTextCharFormat>
#include <QUrl>

namespace calendar {

QDate CurrentMonth(const QLocale &locale)
{
    const QDate today = QDate::currentDate();
    QDate thisMonth = QDate(today.year(), today.month(), 1);
    while (thisMonth.dayOfWeek() != locale.firstDayOfWeek()) {
        thisMonth = thisMonth.addDays(-1);
    }
    return thisMonth;
}

namespace {

class CalendarModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    CalendarModel(QObject *parent = 0) :
        QAbstractTableModel(parent)
    { }
    virtual ~CalendarModel() {}

    int rowCount(const QModelIndex &) const { return m_currentEvents.size(); }
    int columnCount(const QModelIndex &) const { return 2; }

    QVariant data(const QModelIndex &index, int role) const;

    void setView(QTableView *view)
    { m_view = view; }

public slots:
    void setEvents(const QList<calendar::Event> &events);

private:
    QTableView *m_view;
    QList<Event> m_currentEvents;

    QTextCharFormat formatForColumn(int column) const;
};

QVariant CalendarModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int column = index.column();

    if (row > m_currentEvents.size()) {
        return QVariant();
    }

    const Event &e = m_currentEvents[row];

    if (role == Qt::DisplayRole) {
        switch (column) {
        case 0:
            if (m_view)
                return m_view->locale().toString(e.start, QLocale::ShortFormat);
            else
                return e.start.toString(Qt::DefaultLocaleShortDate);
        case 1:
            return e.summary;
        default:
            return QVariant();
        }
    }

    if (role == Qt::DecorationRole && column == 1) {
        QFontMetrics fm = m_view ? m_view->fontMetrics() : QApplication::fontMetrics();
        // Make icon half as big as the available space
        QPixmap pix(fm.height() / 2, fm.height() / 2);
        pix.fill(Qt::transparent);

        QPainter *p = new QPainter(&pix);
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(Qt::NoPen);
        p->setBrush(QBrush(e.color));
        p->drawEllipse(pix.rect());
        delete p;

        return pix;
    }

    QTextCharFormat fmt = formatForColumn(column);
    if (role == Qt::BackgroundRole)
        return fmt.background().color();
    if (role == Qt::ForegroundRole)
        return fmt.foreground().color();
    if (role == Qt::FontRole)
        return fmt.font();
    if (role == Qt::ToolTipRole)
        return fmt.toolTip();
    return QVariant();

    return QVariant();
}

void CalendarModel::setEvents(const QList<Event> &events)
{
    const QDate today = QDate::currentDate();

    beginResetModel();
    m_currentEvents.clear();
    m_currentEvents.reserve(events.size());
    foreach (const Event& e, events) {
        if (e.stop >= today)
            m_currentEvents.push_back(e);
    }
    endResetModel();
}

QTextCharFormat CalendarModel::formatForColumn(int column) const
{
    QPalette pal;
    QPalette::ColorGroup cg = QPalette::Active;
    if (m_view) {
        pal = m_view->palette();
        if (!m_view->isEnabled())
            cg = QPalette::Disabled;
        else if (!m_view->isActiveWindow())
            cg = QPalette::Inactive;
    }
    QTextCharFormat format;
    QFont font = m_view ? m_view->font() : QFont();
    if (column == 0) {
        font.setPointSize(font.pointSize() * 2 / 3);
    }
    format.setFont(font);
    format.setBackground(pal.brush(cg, QPalette::Base));
    if (column == 0) {
        format.setForeground(pal.brush(QPalette::Disabled, QPalette::Text));
    }
    else {
        format.setForeground(pal.brush(cg, QPalette::Text));
    }
    return format;
}

} // anonymous namespace

#define CALENDAR_SYNC_PERIOD (2 * 60 * 60 * 1000)

bool Calendar::Create(Calendar *&cal, QSharedPointer<QNetworkAccessManager> net, QWidget *parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Calendar");

    QList<ISource*> sources;
    if (ISource::Create(sources, settings, net, parent)) {
        cal = new Calendar(sources, parent);
        if (!cal) {
            qDeleteAll(sources);
        }
    }
    return (cal == NULL ? false : true);
}

Calendar::Calendar(const QList<ISource *> &dataSources, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calendar),
      m_sources(dataSources)
{
    ui->setupUi(this);

    utils::ApplyFade(ui->events);

    CalendarModel *model = new CalendarModel(this);

    ui->events->setModel(model);
    model->setView(ui->events);
    ui->events->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->events->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->events->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->events->setWordWrap(false);
    ui->events->setTextElideMode(Qt::ElideRight);

    ui->calendar->setFirstDayOfWeek(locale().firstDayOfWeek());

    connect(this, &Calendar::setEvents, model, &CalendarModel::setEvents);
    connect(this, &Calendar::setEvents, ui->calendar, &CustomCalendarWidget::setEvents);

    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setInterval(CALENDAR_SYNC_PERIOD);
    m_timer.setSingleShot(false);

    foreach(ISource *source, m_sources) {
        connect(&m_timer, &QTimer::timeout, source, &ISource::sync);
        connect(source, &ISource::finished,
                this, [this, source](const QList<Event>&events) { aggregateEvents(source, events); });
        source->sync();
    }

    m_timer.start();
}

void Calendar::aggregateEvents(ISource *source, const QList<Event> &events)
{
    QList<Event> allEvents;
    m_events[source] = events;
    foreach (const QList<Event>& sourceEvents, m_events) {
        allEvents.append(sourceEvents);
    }
    std::sort(allEvents.begin(), allEvents.end());
    emit setEvents(allEvents);
}

Calendar::~Calendar()
{
    qDeleteAll(m_sources);
    delete ui;
}

void Calendar::changeDay(const QDate &day)
{
    ui->month_and_year->setDate(day);
}

} // namespace calendar

#include "calendar.moc"
