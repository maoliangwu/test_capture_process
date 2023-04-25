/*!
 * @file SessionCaptureWork.cpp
 * @author liuwanchun
 * @param  Email: xpp@corp.netease.com
 * @date 2021/12/30
 * @param  Copyright(C) 2004-2021 网易互动娱乐
 * @brief
 *
 */

#include "SessionCaptureWork.h"
#include "qtcapture_instance.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QTimer>
#include <QDebug>
#include <memory>
#include <QLocalSocket>
SessionCaptureWorker::SessionCaptureWorker()
{
    heatbeat_timer_ = new QTimer(this);
    connect(this, &SessionCaptureWorker::s_workerNeedtoColose, this, &SessionCaptureWorker::OnNeedToCloseWorker);

#ifdef DEBUG_SESSION_CONTENT_MEMORY_CHECK
    QLOG_APP("{0} -- memory check. constructor of {1}") << __FUNCTION__ << "SessionCaptureWorker";
#endif
}


SessionCaptureWorker::~SessionCaptureWorker()
{
    if (capture_socket_)
    {
        capture_socket_->close();
        delete capture_socket_;
        capture_socket_ = nullptr;
    }
#ifdef DEBUG_SESSION_CONTENT_MEMORY_CHECK
    QLOG_APP("{0} -- memory check. destruction of {1}") << __FUNCTION__ << "SessionCaptureWorker";
#endif
}

void SessionCaptureWorker::InitObject()
{
    capture_socket_ = new QLocalSocket(this);

    clock_t begin = clock();
    connect(QtCaptureInstance::GetInstance(), &QtCaptureInstance::SignalProcessStarted, this, &SessionCaptureWorker::OnCaptureProcessStarted);
    connect(QtCaptureInstance::GetInstance(), &QtCaptureInstance::SignalProcessFinished, this, &SessionCaptureWorker::OnFinishedCaptureProcess);

    connect(heatbeat_timer_, &QTimer::timeout, this, [this] {

        QJsonObject jsn_obj;
        jsn_obj["method"] = "keep";
        auto cmd = QJsonDocument(jsn_obj).toJson(QJsonDocument::Compact) + "\r\n";
        auto clock1 = clock();
        if (capture_socket_->isOpen())
        {
            capture_socket_->write(cmd.toStdString().c_str());
            capture_socket_->flush();
            capture_socket_->waitForBytesWritten();
            auto clock2 = clock();
            if (clock2 - clock1 > 250)
            {
                qDebug()<<("write failed, cost time is {0}") << clock2 - clock1;
            }
        }
        else
        {
            qDebug()<<("capture_socket_ is unable");
        }
    });
    connect(capture_socket_, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
    connect(capture_socket_, SIGNAL(disconnected()), this, SLOT(OnDisconnected()));
    connect(capture_socket_, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error), this,
        [=](QLocalSocket::LocalSocketError socketError) {if (socketError == QLocalSocket::ServerNotFoundError)
    {
        if (reconnect_localserver_cnt_ < 4)
        {
            QTimer::singleShot(500, this, [this, socketError]() {
                if (!app_exit_)
                {
                    qDebug()<<("capture_socket_ connectToServer error ") ;
                }
                ConnectSocket();
            });
            reconnect_localserver_cnt_++;
        }
    }
    });

    connect(capture_socket_, &QLocalSocket::connected, this, [this]() {
        needrestart_capture_ = true;
        qDebug()<<("capture_socket_ connected");
        //发心跳包
        heatbeat_timer_->start(10 * 1000);
    });
    clock_t end1 = clock();


    if (end1 - begin > 50)
    {
        qDebug()<<QString("SessionCaptureWorker time= %1").arg(end1 - begin);
    }
}

void SessionCaptureWorker::ConnectSocket()
{
    qDebug()<<"1111111111111";
    if (QtCaptureInstance::GetInstance()->CaptureProcessInRunning() && capture_socket_)
    {
#if defined (Q_OS_WIN)
        DWORD pid = GetCurrentProcessId();
#elif defined (Q_OS_LINUX)
        pid_t pid;
        pid = 333;
#elif defined (Q_OS_MACOS)
        pid_t pid;
        pid = getpid();
#endif
        QString servername = "capture_server" + QString("%1").arg(pid);
        qDebug()<<QString("capture_socket_ try to connect servername %1").arg(servername);
        capture_socket_->connectToServer(servername);
    }
}

void SessionCaptureWorker::OnCaptureProcessStarted()
{
    needrestart_capture_ = false;
    reconnect_localserver_cnt_ = 0;
    QTimer::singleShot(1000, this, [this]() {
        ConnectSocket();
    });
}

void SessionCaptureWorker::OnFinishedCaptureProcess()
{
    capture_socket_->close();
}

void SessionCaptureWorker::OnReadyRead()
{
    QString recevice;

    // 处理来自客户端的数据
    if (capture_socket_)
    {
        QTextStream stream(capture_socket_);
        stream.setCodec("UTF-8");
        while (!stream.atEnd())
        {
            QString respond = stream.readLine();
            QJsonDocument jsonDocument = QJsonDocument::fromJson(respond.toUtf8());
            if (!jsonDocument.isNull())
            {
                ParserJson(jsonDocument);
            }
        }

    }
}

void SessionCaptureWorker::ParserJson(QJsonDocument& jsonDocument)
{
    QJsonObject jsn_obj = jsonDocument.object();
    QJsonObject param = jsn_obj["param"].toObject();
}

void SessionCaptureWorker::ReportUserTrack(QJsonObject& jsn_obj)
{}

void SessionCaptureWorker::ReportCaptureSaveFile(QJsonObject& jsn_obj)
{
}

void SessionCaptureWorker::OnNeedToCloseWorker()
{
    CancelCapture();
    ExitCaptureProcess();
    emit s_workerColosed();
}

void SessionCaptureWorker::OnDisconnected()
{
    if (app_exit_)
    {
        return;
    }
    qCritical()<<("capture_socket_ OnDisconnected ");
}

bool SessionCaptureWorker::AppExitFlag()
{
    return app_exit_;
}

void SessionCaptureWorker::SetAppExit(bool exit)
{
    if (exit)
    {
        qDebug()<<("SessionCaptureWorker popo app exit ");
    }
    app_exit_ = exit;
}

void SessionCaptureWorker::CancelCapture()
{
}

void SessionCaptureWorker::ExitCaptureProcess()
{
    if (heatbeat_timer_)
    {
        delete heatbeat_timer_;
        heatbeat_timer_ = nullptr;
    }
    if (capture_socket_)
    {
        auto begin = clock();
        QJsonObject jsn_obj;
        jsn_obj["method"] = "kill";
        jsn_obj["param"] = "";
        auto cmd = QString(QJsonDocument(jsn_obj).toJson(QJsonDocument::Compact)) + "\r\n";
        capture_socket_->write(cmd.toStdString().c_str());
        capture_socket_->flush();
        capture_socket_->waitForBytesWritten();
        auto end = clock();
        if (end - begin > 200)
        {
            qDebug()<<QString("write exit, cost time is %1") .arg(end - begin);
        }
    }
}

void SessionCaptureWorker::OnBeginCapture()
{
    if (capture_socket_)
    {
        auto str = "{\"method\":\"capture\",\"param\":{\"cappath\":\"/home/netease/.config/Netease/Popo/users/xupeipei@corp.netease.com/mycapture//\",\"capsavepath\":\"/home/netease/.config/Netease/Popo/users/xupeipei@corp.netease.com/mycapture//\",\"capsessiontype\":\"1\",\"capsid\":\"wb.zhuleyi@mesg.corp.netease.com\",\"captype\":\".jpg\"}}";
        capture_socket_->write(str);
        capture_socket_->flush();
        capture_socket_->waitForBytesWritten();
    }
}
void SessionCaptureWorker::BeginCapture()
{
    emit sBeginCapture();
}

bool SessionCaptureWorker::IsSocketOpen()
{
    if (capture_socket_)
    {
        return capture_socket_->isOpen();
    }
    return false;
}

void SessionCaptureWorker::CloseSocket()
{
    if (capture_socket_)
    {
        capture_socket_->close();
    }
}
