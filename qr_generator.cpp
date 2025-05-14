#include "qr_generator.h"
#include <qrencode.h>
#include <png.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

void QRGenerator::saveQRToPNG(const std::string& data, const std::string& output_file) {
    std::lock_guard<std::mutex> lock(file_mutex);
    
    // Генерация QR-кода
    QRcode* qr = QRcode_encodeString(data.c_str(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
    if (!qr) {
        LOG_ERROR("Failed to generate QR code");
        throw std::runtime_error("Failed to generate QR code");
    }

    // Сохранение в PNG
    FILE* fp = fopen(output_file.c_str(), "wb");
    if (!fp) {
        QRcode_free(qr);
        LOG_ERROR("Failed to create QR image file: " + output_file);
        throw std::runtime_error("Failed to create QR image file");
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        QRcode_free(qr);
        LOG_ERROR("Failed to initialize PNG writer");
        throw std::runtime_error("Failed to initialize PNG writer");
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        QRcode_free(qr);
        LOG_ERROR("Failed to initialize PNG info");
        throw std::runtime_error("Failed to initialize PNG info");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        QRcode_free(qr);
        LOG_ERROR("Error during PNG creation");
        throw std::runtime_error("Error during PNG creation");
    }

    png_init_io(png, fp);
    png_set_IHDR(png, info, qr->width, qr->width, 1,
                 PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    png_bytep row = (png_bytep)malloc((qr->width + 7) / 8);
    for (int y = 0; y < qr->width; y++) {
        memset(row, 0, (qr->width + 7) / 8);
        for (int x = 0; x < qr->width; x++) {
            if (qr->data[y * qr->width + x] & 1) {
                row[x / 8] |= (1 << (7 - x % 8));
            }
        }
        png_write_row(png, row);
    }

    free(row);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    QRcode_free(qr);
    LOG_DEBUG("QR code saved to " + output_file);
}

void QRGenerator::generateQR(const std::string& data) {
    LOG_INFO("Generating QR code for: " + data);
    saveQRToPNG(data, qr_file);
}

void QRGenerator::generateLocationQR(double latitude, double longitude, int zoom) {
    LOG_INFO("Generating QR for coordinates: lat=" + std::to_string(latitude) + 
            ", long=" + std::to_string(longitude));
    
    if (latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
        LOG_ERROR("Invalid coordinates: lat=" + std::to_string(latitude) + 
                 ", long=" + std::to_string(longitude));
        throw std::runtime_error("Invalid coordinates (lat: -90..90, long: -180..180)");
    }

    std::ostringstream location_stream;
    location_stream << std::fixed << std::setprecision(5);
    location_stream << "geo:" << latitude << "," << longitude << "?z=" << zoom;
    
    LOG_DEBUG("QR code content: " + location_stream.str());
    saveQRToPNG(location_stream.str(), qr_file);
}

std::string QRGenerator::getQRImage() {
    std::lock_guard<std::mutex> lock(file_mutex);
    std::ifstream file(qr_file, std::ios::binary);
    if (!file) {
        LOG_ERROR("Failed to read QR image file: " + qr_file);
        throw std::runtime_error("Failed to read QR image file");
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), 
                           std::istreambuf_iterator<char>());
    return std::string(buffer.begin(), buffer.end());
}
