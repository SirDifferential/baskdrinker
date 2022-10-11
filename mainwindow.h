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
    void on_load_warning_sfx_button_clicked();
    void on_remove_all_warning_sfx_button_clicked();
    void on_remove_warning_sfx_button_clicked();
    void on_repeat_warning_toggled(bool checked);
    void on_switch_volume_valueChanged(int value);
    void on_warning_volume_valueChanged(int value);
public slots:
    void positionChangedSwitch(qint64);
    void positionChangedWarning(qint64);
private:
    Ui::MainWindow *ui;
    bool m_running;
    QTimer* m_timer;
    int m_interval_s;
    int m_warning_time_s;
    bool m_warned;
    bool m_repeat_warning;
    int m_volume_switch;
    int m_volume_warning;
    QElapsedTimer m_elapsed_timer;
    QMediaPlayer* m_player_switch;
    QMediaPlayer* m_player_warning;
    QAudioOutput* m_audio_switch;
    QAudioOutput* m_audio_warning;
    QRandomGenerator* m_rand;
};
#endif // MAINWINDOW_H
