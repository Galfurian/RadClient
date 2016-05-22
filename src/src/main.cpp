#include "../inc/radclient.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(RadClient);

    app.setOverrideCursor(QCursor(QPixmap(":/resources/pointer/pointer-default.png"), 0, 0));

    //QApplication::setStyle(QStyleFactory::create("Fusion"));
    app.setDesktopSettingsAware(false);

    QFile file(":/RadClient.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);

    RadClient radClient;
    radClient.show();

    return app.exec();
}
