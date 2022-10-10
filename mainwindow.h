#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QRandomGenerator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

protected:
    void nextPlayer();
    void doWarning();
    void reset();
    void persist_switch_sfx();
    void persist_warning_sfx();
    QStringList getSwitchSFX();
    QStringList getWarningSFX();
private slots:
    void on_runbutton_clicked();
    void on_timer_interval();
    void on_interval_valueChanged(int arg1);
    void on_warning_time_valueChanged(int arg1);
    void on_load_switch_sfx_button_clicked();
    void on_remove_switch_sfx_button_clicked();
    void on_remove_all_switch_sfx_button_clicked();
    void on_volume_valueChanged(int value);
    void on_load_warning_sfx_button_clicked();
    void on_remove_all_warning_sfx_button_clicked();
    void on_remove_warning_sfx_button_clicked();
public slots:
    void positionChanged(qint64);
private:
    Ui::MainWindow *ui;
    bool m_running;
    QTimer* m_timer;
    int m_interval_s;
    int m_warning_time_s;
    bool m_warned;
    QElapsedTimer m_elapsed_timer;
    QMediaPlayer* m_player;
    QAudioOutput* m_audio_out;
    QRandomGenerator* m_rand;
};
#endif // MAINWINDOW_H