#include <QCoreApplication>
#include <QCommandLineParser>
#include "videoserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Video Surveillance Server");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("QT Video Surveillance Server");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption portOption("p", "Port number", "port", "8080");
    parser.addOption(portOption);
    parser.process(app);

    quint16 port = parser.value(portOption).toUShort();

    VideoServer server;
    if (!server.startServer(port)) {
        return -1;
    }

    qDebug() << "Video surveillance server running...";
    qDebug() << "Press Ctrl+C to stop the server";

    return app.exec();
}
