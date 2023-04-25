#ifndef QT_CAPTURE_INSTANCE
#define QT_CAPTURE_INSTANCE

#include <QProcess>
#include <QObject>
#include <QMutex>
#include <QThread>
class SessionCaptureWorker;
class QLocalSocket;

class QtCaptureInstance :
    public QObject
{
    Q_OBJECT
public:
    static QtCaptureInstance *GetInstance();
    static void ReleaseInstance();

    ~QtCaptureInstance();
    void							SetCapturing(bool iscapturing);
    bool							IsCapturing();
    bool                            CaptureProcessInRunning();
    //开启截图进程
    void							StartCaptureProcess(QString app_path);
    //退出截图进程
    void                            EndCaptureProcess();
    SessionCaptureWorker * GetWorker();
private:
    explicit QtCaptureInstance(QObject *parent = nullptr);
signals:
    void SignalProcessStarted();
    void SignalProcessFinished();
    void SignalErr(QProcess::ProcessError);
    void DoWork();

public slots:
    void							OnCaptureProcessFinished(int, QProcess::ExitStatus);
    void							OnProcessError(QProcess::ProcessError);
    void							OnCaptureProcessStarted();
    void							CleanupCatrueFiles();
    void                            OnStateChanged(QProcess::ProcessState state, QPrivateSignal);
protected:
    void InitCaptureWorker();
private:
    QProcess* capture_process_{nullptr};
    QMutex mutex_;
    bool incapture_status_{ false };
    bool need_rebuild_process_{ false };

private:
    //以下用于替换会话区的多个截图对象
    SessionCaptureWorker *worker_=nullptr;
    QThread *worker_thread_ = nullptr;
    int auto_restart_cnt_{ 0 };
    QString app_path_;
};

#endif
