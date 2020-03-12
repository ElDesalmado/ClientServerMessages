#include "server.h"

#include "qapplication.h"

int main(int argc, char **argv)
{
    QApplication app{ argc, argv };
    Server server;
    server.show();

    return app.exec();
}
