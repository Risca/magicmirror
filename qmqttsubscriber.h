/*
 * Copyright (c) 2019 <copyright holder> <email>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef QMQTTSUBSCRIBER_H
#define QMQTTSUBSCRIBER_H

#include <qmqtt/qmqtt>

/**
 * @todo write docs
 */
class QMqttSubscriber : public QMQTT::Client
{
    Q_OBJECT
public:
    QMqttSubscriber(const QHostAddress& host, const quint16 port, QObject* parent) : QMQTT::Client(host, port, parent)
    {
        connect(this, &QMqttSubscriber::connected, this, &QMqttSubscriber::onConnected);
        connect(this, &QMqttSubscriber::disconnected, this, &QMqttSubscriber::onDisconnected);
        connect(this, &QMqttSubscriber::received, this, &QMqttSubscriber::onReceived);
        connect(this, &QMqttSubscriber::error, this, &QMqttSubscriber::onError);
    }
    virtual ~QMqttSubscriber() {}

    void subscribeToTopic(QString);

signals:
    void messageReceivedOnTopic(QString, QString);
    void connectionComplete();
    void disconnectedEvent();

public slots:
    void onConnected();
    void onReceived(const QMQTT::Message&);
    void onDisconnected();
    void onError(const QMQTT::ClientError error);
};

#endif // QMQTTSUBSCRIBER_H
