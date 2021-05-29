#include "schedule.h"
#include "ui_schedule.h"

#include "utils/settingsfactory.h"
#include "widgets/calendar/data_sources/icssource.h"
#include "widgets/calendar/data_sources/event.hpp"

namespace schedule {

bool Schedule::Create(Schedule *&obj, QSharedPointer<QNetworkAccessManager> &net, QWidget *parent)
{
    bool ok;
    calendar::ISource* source;
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Schedule");

    settings->beginGroup("ICS");
    ok = calendar::IcsSource::Create(source, settings, net, parent);
    settings->endGroup();

    if (ok) {
        obj = new Schedule(source, settings->value("title").toString(), parent);
    }

    return !!obj;
}

Schedule::Schedule(calendar::ISource *source, const QString &title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Schedule),
    m_source(source)
{
    ui->setupUi(this);

    if (!title.isNull()) {
        ui->title->setText(title);
    }

    // parent is probably a MirrorFrame object
    connect(parent, SIGNAL(dayChanged(QDate)), source, SLOT(sync()));
    connect(source, &calendar::ISource::finished, this, &Schedule::syncFinished);
    source->sync();
}

void Schedule::syncFinished(const QList<calendar::Event> &events)
{
    const QDate today = QDate::currentDate();
    foreach (const calendar::Event& e, events) {
        if (e.start <= today && today <= e.stop) {
            ui->current->setText(e.summary);
            break;
        }
    }
}

Schedule::~Schedule()
{
    delete m_source;
    delete ui;
}

} // namespace schedule
