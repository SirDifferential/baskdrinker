#ifndef BASKCLIENT_H
#define BASKCLIENT_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>

/**
 * A connected websocket client containing socket + other info,
 * including list of events to which this client has subscribed
 */
class BaskClient : public QObject {
    Q_OBJECT
public:
    explicit BaskClient(QObject *parent = nullptr, QWebSocket* socket = nullptr);
    virtual ~BaskClient();
    QWebSocket* m_socket;
    QString m_name;
    std::vector<QString> m_subscriptions;
private:
signals:

};

#endif // BASKCLIENT_H
