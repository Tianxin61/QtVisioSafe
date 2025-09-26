#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include "framebuffer.h"

class VideoServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit VideoServer(QObject* parent = nullptr);
    ~VideoServer();

    bool startServer(quint16 port);
    void stopServer();

    // 模拟获取设备属性
    QMap<QString, QString> getDeviceProperties() const;

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();
    void generateVideoFrame();  // 模拟视频帧生成

private:
    void sendFrameToClient(QTcpSocket* client, const QImage& frame);

    QList<QTcpSocket*> m_clients;
    FrameBuffer m_frameBuffer;
    QTimer* m_frameTimer;
    int m_frameCount;
};

#endif // VIDEOSERVER_H
