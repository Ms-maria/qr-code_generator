#ifndef QR_GENERATOR_H
#define QR_GENERATOR_H

#include <string>
#include <mutex>
#include <vector>
#include "logging.h"

class QRGenerator {
private:
    std::string qr_file = "/tmp/qrcode.png";
    std::mutex file_mutex;

    void saveQRToPNG(const std::string& data, const std::string& output_file);

public:
    /**
     * Генерирует QR-код из текста
     * @param data Текст для кодирования
     */
    void generateQR(const std::string& data);
    
    /**
     * Генерирует QR-код с геолокацией
     * @param latitude Широта (-90 до 90)
     * @param longitude Долгота (-180 до 180)
     * @param zoom Уровень масштаба (1-20)
     */
    void generateLocationQR(double latitude, double longitude, int zoom = 15);
    
    /**
     * Возвращает сгенерированное изображение QR-кода
     * @return Бинарные данные изображения PNG
     */
    std::string getQRImage();
};

#endif // QR_GENERATOR_H
