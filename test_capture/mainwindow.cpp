#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include"qtcapture_instance.h"
#include "SessionCaptureWork.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(QtCaptureInstance::GetInstance(), &QtCaptureInstance::SignalProcessFinished, this, &MainWindow::OnCaptureProcessFinished);
    connect(QtCaptureInstance::GetInstance(), &QtCaptureInstance::SignalProcessStarted, this, &MainWindow::OnCaptureProcessStarted);
    connect(QtCaptureInstance::GetInstance(), &QtCaptureInstance::SignalErr, this, &MainWindow::OnCaptureProcessStarted);


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_start_process_clicked()
{
    StartCaptureProcess(ui->process_path_lineedit->text());

}

void MainWindow::OnCaptureProcessFinished()
{
    ui->process_status->setText("popo_capture OnCaptureProcessFinished ");
}
void MainWindow::OnCaptureProcessStarted()
{
    ui->process_status->setText("popo_capture OnCaptureProcessStarted ");
}
void MainWindow::PrintError(QProcess::ProcessError err)
{
    ui->process_status->setText(QString("process error %1").arg(err));
}

void MainWindow::on_capture_btn_clicked()
{
    QtCaptureInstance::GetInstance()->GetWorker()->BeginCapture();

}


void MainWindow::StartCaptureProcess(QString app_path)
{
    QtCaptureInstance::GetInstance()->StartCaptureProcess(app_path);
}
