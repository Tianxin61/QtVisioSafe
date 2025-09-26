#include "clientwindow.h"
#include <QDataStream>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

ClientWindow::ClientWindow(QWidget *parent) 
    : QMainWindow(parent), m_socket(nullptr), m_packetSize(0), m_receivingPacket(false)
{
    setupUI();
    updateStatus("未连接");
}

ClientWindow::~ClientWindow()
{
    disconnectFromServer();
}

void ClientWindow::setupUI()
{
    setWindowTitle("QT视频监控客户端");
    setMinimumSize(800, 600);
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 视频显示区域
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("border: 2px solid gray; background-color: black;");
    m_videoLabel->setMinimumSize(640, 480);
    mainLayout->addWidget(m_videoLabel);
    
    // 状态显示
    m_statusLabel = new QLabel(this);
    mainLayout->addWidget(m_statusLabel);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_connectButton = new QPushButton("连接服务器", this);
    m_disconnectButton = new QPushButton("断开连接", this);
    m_captureButton = new QPushButton("抓拍", this);
    
    m_disconnectButton->setEnabled(false);
    m_captureButton->setEnabled(false);
    
    buttonLayout->addWidget(m_connectButton);
    buttonLayout->addWidget(m_disconnectButton);
    buttonLayout->addWidget(m_captureButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(m_connectButton, &QPushButton::clicked, this, &ClientWindow::connectToServer);
    connect(m_disconnectButton, &QPushButton::clicked, this, &ClientWindow::disconnectFromServer);
    connect(m_captureButton, &QPushButton::clicked, this, [this]() {
        if (m_videoLabel->pixmap()) {
            QString fileName = QFileDialog::getSaveFileName(this, "保存抓拍", 
                                                           QString("capture_%1.png")
                                                           .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
                                                           "PNG Images (*.png)");
            if (!fileName.isEmpty()) {
                m_videoLabel->pixmap()->save(fileName);
                QMessageBox::information(this, "成功", "抓拍已保存");
            }
        }
    });
}

void ClientWindow::connectToServer()
{
    if (m_socket) {
        disconnectFromServer();
    }
    
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &ClientWindow::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientWindow::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientWindow::onReadyRead);
    
    m_socket->connectToHost("127.0.0.1", 8080);
    updateStatus("正在连接服务器...");
}

void ClientWindow::disconnectFromServer()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    m_videoLabel->clear();
    updateStatus("未连接");
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
    m_captureButton->setEnabled(false);
}

void ClientWindow::onConnected()
{
    updateStatus("已连接到服务器");
    m_connectButton->setEnabled(false);
    m_disconnectButton->setEnabled(true);
    m_captureButton->setEnabled(true);
    
    // 开始请求视频帧
    requestVideoFrame();
}

void ClientWindow::onDisconnected()
{
    updateStatus("与服务器断开连接");
    disconnectFromServer();
}

void ClientWindow::onReadyRead()
{
    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_5_15);
    
    while (!stream.atEnd()) {
        if (m_packetSize == 0) {
            if (m_socket->bytesAvailable() < sizeof(quint32)) {
                return;
            }
            stream >> m_packetSize;
            m_receivingPacket = true;
        }
        
        if (m_socket->bytesAvailable() < m_packetSize) {
            return;
        }
        
        QString dataType;
        stream >> dataType;
        
        if (dataType == "VIDEO_FRAME") {
            QImage frame;
            stream >> frame;
            displayFrame(frame);
        } else if (dataType == "PROPERTIES") {
            QMap<QString, QString> properties;
            stream >> properties;
            
            QString propsText = "设备属性: ";
            for (auto it = properties.begin(); it != properties.end(); ++it) {
                propsText += QString("%1:%2 ").arg(it.key()).arg(it.value());
            }
            qDebug() << propsText;
        }
        
        m_packetSize = 0;
        m_receivingPacket = false;
    }
    
    // 继续请求下一帧
    requestVideoFrame();
}

void ClientWindow::requestVideoFrame()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << QString("REQUEST_FRAME");
        m_socket->write(data);
    }
}

void ClientWindow::displayFrame(const QImage& frame)
{
    if (!frame.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(frame.scaled(m_videoLabel->size(), 
                                                        Qt::KeepAspectRatio, 
                                                        Qt::SmoothTransformation));
        m_videoLabel->setPixmap(pixmap);
        
        updateStatus(QString("分辨率: %1x%2 | 帧大小: %3 KB")
                    .arg(frame.width()).arg(frame.height())
                    .arg(frame.sizeInBytes() / 1024));
    }
}

void ClientWindow::updateStatus(const QString& status)
{
    m_statusLabel->setText(QString("状态: %1").arg(status));
}
