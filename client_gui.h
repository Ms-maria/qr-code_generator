#ifndef CLIENT_GUI_H
#define CLIENT_GUI_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QDoubleValidator>
#include <QDebug>
#include <QTcpSocket>
#include <QPixmap>
#include <QThread>
#include "/home/tanya/qr_project/common/include/logging.h"
#include "/home/tanya/qr_project/common/include/network_utils.h"

class ClientThread : public QThread {
    Q_OBJECT
public:
    ClientThread(const QString& request, QObject* parent = nullptr)
        : QThread(parent), request(request) {}
    
    void run() override {
        try {
            std::string response = NetworkUtils::sendRequest(
                "localhost", 
                8080, 
                request.toStdString()
            );
            emit resultReady(QByteArray::fromStdString(response));
        } catch (const NetworkException& e) {
            LOG_ERROR(e.what());
            emit errorOccurred(QString::fromStdString(e.what()));
        }
    }
    
signals:
    void resultReady(const QByteArray& result);
    void errorOccurred(const QString& error);
    
private:
    QString request;
};

class ClientGUI : public QMainWindow {
    Q_OBJECT
    
public:
    ClientGUI(QWidget *parent = nullptr);
    
private slots:
    void generateTextQR();
    void generateGeoQR();
    void handleQRResponse(const QByteArray& response);
    void handleError(const QString& error);
    
private:
    QLabel *qrDisplay;
    QLineEdit *textInput;
    QLineEdit *latInput;
    QLineEdit *lonInput;
    QPushButton *textButton;
    QPushButton *geoButton;
    
    void setupUI();
    void showMessage(const QString& message);
};

#endif // CLIENT_GUI_H
