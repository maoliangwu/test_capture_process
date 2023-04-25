#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_start_process_clicked();

    void on_capture_btn_clicked();
    void OnCaptureProcessStarted();
    void OnCaptureProcessFinished();
    void PrintError(QProcess::ProcessError);

private:
    void StartCaptureProcess(QString app_path);

private:
    Ui::MainWindow *ui;
    QProcess * capture_process_{nullptr};
};
#endif // MAINWINDOW_H
