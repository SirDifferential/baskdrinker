#ifndef KEYBOARDHANDLER_H
#define KEYBOARDHANDLER_H

#include <QObject>
#ifdef WIN32
#include <windows.h>
#include <winuser.h>
#endif

#include <QTimer>
#include <QElapsedTimer>

class KeyboardHandler : public QObject
{
    Q_OBJECT
public:
    KeyboardHandler(QObject* parent = nullptr);
private:
    QTimer m_timer;
    QElapsedTimer m_event_sent;
#ifdef WIN32
    SHORT m_hotkey1;
    SHORT m_hotkey2;
    SHORT m_hotkey3;
#endif
public slots:
    void trackActions();
signals:
    void hotkey1Pressed();
};

#endif // KEYBOARDHANDLER_H
