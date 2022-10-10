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
    m_running = false;
    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::on_timer_interval);
    m_interval_s = ui->interval->value();
    m_warning_time_s = ui->warning_time->value();
    m_warned = false;

    QSettings settings;
    QStringList sfx_switch_list = settings.value("switch_sfx_list").toStringList();

    for (auto& s : sfx_switch_list) {
        QFileInfo finfo(s);
        if (finfo.exists()) {
            QListWidgetItem* it = new QListWidgetItem(ui->switch_sfx_list);
            it->setText(s);
        }
    }

    QStringList warning_sfx_list = settings.value("warning_sfx_list").toStringList();

    for (auto& s : warning_sfx_list) {
        QFileInfo finfo(s);
        if (finfo.exists()) {
            QListWidgetItem* it = new QListWidgetItem(ui->warning_sfx_list);
            it->setText(s);
        }
    }

    m_player = new QMediaPlayer();
    m_audio_out = new QAudioOutput();
    m_player->setAudioOutput(m_audio_out);
    connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    m_audio_out->setVolume(50);
    int vol = settings.value("volume", 50).toInt();
    ui->volume->setValue(vol);
    m_audio_out->setVolume(vol / 100.0f);

    m_rand = QRandomGenerator::system();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::positionChanged(qint64 pos) {
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
        m_timer->stop();
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

        m_player->setSource(QUrl::fromLocalFile(sfx));
        m_player->play();
    }
}

void MainWindow::doWarning() {

    QStringList sfx_list = getWarningSFX();
    qDebug() << sfx_list.count();

    if (sfx_list.count() > 0) {
        QString sfx = sfx_list[0];

        if (sfx_list.count() > 1) {
            quint32 v = m_rand->bounded(0, sfx_list.count());
            qDebug() << "got: " << v;
            sfx = sfx_list[v];
        }

        m_player->setSource(QUrl::fromLocalFile(sfx));
        m_player->play();
    }

    m_warned = true;
}

void MainWindow::on_timer_interval() {
    int64_t secs_since = m_elapsed_timer.nsecsElapsed() / 1000000000LL;
    int64_t t = m_interval_s - secs_since;

    if (t <= m_warning_time_s && !m_warned && m_warning_time_s > 0) {
        doWarning();
    }

    if (t <= 0) {
        nextPlayer();
        t = m_interval_s - secs_since;
    }

    QString l = "Next turn in: " + sensibleUnits(t);
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

void MainWindow::on_volume_valueChanged(int value)
{
    QSettings settings;
    settings.setValue("volume", value);
    m_audio_out->setVolume(value / 100.0f);
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

    QStringList items = getSwitchSFX();

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

