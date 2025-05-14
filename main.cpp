#include <QApplication>
#include "client_gui.h"
#include "/home/tanya/qr_project/common/include/logging.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    ClientGUI window;
    window.show();
    
    return app.exec();
}
