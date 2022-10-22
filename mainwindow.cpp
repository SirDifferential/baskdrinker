#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QListWidget>
#include <QAbstractItemView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString app_title = QString("Loathsome Bäsk Drinker v") + QString(BASK_VERSION);
    setWindowTitle(app_title);
    m_running = false;
    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerInterval);
    m_interval_s = ui->interval->value();
    m_warning_time_s = ui->warning_time->value();
    m_warned = false;
    m_server = new BaskServer(this);
    connect(m_server, &BaskServer::clientsChanged, this, &MainWindow::onWSClientsChanged);
    connect(m_server, &BaskServer::stateChanged, this, &MainWindow::onWSStateChanged);

    QSettings settings;
    QStringList sfx_switch_list = settings.value("switch_sfx_list").toStringList();

    for (auto& s : sfx_switch_list) {
        QFileInfo finfo(s);
        if (finfo.exists()) {
            QListWidgetItem* it = new QListWidgetItem(ui->switch_sfx_list);
            it->setText(s);
        }
    }

    QStringList warning_sfx_list = settings.value("warning_sfx_list", QStringList()).toStringList();

    for (auto& s : warning_sfx_list) {
        QFileInfo finfo(s);
        if (finfo.exists()) {
            QListWidgetItem* it = new QListWidgetItem(ui->warning_sfx_list);
            it->setText(s);
        }
    }

    m_player_switch = new QMediaPlayer();
    m_player_warning = new QMediaPlayer();
    m_player_hotkey = new QMediaPlayer();
    m_audio_switch = new QAudioOutput();
    m_audio_warning = new QAudioOutput();
    m_audio_hotkey = new QAudioOutput();

    m_player_switch->setAudioOutput(m_audio_switch);
    m_player_warning->setAudioOutput(m_audio_warning);
    m_player_hotkey->setAudioOutput(m_audio_hotkey);

    connect(m_player_switch, SIGNAL(positionChanged(qint64)), this, SLOT(positionChangedSwitch(qint64)));
    connect(m_player_warning, SIGNAL(positionChanged(qint64)), this, SLOT(positionChangedWarning(qint64)));

    m_audio_switch->setVolume(m_volume_switch / 100.0f);
    m_audio_warning->setVolume(m_volume_warning / 100.0f);

    m_rand = QRandomGenerator::system();
    m_repeat_warning = settings.value("repeat_warning", false).toBool();
    m_player_warning->setLoops(m_repeat_warning ? QMediaPlayer::Infinite : 1);

    ui->repeat_warning->setChecked(m_repeat_warning);
    m_volume_switch = settings.value("volume_switch", 50).toInt();
    m_volume_warning =settings.value("volume_warning", 50).toInt();
    ui->switch_volume->setValue(m_volume_switch);
    ui->warning_volume->setValue(m_volume_warning);

    int port = settings.value("websocket_port", 10666).toInt();
    ui->websocket_port->setValue(port);
    m_server->setPort(port);

    m_kb = new KeyboardHandler(this);
    connect(m_kb, &KeyboardHandler::hotkey1Pressed, this, &MainWindow::handleHotkey1);

    hotkey1_on_sfx = settings.value("hotkey1_on_sfx", "").toString();
    hotkey1_off_sfx = settings.value("hotkey1_off_sfx", "").toString();
}

MainWindow::~MainWindow()
{
    delete ui;
    ui = nullptr;
}

void MainWindow::positionChangedSwitch(qint64 pos) {
}

void MainWindow::positionChangedWarning(qint64 pos) {
}

void MainWindow::on_runbutton_clicked()
{
    if (!m_running) {
        m_running = true;
        m_elapsed_timer.restart();
        m_timer->start();
        ui->runbutton->setText("STOP");
    } else {
        m_running = false;
        ui->runbutton->setText("START");
        ui->timer_l->setText("");
        m_timer->stop();
        m_player_warning->stop();
        m_player_switch->stop();
        m_warned = false;
    }
}

QString sensibleUnits(int64_t t) {
    if (t >= 60LL * 60LL) {
        return QString::number(t / 60LL / 60LL) + " hours";
    } else if (t >= 120LL) {
        return QString::number(t / 60LL) + " minutes";
    } else if (t <= 120LL) {
        return QString::number(t) + " seconds";
    }

    return QString::number(t) + " seconds";
}

void MainWindow::reset() {
    m_elapsed_timer.restart();
    m_warned = false;
}

QStringList MainWindow::getSwitchSFX() {
    QStringList items;
    for (int i = 0; i < ui->switch_sfx_list->count(); ++i) {
        items.push_back(ui->switch_sfx_list->item(i)->text());
    }
    return items;
}

QStringList MainWindow::getWarningSFX() {
    QStringList items;
    for (int i = 0; i < ui->warning_sfx_list->count(); ++i) {
        items.push_back(ui->warning_sfx_list->item(i)->text());
    }
    return items;
}

void MainWindow::nextPlayer() {
    reset();
    QStringList sfx_list = getSwitchSFX();

    if (sfx_list.count() > 0) {
        QString sfx = sfx_list[0];

        if (sfx_list.count() > 1) {
            quint32 v = m_rand->bounded(0, sfx_list.count());
            sfx = sfx_list[v];
        }

        m_player_switch->setSource(QUrl::fromLocalFile(sfx));
        m_player_switch->play();
    }

    QJsonObject obj;
    obj["msgType"] = "nextPlayer";
    QJsonDocument doc(obj);
    QString strJson = doc.toJson(QJsonDocument::Compact);
    m_server->publish("nextPlayer", strJson);
}

void MainWindow::doWarning(int seconds) {

    QStringList sfx_list = getWarningSFX();

    if (sfx_list.count() > 0) {
        QString sfx = sfx_list[0];

        if (sfx_list.count() > 1) {
            quint32 v = m_rand->bounded(0, sfx_list.count());
            sfx = sfx_list[v];
        }

        m_player_warning->setSource(QUrl::fromLocalFile(sfx));
        m_player_warning->play();
    }

    m_warned = true;

    QJsonObject obj;
    obj["msgType"] = "warning";
    obj["timeUntilNextPlayerSeconds"] = seconds;
    QJsonDocument doc(obj);
    QString strJson = doc.toJson(QJsonDocument::Compact);
    m_server->publish("warning", strJson);
}

void MainWindow::onTimerInterval() {
    int64_t secs_since = m_elapsed_timer.nsecsElapsed() / 1000000000LL;
    int64_t t = m_interval_s - secs_since;

    bool soon = false;
    if (t <= m_warning_time_s && m_warning_time_s > 0) {
        soon = true;
        if (!m_warned) {
            doWarning(t);
        }
    }

    if (t <= 0) {
        m_player_warning->stop();
        nextPlayer();
        t = m_interval_s - secs_since;
    }

    QString l;
    if (soon) {
        l += "<b>";
    }

    l += "Next turn in: " + sensibleUnits(t);

    if (soon) {
        l += "</b>";
    }

    ui->timer_l->setText(l);
}

void MainWindow::on_interval_valueChanged(int val)
{
    m_interval_s = val;
    reset();
}

void MainWindow::on_warning_time_valueChanged(int val)
{
    m_warning_time_s = val;
    // Play warnings even if they have already played if we adjust the warning time
    m_warned = false;
}

void MainWindow::on_load_switch_sfx_button_clicked()
{
    QSettings settings;

    QFileDialog d(this);
    d.setFileMode(QFileDialog::ExistingFiles);
    d.setNameFilter(tr("Audio files (*.wav *.mp3 *.ogg *.flac)"));
    d.setViewMode(QFileDialog::Detail);

    QString p = settings.value("switch_sfx_dir", QDir::homePath()).toString();

    d.setDirectory(p);
    QStringList files;

    QStringList items = getSwitchSFX();

    if (d.exec()) {

        QString lastDir;

        files = d.selectedFiles();

        for (auto& f : files) {

            if (items.contains(f)) {
                continue;
            }

            lastDir = QFileInfo(f).absoluteDir().absolutePath();
            ui->switch_sfx_list->addItem(f);
        }

        settings.setValue("switch_sfx_dir", lastDir);
    }

    persist_switch_sfx();
}

void MainWindow::persist_switch_sfx() {
    QStringList items = getSwitchSFX();
    QSettings settings;
    settings.setValue("switch_sfx_list", items);
}

void MainWindow::persist_warning_sfx() {
    QStringList items = getWarningSFX();
    QSettings settings;
    settings.setValue("warning_sfx_list", items);
}

void MainWindow::on_remove_switch_sfx_button_clicked()
{
    QList<QListWidgetItem*> items = ui->switch_sfx_list->selectedItems();
    for (auto& w : items) {
        delete ui->switch_sfx_list->takeItem(ui->switch_sfx_list->row(w));
    }

    persist_switch_sfx();
}

void MainWindow::on_remove_all_switch_sfx_button_clicked()
{
    QList<QListWidgetItem*> items;
    for (int i = 0; i < ui->switch_sfx_list->count(); ++i) {
        items.push_back(ui->switch_sfx_list->item(i));
    }

    for (auto& w : items) {
        delete ui->switch_sfx_list->takeItem(ui->switch_sfx_list->row(w));
    }

    persist_switch_sfx();
}

void MainWindow::on_load_warning_sfx_button_clicked()
{
    QSettings settings;

    QFileDialog d(this);
    d.setFileMode(QFileDialog::ExistingFiles);
    d.setNameFilter(tr("Audio files (*.wav *.mp3 *.ogg *.flac)"));
    d.setViewMode(QFileDialog::Detail);

    QString p = settings.value("warning_sfx_dir", QDir::homePath()).toString();

    d.setDirectory(p);
    QStringList files;

    QStringList items = getWarningSFX();

    if (d.exec()) {

        QString lastDir;

        files = d.selectedFiles();

        for (auto& f : files) {

            if (items.contains(f)) {
                continue;
            }

            lastDir = QFileInfo(f).absoluteDir().absolutePath();
            ui->warning_sfx_list->addItem(f);
        }

        settings.setValue("warning_sfx_dir", lastDir);
    }

    persist_warning_sfx();
}

void MainWindow::on_remove_all_warning_sfx_button_clicked()
{
    QList<QListWidgetItem*> items;
    for (int i = 0; i < ui->warning_sfx_list->count(); ++i) {
        items.push_back(ui->warning_sfx_list->item(i));
    }

    for (auto& w : items) {
        delete ui->warning_sfx_list->takeItem(ui->warning_sfx_list->row(w));
    }

    persist_warning_sfx();
}

void MainWindow::on_remove_warning_sfx_button_clicked()
{
    QList<QListWidgetItem*> items = ui->warning_sfx_list->selectedItems();
    for (auto& w : items) {
        delete ui->warning_sfx_list->takeItem(ui->warning_sfx_list->row(w));
    }

    persist_warning_sfx();
}

void MainWindow::on_repeat_warning_toggled(bool checked)
{
    m_repeat_warning = checked;
    QSettings settings;
    settings.setValue("repeat_warning", checked);
    m_player_warning->setLoops(m_repeat_warning ? QMediaPlayer::Infinite : 1);
}


void MainWindow::on_switch_volume_valueChanged(int value)
{
    QSettings settings;
    settings.setValue("volume_switch", value);
    m_audio_switch->setVolume(value / 100.0f);
}

void MainWindow::on_warning_volume_valueChanged(int value)
{
    QSettings settings;
    settings.setValue("volume_warning", value);
    m_audio_warning->setVolume(value / 100.0f);
}

void MainWindow::on_websocket_port_valueChanged(int arg)
{
    QSettings settings;
    settings.setValue("websocket_port", arg);
    m_server->setPort(arg);
}

void MainWindow::on_start_ws_server_clicked()
{
    if (m_server->isRunning()) {
        m_server->stop();
    } else {
        if (!m_server->init()) {
            return;
        }
    }
}

void MainWindow::onWSClientsChanged(int arg) {
    if (!ui) {
        return;
    }
    ui->ws_clients_label->setText("Clients: " + QString::number(arg));
}

void MainWindow::onWSStateChanged(bool arg, QString details) {

    if (!ui) {
        return;
    }

    if (arg) {
        ui->start_ws_server->setText("Stop server");
    } else {
        ui->start_ws_server->setText("Start server");
    }
    ui->ws_server_state_l->setText(details);
}

void MainWindow::handleHotkey1() {
    on_runbutton_clicked();

    if (m_running && !hotkey1_on_sfx.isEmpty()) {
        m_player_hotkey->setSource(QUrl::fromLocalFile(hotkey1_on_sfx));
        m_player_hotkey->play();
    } else if (!m_running && !hotkey1_off_sfx.isEmpty()) {
        m_player_hotkey->setSource(QUrl::fromLocalFile(hotkey1_off_sfx));
        m_player_hotkey->play();
    }
}

QString MainWindow::getHotkeySfx() {

    QSettings settings;
    QFileDialog d(this);

    d.setFileMode(QFileDialog::ExistingFile);
    d.setNameFilter(tr("Audio files (*.wav *.mp3 *.ogg *.flac)"));
    d.setViewMode(QFileDialog::Detail);

    QString p = settings.value("last_hotkey_sfx_dir", QDir::homePath()).toString();

    d.setDirectory(p);
    QStringList files;

    QStringList items = getWarningSFX();

    QString sfx;
    if (d.exec()) {

        QString lastDir;

        files = d.selectedFiles();
        if (files.size() > 1) {
            qDebug() << "more than 1 sfx picked for hotkey 1 on";
            return sfx;
        }

        sfx = files[0];
        lastDir = QFileInfo(sfx).absoluteDir().absolutePath();
        settings.setValue("last_hotkey_sfx_dir", lastDir);
    }

    return sfx;
}

void MainWindow::on_hotkey1_fsx_on_button_clicked()
{
    QString sfx = getHotkeySfx();
    QSettings settings;
    settings.setValue("hotkey1_on_sfx", sfx);
    hotkey1_on_sfx = sfx;
}


void MainWindow::on_hotkey1_fsx_off_button_clicked()
{
    QString sfx = getHotkeySfx();
    QSettings settings;
    settings.setValue("hotkey1_off_sfx", sfx);
    hotkey1_off_sfx = sfx;
}
