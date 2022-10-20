#include "keyboardhandler.h"
#include <QDebug>

KeyboardHandler::KeyboardHandler(QObject* parent) : QObject(parent) {
    connect(&m_timer, &QTimer::timeout, this, &KeyboardHandler::trackActions);
    m_timer.setInterval(100);
    m_timer.start();
    m_event_sent.restart();
}

void KeyboardHandler::trackActions() {
#ifdef WIN32
    int numActions = 0;
    for (int i = 1; i < 255; ++i) {
        if (GetAsyncKeyState(i) & 0x100000) {
            numActions++;
        }
    }

    // throttle
    SHORT st_space = GetAsyncKeyState(VK_SPACE);
    SHORT st_ctrl = GetAsyncKeyState(VK_LCONTROL);
    SHORT st_alt = GetAsyncKeyState(VK_LMENU);

    // if keys are down
    if (st_space & 0x100000 && st_ctrl & 0x100000 && st_alt & 0x100000) {
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
