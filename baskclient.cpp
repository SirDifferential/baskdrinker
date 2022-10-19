#include "baskclient.h"

BaskClient::BaskClient(QObject* parent, QWebSocket* socket) : QObject{parent} {
    m_socket = socket;
}

BaskClient::~BaskClient() {
    m_socket->deleteLater();
}
