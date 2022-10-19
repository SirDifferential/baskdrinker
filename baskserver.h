#ifndef BASKSERVER_H
#define BASKSERVER_H

#include <QObject>
#include <QtWebSockets/QtWebSockets>
#include "baskclient.h"
#include <vector>

typedef enum {
    INVALID_JSON = 4000,
    INVALID_PARAM = 4001
} WSErrorCode;

/**
 * Websocket server that publishes info about turn timers
 */
class BaskServer : public QObject {
    Q_OBJECT
public:
    explicit BaskServer(QObject *parent = nullptr);
    virtual ~BaskServer();

    /**
     * Starts the server. Returns true on success, otherwise false.
     * Before calling again, stop server if it's already running.
     */
    bool init();

    /**
     * Stops the server and disconnects all clients.
     */
    void stop();

    /**
     * Sends the given message to all clients that have subscribed to msgType
     */
    void publish(const QString& msgType, const QString& msg);

    /**
     * Returns a pointer to a connected client that holds the given socket,
     * or nullptr if none found.
     */
    BaskClient* getClient(QWebSocket* socket);

    /**
     * Utility function that sends a JSON error to the client
     */
    void handleInvalidMessage(BaskClient* cl, WSErrorCode code, const QString& errorMsg);

    /**
     * Assigns the port to use when hosting a server. Does not change the port before the
     * server is restarted.
     */
    void setPort(int port);

    /**
     * Returns true if the websocket server is listening
     */
    bool isRunning();

    /**
     * Returns the number of connected clients
     */
    int getClientCount();
private:

    qint16 m_port;
    bool m_debug;
    QWebSocketServer* m_webSocketServer;

    /**
     * Container of connected clients
     */
    std::vector<BaskClient*> m_clients;
signals:
    /**
     * Emitted when the number of connected clients changes.
     * Parameter is the number of connected clients.
     */
    void clientsChanged(int);

    /**
     * Emitted when the server is started or stopped.
     * bool is server running or not state, QString is additional info.
     */
    void stateChanged(bool, QString);
private slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void closed();
};

#endif // BASKSERVER_H
