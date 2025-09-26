#include <QApplication>
#include "clientwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Video Surveillance Client");
    app.setApplicationVersion("1.0");

    ClientWindow window;
    window.show();

    return app.exec();
}
