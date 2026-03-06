#include <sailfishapp.h>

#include <QGuiApplication>
#include <QQuickView>
#include <QtQml>

#include "BridgeHelper.h"

int main(int argc, char *argv[])
{
    QGuiApplication *app = SailfishApp::application(argc, argv);

    qmlRegisterType<BridgeHelper>("harbour.hydroxide", 1, 0, "BridgeHelper");

    QQuickView *view = SailfishApp::createView();
    view->setSource(SailfishApp::pathTo("qml/harbour-hydroxide-ui.qml"));
    view->show();

    return app->exec();
}