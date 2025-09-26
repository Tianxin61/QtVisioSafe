#include "videoserver.h"
#include <QDataStream>
#include <QDateTime>
#include <QThread>

VideoServer::VideoServer(QObject* parent) 
    : QTcpServer(parent), m_frameCount(0)
{
    m_frameTimer = new QTimer(this);
    connect(m_frameTimer, &QTimer::timeout, this, &VideoServer::generateVideoFrame);
}

VideoServer::~VideoServer()
{
    stopServer();
}

bool VideoServer::startServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        qDebug() << "Server could not start:" << errorString();
        return false;
    }
    
    m_frameTimer->start(33); // 30 FPS
    qDebug() << "Server started on port" << port;
    return true;
}

void VideoServer::stopServer()
{
    m_frameTimer->stop();
    
    for (auto client : m_clients) {
        client->disconnectFromHost();
        if (client->state() != QAbstractSocket::UnconnectedState) {
            client->waitForDisconnected(1000);
        }
    }
    
    close();
    qDebug() << "Server stopped";
}

QMap<QString, QString> VideoServer::getDeviceProperties() const
{
    return {
        {"device_name", "模拟摄像头"},
        {"resolution", "1280x720"},
        {"frame_rate", "30 FPS"},
        {"color_format", "RGB32"},
        {"status", "在线"}
    };
}

void VideoServer::onNewConnection()
{
    QTcpSocket* clientSocket = nextPendingConnection();
    m_clients.append(clientSocket);
    
    connect(clientSocket, &QTcpSocket::disconnected, this, &VideoServer::onClientDisconnected);
    connect(clientSocket, &QTcpSocket::readyRead, this, &VideoServer::onReadyRead);
    
    qDebug() << "Client connected:" << clientSocket->peerAddress().toString();
    
    // 发送设备属性给客户端
    QMap<QString, QString> properties = getDeviceProperties();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << QString("PROPERTIES") << properties;
    clientSocket->write(data);
}

void VideoServer::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        m_clients.removeAll(client);
        client->deleteLater();
        qDebug() << "Client disconnected";
    }
}

void VideoServer::onReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    QDataStream stream(client);
    QString command;
    stream >> command;
    
    if (command == "REQUEST_FRAME") {
        if (m_frameBuffer.queueSize() > 0) {
            QImage frame = m_frameBuffer.dequeueFrame();
            sendFrameToClient(client, frame);
        }
    }
}

void VideoServer::generateVideoFrame()
{
    // 生成模拟视频帧
    QImage frame(1280, 720, QImage::Format_RGB32);
    frame.fill(Qt::black);
    
    // 在帧上绘制一些动态内容
    QPainter painter(&frame);
    painter.setPen(QPen(Qt::green, 3));
    painter.drawText(frame.rect(), Qt::AlignCenter, 
                    QString("模拟视频帧 #%1\n时间: %2\n客户端数量: %3")
                    .arg(m_frameCount++)
                    .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                    .arg(m_clients.size()));
    
    painter.drawEllipse(600 + 200 * qSin(m_frameCount * 0.1), 
                       300 + 150 * qCos(m_frameCount * 0.1), 50, 50);
    
    // 帧数据入队
    m_frameBuffer.enqueueFrame(frame);
    
    // 主动发送给所有连接的客户端（可选：也可根据客户端请求发送）
    for (auto client : m_clients) {
        sendFrameToClient(client, frame);
    }
}

void VideoServer::sendFrameToClient(QTcpSocket* client, const QImage& frame)
{
    if (client->state() == QAbstractSocket::ConnectedState) {
        QByteArray frameData;
        QDataStream stream(&frameData, QIODevice::WriteOnly);
        stream << QString("VIDEO_FRAME") << frame;
        
        // 简单的数据包头部（长度信息）
        QByteArray packet;
        QDataStream packetStream(&packet, QIODevice::WriteOnly);
        packetStream << quint32(frameData.size());
        packet.append(frameData);
        
        client->write(packet);
    }
}
