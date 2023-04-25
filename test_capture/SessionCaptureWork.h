/*!
 * @file SessionCaptureWork.h
 * @author liuwanchun
 * @param  Email: liuwanchun@corp.netease.com
 * @date 2021/12/30
 * @param  Copyright(C) 2004-2021 网易互动娱乐
 * @brief
 *
 */

#ifndef SESSION_CAPTURE_WORK_H
#define SESSION_CAPTURE_WORK_H

#include <QObject>
#include <QLocalSocket>
#include <QTimer>
class SessionCaptureWorker : public QObject
{
    Q_OBJECT

public:
    enum CaptureWayEnum
    {
        CaptureWayEnum_Undefined = 0,
        CaptureWayEnum_Hotkey,
        CaptureWayEnum_ToolBar,
    };

    SessionCaptureWorker();
    ~SessionCaptureWorker();
public:
    bool IsSocketOpen();
    void BeginCapture();
    //取消截图
    void CancelCapture();
    void SetAppExit(bool exit);
    bool AppExitFlag();
    //退出截图进程
    void ExitCaptureProcess();
    void CloseSocket();
    void ExitCapture();
    void InitObject();
signals:
    void s_workerNeedtoColose();
    void s_workerColosed();
    void sBeginCapture();
protected:
    void ParserJson(QJsonDocument& jsonDocument);
private:
    void ReportCaptureSaveFile(QJsonObject& jsn_obj);
    void ReportUserTrack(QJsonObject& jsn_obj);
    void PinImage(QJsonObject& param);
    void ConnectSocket();
public slots:

    void OnBeginCapture();
 private slots:
    void OnCaptureProcessStarted();
    void OnFinishedCaptureProcess();

    void OnReadyRead();
    void OnDisconnected();
    void OnNeedToCloseWorker();

protected:
    QLocalSocket* capture_socket_ = nullptr;
    int reconnect_localserver_cnt_{ 0 };
    bool needrestart_capture_{ true };
    QTimer *heatbeat_timer_{ nullptr };
    bool app_exit_ = false;
};

#endif
