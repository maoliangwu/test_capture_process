#include "qtcapture_instance.h"
#include "SessionCaptureWork.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <future>
#include <QTimer>
#if defined (Q_OS_LINUX)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif
QtCaptureInstance::QtCaptureInstance(QObject* parent):QObject(parent)
{
    capture_process_ = new QProcess(this);
    if (capture_process_)
    {
        connect(capture_process_, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &QtCaptureInstance::OnCaptureProcessFinished);
        connect(capture_process_, SIGNAL(error(QProcess::ProcessError)), this, SLOT(OnProcessError(QProcess::ProcessError)));
        connect(capture_process_, &QProcess::started, this, &QtCaptureInstance::OnCaptureProcessStarted);
    }
    InitCaptureWorker();
}
void QtCaptureInstance::InitCaptureWorker()
{
    worker_thread_ = new QThread();
    worker_ = new SessionCaptureWorker();
    worker_->moveToThread(worker_thread_);
    connect(worker_, &SessionCaptureWorker::sBeginCapture, worker_, &SessionCaptureWorker::OnBeginCapture);
    connect(worker_thread_, &QThread::started, this, [this](){
            worker_->InitObject();}, Qt::DirectConnection);
    worker_thread_->start();
}

SessionCaptureWorker * QtCaptureInstance::GetWorker()
{
    return worker_;
}
static QtCaptureInstance *s_instance = nullptr;

QtCaptureInstance* QtCaptureInstance::GetInstance()
{
    if(s_instance == nullptr)
    {
        s_instance = new QtCaptureInstance();
    }
    return s_instance;
}
void QtCaptureInstance::ReleaseInstance()
{
    if(s_instance)
    {
        delete  s_instance;
        s_instance = nullptr;
    }
}

QtCaptureInstance::~QtCaptureInstance()
{
    if (worker_thread_)
    {
        worker_thread_->quit();
        worker_thread_->wait();
        delete worker_thread_;
    }
    if (worker_)
    {
        delete worker_;
        worker_ = nullptr;
    }
    if(capture_process_)
    {
        capture_process_->terminate();
        delete capture_process_;
        capture_process_ = nullptr;
    }
}

void QtCaptureInstance::EndCaptureProcess()
{
    worker_->SetAppExit(true);
    emit worker_->s_workerNeedtoColose();
}

void QtCaptureInstance::StartCaptureProcess(QString app_path)
{
    app_path_ = app_path;
    if (1)
    {
        if (capture_process_)
        {
            capture_process_->disconnect();
            delete capture_process_;
        }
        capture_process_ = new QProcess(this);
        if (capture_process_)
        {
            connect(capture_process_, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &QtCaptureInstance::OnCaptureProcessFinished);
            connect(capture_process_, SIGNAL(error(QProcess::ProcessError)), this, SLOT(OnProcessError(QProcess::ProcessError)));
            connect(capture_process_, &QProcess::started, this, &QtCaptureInstance::OnCaptureProcessStarted);
        }
    }
    if (capture_process_ && capture_process_->state() == QProcess::NotRunning)
    {
        unsigned int process_id = 0;
        std::string data_path = "/home/netease/.config/Netease/Popo/log/";
#if defined (Q_OS_WIN)
        QString program = (platform_common::GetAppPath() + "popo_capture.exe").c_str();
        process_id = GetCurrentProcessId();
#elif defined (Q_OS_LINUX)
        QString program = app_path;
        process_id = 333;
#elif defined (Q_OS_MACOS)
#ifdef _DEBUG
        auto apppath = platform_common::GetAppPath();
        QString program = QString::fromStdString(apppath)+ "../POPOCapture/popo_capture	";
#else
        QString program = QString::fromStdString(platform_common::GetAppPath())+ "../../../../popo_capture";
#endif
        process_id = getpid();
#endif
        int language_type = 1;
        QStringList list;
        list << QString("%1").arg(process_id)
             << QString("%1").arg(language_type)
            << QString("%1").arg(data_path.c_str());
        capture_process_->start(program, list);
        qDebug()<<"aaaa "<<program<< list.join(",");
    }
}

bool QtCaptureInstance::CaptureProcessInRunning()
{
    return capture_process_ ? capture_process_->state() == QProcess::Running : false;
}

void QtCaptureInstance::OnProcessError(QProcess::ProcessError error)
{
    if (error == QProcess::ProcessError::FailedToStart)
    {
        need_rebuild_process_ = true;
    }
    emit SignalErr(error);
}

void QtCaptureInstance::OnStateChanged(QProcess::ProcessState state, QPrivateSignal)
{
    qDebug()<<QString("capture process OnStateChanged : %1").arg((int)state);
}

void QtCaptureInstance::OnCaptureProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (worker_)
    {
        if (!worker_->AppExitFlag())
        {
            if (auto_restart_cnt_ < 10)//最多允许自动重启恢复5次，之后就不再自动恢复，因为有时候是这个截图已经有严重问题了，怕影响主程序
            {
                StartCaptureProcess(app_path_);
                auto_restart_cnt_++;
            }
        }
    }
    emit SignalProcessFinished();
}

bool QtCaptureInstance::IsCapturing()
{
    return incapture_status_;
}

void QtCaptureInstance::OnCaptureProcessStarted()
{
    emit SignalProcessStarted();
}

void QtCaptureInstance::SetCapturing(bool iscapturing)
{
    incapture_status_ = iscapturing;
}

void QtCaptureInstance::CleanupCatrueFiles()
{
}
