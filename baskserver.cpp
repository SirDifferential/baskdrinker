#include "baskserver.h"
#include <QJsonDocument>

BaskServer::BaskServer(QObject *parent) : QObject{parent} {
    m_webSocketServer = new QWebSocketServer(QStringLiteral("Bäska Server"), QWebSocketServer::NonSecureMode, this);
    m_port = 10666;
    m_debug = true;
}

BaskServer::~BaskServer() {
    emit stateChanged(false, "Server not running");

    qDebug() << "Closing server in dtor";
    m_webSocketServer->close();

    qDebug() << "deleting clients in dtor";
    qDeleteAll(m_clients.begin(), m_clients.end());
}

bool BaskServer::init() {

    if (!m_webSocketServer->listen(QHostAddress::Any, m_port)) {
        qDebug() << "Failed to start websocket server at port: " << m_port;
        emit stateChanged(false, "Failed to start websocket server at port: " + QString::number(m_port));
        return false;
    }

    qDebug() << "Server listening on port" << m_port;

    connect(m_webSocketServer, &QWebSocketServer::newConnection, this, &BaskServer::onNewConnection);
    connect(m_webSocketServer, &QWebSocketServer::closed, this, &BaskServer::closed);
    emit stateChanged(true, "Server running");
    return true;
}

void BaskServer::closed() {
    qDebug() << "websocket server closed";
    emit stateChanged(false, "Server not running");
}

void BaskServer::setPort(int port) {
    m_port = port;
}

bool BaskServer::isRunning() {
    return m_webSocketServer->isListening();
}

int BaskServer::getClientCount() {
    return (int)m_clients.size();
}

void BaskServer::stop() {
    // might not be sent before server closes
    publish("connection_close", "server closing normally");
    m_clients.clear();
    m_webSocketServer->close();

    // Emit can happen via closed(), but if server is already closed, then
    // the emit in closed() is not done.
    emit stateChanged(false, "Server not running");
}

void BaskServer::onNewConnection() {
    QWebSocket* socket = m_webSocketServer->nextPendingConnection();

    BaskClient* c = new BaskClient(this, socket);

    connect(socket, &QWebSocket::textMessageReceived, this, &BaskServer::processTextMessage);
    connect(socket, &QWebSocket::binaryMessageReceived, this, &BaskServer::processBinaryMessage);
    connect(socket, &QWebSocket::disconnected, this, &BaskServer::socketDisconnected);

    // By default, inform clients about close events before they happen
    c->m_subscriptions.push_back("connection_close");

    m_clients.push_back(c);
    emit clientsChanged((int)m_clients.size());
}

BaskClient* BaskServer::getClient(QWebSocket* socket) {
    auto iter = std::find_if(m_clients.begin(), m_clients.end(), [&](const BaskClient* cl){
        return cl->m_socket == socket;
    });

    if (iter != m_clients.end()) {
        return *iter;
    }

    return nullptr;
}

void BaskServer::handleInvalidMessage(BaskClient* cl, WSErrorCode code, const QString& errMsg) {

    QJsonObject obj;
    obj["msgType"] = "error";
    obj["errorCode"] = code;
    obj["details"] = errMsg;

    QJsonDocument doc(obj);
    QString strJson = doc.toJson(QJsonDocument::Compact);

    cl->m_socket->sendTextMessage(strJson);
    qDebug() << "sent error string: " << strJson;
}

QString valTypeToString(QJsonValue::Type val) {
    switch (val) {
    case QJsonValue::Null: return "null";
    case QJsonValue::Bool: return "bool";
    case QJsonValue::Double: return "double";
    case QJsonValue::String: return "string";
    case QJsonValue::Array: return "array";
    case QJsonValue::Object: return "object";
    default: return "unknown";
    }
}

/**
 * Ensures given field exists in the JSON object and is of correct type.
 * If all is in order, empty QString is returned.
 * Otherwise an error string is returned.
 */
QString ensureField(const QJsonObject& obj, QString field, QJsonValue::Type t) {
    if (!obj.contains(field)) {
        return "Missing field: " + field;
    }

    switch (t) {
    case QJsonValue::Null:
        if (!obj[field].isNull()) {
            return "Invalid type: expected: null, got: " + valTypeToString(obj[field].type());
        }
    case QJsonValue::Bool:
        if (!obj[field].isBool()) {
            return "Invalid type: expected: bool, got: " + valTypeToString(obj[field].type());
        }
    case QJsonValue::Double:
        if (!obj[field].isDouble()) {
            return "Invalid type: expected: double, got: " + valTypeToString(obj[field].type());
        }
    case QJsonValue::String:
        if (!obj[field].isString()) {
            return "Invalid type: expected: string, got: " + valTypeToString(obj[field].type());
        }
    case QJsonValue::Array:
        if (!obj[field].isArray()) {
            return "Invalid type: expected: array, got: " + valTypeToString(obj[field].type());
        }
    case QJsonValue::Object:
        if (!obj[field].isObject()) {
            return "Invalid type: expected: object, got: " + valTypeToString(obj[field].type());
        }
    default:
        break;
    }

    return "";
}

void BaskServer::processTextMessage(QString message) {
    QWebSocket* socket = qobject_cast<QWebSocket *>(sender());

    if (m_debug) {
        qDebug() << "Message received:" << message;
    }

    auto cl = getClient(socket);
    if (cl == nullptr) {
        qDebug() << "text message from client for which we have no socket";
        return;
    }

    QJsonParseError jserr;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &jserr);
    if (doc.isNull()) {
        QString err = "Failed parsing JSON document: " + jserr.errorString();
        handleInvalidMessage(cl, WSErrorCode::INVALID_JSON, err);
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.isEmpty()) {
        QString err = "Failed parsing object from valid JSON";
        handleInvalidMessage(cl, WSErrorCode::INVALID_JSON, err);
        return;
    }

    QString err = ensureField(obj, "msg_type", QJsonValue::String);
    if (!err.isEmpty()) {
        handleInvalidMessage(cl, WSErrorCode::INVALID_PARAM, err);
        return;
    }

    QString msgtype = obj["msg_type"].toString();

    if (msgtype == "hello") {

        QString err = ensureField(obj, "name", QJsonValue::String);
        if (!err.isEmpty()) {
            handleInvalidMessage(cl, WSErrorCode::INVALID_PARAM, err);
            return;
        }

        QString name = obj["name"].toString();
        cl->m_name = name;
        qDebug() << "got hello from client: " << name;
    } else if (msgtype == "subscribe") {

        QString err = ensureField(obj, "topic", QJsonValue::String);
        if (!err.isEmpty()) {
            handleInvalidMessage(cl, WSErrorCode::INVALID_PARAM, err);
            return;
        }

        QString topic = obj["topic"].toString();
        cl->m_subscriptions.push_back(topic);
        qDebug() << "client: " << cl->m_name << " subscribed to: " << topic;
    } else if (msgtype == "unsubscribe") {

        QString err = ensureField(obj, "topic", QJsonValue::String);
        if (!err.isEmpty()) {
            handleInvalidMessage(cl, WSErrorCode::INVALID_PARAM, err);
            return;
        }

        QString topic = obj["topic"].toString();
        auto to_erase = std::remove_if(cl->m_subscriptions.begin(), cl->m_subscriptions.end(), [&](const QString& c) {
            return c == topic;
        });

        cl->m_subscriptions.erase(to_erase, cl->m_subscriptions.end());

        qDebug() << "client: " << cl->m_name << " unsubscribed from: " << topic;
    } else {
        qDebug() << "not handling message of type: " << msgtype;
    }
}

void BaskServer::processBinaryMessage(QByteArray) {
    if (m_debug) {
        qDebug() << "Binary Message received, not handling";
    }
    return;
}

void BaskServer::socketDisconnected() {
    QWebSocket* socket = qobject_cast<QWebSocket *>(sender());

    if (m_debug) {
        qDebug() << "socketDisconnected: " << socket;
    }

    if (socket) {
        auto to_erase = std::remove_if(m_clients.begin(), m_clients.end(), [&](const BaskClient* cl){
            return cl->m_socket == socket;
        });

        m_clients.erase(to_erase, m_clients.end());
    }

    emit clientsChanged((int)m_clients.size());
}

void BaskServer::publish(const QString& msgType, const QString& msg) {

    for (auto& iter : m_clients) {
        auto i = std::find(iter->m_subscriptions.begin(), iter->m_subscriptions.end(), msgType);
        if (i == iter->m_subscriptions.end()) {
            continue;
        }

        iter->m_socket->sendTextMessage(msg);
    }
}
