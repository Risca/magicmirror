#include "compliment.h"

#include "utils/settingsfactory.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#define COMPLIMENT_UPDATE_INTERVAL (3 * 60 * 60 * 1000)

Compliment::Compliment(QWidget *parent) :
    QLabel(parent)
{
    QSharedPointer<QSettings> settings = SettingsFactory::Create("Compliments");
    if (settings->contains("anytime")) {
        m_compliments = settings->value("anytime").toStringList();
    }
    else {
        m_compliments << "Hello sexy!" << "Looking good!";
    }

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
