#include "client_gui.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QBuffer>
#include <QDoubleValidator>
#include <QDebug>

ClientGUI::ClientGUI(QWidget *parent) : QMainWindow(parent) {
    Logger::getInstance().init("client.log");
    LOG_INFO("Starting QR Client GUI");
    setupUI();
    setWindowTitle("QR Code Generator Client");
    resize(600, 400);
}

void ClientGUI::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // Left side - input forms
    QWidget *inputWidget = new QWidget;
    QVBoxLayout *inputLayout = new QVBoxLayout(inputWidget);
    
    // Text input
    QGroupBox *textGroup = new QGroupBox("Text QR Code");
    QVBoxLayout *textLayout = new QVBoxLayout;
    textInput = new QLineEdit;
    textButton = new QPushButton("Generate QR");
    textLayout->addWidget(textInput);
    textLayout->addWidget(textButton);
    textGroup->setLayout(textLayout);
    
    // Geo input
    QGroupBox *geoGroup = new QGroupBox("Location QR Code");
    QFormLayout *geoLayout = new QFormLayout;
    latInput = new QLineEdit;
    lonInput = new QLineEdit;
    geoButton = new QPushButton("Generate QR");
    geoLayout->addRow("Latitude:", latInput);
    geoLayout->addRow("Longitude:", lonInput);
    geoLayout->addWidget(geoButton);
    geoGroup->setLayout(geoLayout);
    
    inputLayout->addWidget(textGroup);
    inputLayout->addWidget(geoGroup);
    inputLayout->addStretch();
    
    // Right side - QR display
    qrDisplay = new QLabel;
    qrDisplay->setAlignment(Qt::AlignCenter);
    qrDisplay->setFrameShape(QFrame::Box);
    
    mainLayout->addWidget(inputWidget, 1);
    mainLayout->addWidget(qrDisplay, 2);
    
    setCentralWidget(centralWidget);
    
    // Connections
    connect(textButton, &QPushButton::clicked, this, &ClientGUI::generateTextQR);
    connect(geoButton, &QPushButton::clicked, this, &ClientGUI::generateGeoQR);

    // Validators
    QDoubleValidator *latValidator = new QDoubleValidator(-90.0, 90.0, 6, this);
    latValidator->setNotation(QDoubleValidator::StandardNotation);
    latInput->setValidator(latValidator);
    
    QDoubleValidator *lonValidator = new QDoubleValidator(-180.0, 180.0, 6, this);
    lonValidator->setNotation(QDoubleValidator::StandardNotation);
    lonInput->setValidator(lonValidator);
}

void ClientGUI::generateTextQR() {
    QString text = textInput->text().trimmed();
    if (text.isEmpty()) {
        showMessage("Please enter text for QR code");
        return;
    }
    
    LOG_INFO("Generating text QR for: " + text.toStdString());
    ClientThread *thread = new ClientThread("TEXT:" + text, this);
    connect(thread, &ClientThread::resultReady, this, &ClientGUI::handleQRResponse);
    connect(thread, &ClientThread::errorOccurred, this, &ClientGUI::handleError);
    connect(thread, &ClientThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void ClientGUI::generateGeoQR() {
    QString latText = latInput->text().trimmed();
    QString lonText = lonInput->text().trimmed();
    
    LOG_DEBUG("Raw coordinates input - lat: " + latText.toStdString() + 
             ", lon: " + lonText.toStdString());
    
    latText.replace(',', '.');
    lonText.replace(',', '.');
    
    bool latOk, lonOk;
    double lat = latText.toDouble(&latOk);
    double lon = lonText.toDouble(&lonOk);
    
    if (!latOk || !lonOk) {
        showMessage("Please enter valid coordinates (e.g. 45.1234 or 45,1234)");
        return;
    }
    
    if (lat < -90 || lat > 90 || lon < -180 || lon > 180) {
        showMessage("Invalid coordinates range:\nLatitude: -90 to 90\nLongitude: -180 to 180");
        return;
    }
    
    LOG_INFO("Generating geo QR for coordinates - lat: " + std::to_string(lat) + 
            ", lon: " + std::to_string(lon));
    
    ClientThread *thread = new ClientThread(QString("GEO:%1,%2").arg(lat).arg(lon), this);
    connect(thread, &ClientThread::resultReady, this, &ClientGUI::handleQRResponse);
    connect(thread, &ClientThread::errorOccurred, this, &ClientGUI::handleError);
    connect(thread, &ClientThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void ClientGUI::handleQRResponse(const QByteArray& response) {
    if (response.startsWith("QRCODE:")) {
        QByteArray imageData = response.mid(7);
        QPixmap pixmap;
        if (pixmap.loadFromData(imageData)) {
            qrDisplay->setPixmap(pixmap.scaled(qrDisplay->size(), Qt::KeepAspectRatio));
            LOG_DEBUG("QR code displayed successfully");
        } else {
            LOG_ERROR("Failed to load QR image from data");
            showMessage("Failed to display QR code");
        }
    } else if (response.startsWith("ERROR:")) {
        LOG_ERROR("Server error: " + response.mid(6).toStdString());
        showMessage(response.mid(6));
    } else {
        LOG_ERROR("Invalid server response format");
        showMessage("Invalid server response");
    }
}

void ClientGUI::handleError(const QString& error) {
    LOG_ERROR("Client error: " + error.toStdString());
    showMessage(error);
}

void ClientGUI::showMessage(const QString& message) {
    QMessageBox::warning(this, "QR Generator", message);
}
