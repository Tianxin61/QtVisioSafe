#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QLabel>
#include <QPushButton>

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void connectToServer();
    void disconnectFromServer();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void requestVideoFrame();

private:
    void setupUI();
    void displayFrame(const QImage& frame);
    void updateStatus(const QString& status);

    QTcpSocket* m_socket;
    QLabel* m_videoLabel;
    QLabel* m_statusLabel;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;
    QPushButton* m_captureButton;
    
    quint32 m_packetSize;
    bool m_receivingPacket;
};

#endif // CLIENTWINDOW_H
