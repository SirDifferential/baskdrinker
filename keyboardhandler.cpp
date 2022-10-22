#include "keyboardhandler.h"
#include <QDebug>
#include <QSettings>

KeyboardHandler::KeyboardHandler(QObject* parent) : QObject(parent) {
    connect(&m_timer, &QTimer::timeout, this, &KeyboardHandler::trackActions);
    m_timer.setInterval(100);
    m_timer.start();
    m_event_sent.restart();

#ifdef WIN32
    QSettings settings;
    bool ok;

    // Read hotkey from settings
    m_hotkey1 = settings.value("hotkey1", VK_SPACE).toInt(&ok);
    if (!ok) {
        // Handle error if value isn't an integer
        qDebug() << "Failed parsing hotkey 1 into int";
        m_hotkey1 = VK_SPACE;
    } else {
        // persist the value back - this is needed if no setting existed yet but we
        // want the user to have an easier time configuring the value
        settings.setValue("hotkey1", m_hotkey1);
    }

    ok = true;
    m_hotkey2 = settings.value("hotkey2", VK_LCONTROL).toInt(&ok);
    if (!ok) {
        qDebug() << "Failed parsing hotkey 2 into int";
        m_hotkey2 = VK_LCONTROL;
    } else {
        settings.setValue("hotkey2", m_hotkey2);
    }

    ok = true;
    m_hotkey3 = settings.value("hotkey3", VK_LMENU).toInt(&ok);
    if (!ok) {
        qDebug() << "Failed parsing hotkey 3 into int";
        m_hotkey3 = VK_LMENU;
    } else {
        settings.setValue("hotkey3", m_hotkey3);
    }
#endif
}

void KeyboardHandler::trackActions() {
#ifdef WIN32
    int numActions = 0;
    for (int i = 1; i < 255; ++i) {
        if (GetAsyncKeyState(i) & 0x100000) {
            numActions++;
        }
    }

    SHORT st_hk1 = GetAsyncKeyState(m_hotkey1);
    SHORT st_hk2 = GetAsyncKeyState(m_hotkey2);
    SHORT st_hk3 = GetAsyncKeyState(m_hotkey3);

    // if keys are down
    if (st_hk1 & 0x100000 && st_hk2 & 0x100000 && st_hk3 & 0x100000) {
        long long ms_since = m_event_sent.nsecsElapsed() / 1000000LL;
        if (ms_since < 600) {
            // Don't send new hotkey press until x ms has passed.
            // Better way to do is is to detect when user lifts the button
            // but effort.
            return;
        }

        m_event_sent.restart();
        emit hotkey1Pressed();
    }
#endif
}
