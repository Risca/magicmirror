#include "compliment.h"

#include "utils/settingsfactory.h"

#include <QDebug>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTextStream>

#define COMPLIMENT_UPDATE_INTERVAL (3 * 60 * 60 * 1000)

namespace {

void LoadComplimentsFromFile(const QString& path, QStringList& compliments)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        compliments.push_back(line);
    }
}

} // anonymous namespace

Compliment::Compliment(QWidget *parent) :
    QLabel(parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Compliments");
    if (settings->contains("anytime")) {
        QVariant compliments = settings->value("anytime");
        if (compliments.type() == QVariant::StringList) {
            m_compliments = compliments.toStringList();
        }
        // assume it's a file path
        else {
            LoadComplimentsFromFile(compliments.toString(), m_compliments);
        }
    }
    if (m_compliments.empty()) {
        m_compliments << "You're beautiful";
    }

    qDebug() << __PRETTY_FUNCTION__ << "Loaded" << m_compliments.size() << "compliments";

    m_timer.setTimerType(Qt::VeryCoarseTimer);
    m_timer.setInterval(COMPLIMENT_UPDATE_INTERVAL);

    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
    effect->setOpacity(0.0f);
    this->setGraphicsEffect(effect);

    m_fadeOut = new QPropertyAnimation(effect, "opacity");
    m_fadeOut->setDuration(1000);
    m_fadeOut->setStartValue(1.0f);
    m_fadeOut->setEndValue(0.0f);

    m_fadeIn = new QPropertyAnimation(effect, "opacity");
    m_fadeIn->setDuration(1000);
    m_fadeIn->setStartValue(0.0f);
    m_fadeIn->setEndValue(1.0f);

    connect(m_fadeOut, SIGNAL(finished()), this, SLOT(textFadedOut()));
    connect(m_fadeIn, SIGNAL(finished()), &m_timer, SLOT(start()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(changeCompliment()));

    changeCompliment();
}

void Compliment::setText(const QString &text)
{
    m_newText = text;
    m_fadeOut->start();
}

void Compliment::textFadedOut()
{
    QLabel::setText(m_newText);
    m_fadeIn->start();
}

void Compliment::changeCompliment()
{
    std::random_shuffle(m_compliments.begin(), m_compliments.end());

    if (m_compliments.first() == this->text()) {
        this->setText(m_compliments.last());
    }
    else {
        this->setText(m_compliments.first());
    }
}
