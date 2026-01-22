#include "icohandler.h"

#include <QFile>
#include <QBuffer>
#include <QDebug>

const QList<int> IcoHandler::STANDARD_SIZES = {16, 32, 48, 256};

IcoHandler::IcoHandler()
{
}

IcoHandler::~IcoHandler()
{
}

bool IcoHandler::isAvailable()
{
    return true; // Always available (native implementation)
}

bool IcoHandler::read(const QString& filePath, QImage& image, QString& errorMessage)
{
    // Qt can read ICO files natively
    if (image.load(filePath, "ICO")) {
        return true;
    }

    // If that fails, try without format hint
    if (image.load(filePath)) {
        return true;
    }

    errorMessage = "Failed to load ICO file";
    return false;
}

bool IcoHandler::write(const QString& filePath, const QImage& image, QString& errorMessage)
{
    return write(filePath, image, STANDARD_SIZES, errorMessage);
}

bool IcoHandler::write(const QString& filePath, const QImage& image,
                       const QList<int>& sizes, QString& errorMessage)
{
    if (image.isNull()) {
        errorMessage = "Source image is null";
        return false;
    }

    if (sizes.isEmpty()) {
        errorMessage = "No sizes specified";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Failed to open file for writing";
        return false;
    }

    // Prepare PNG data for each size
    QList<QByteArray> pngDataList;
    for (int size : sizes) {
        QByteArray pngData = createPngData(image, size);
        if (pngData.isEmpty()) {
            errorMessage = QString("Failed to create PNG data for size %1").arg(size);
            file.close();
            return false;
        }
        pngDataList.append(pngData);
    }

    // Calculate offsets
    int headerSize = sizeof(ICONDIR) + sizes.count() * sizeof(ICONDIRENTRY);
    QList<quint32> offsets;
    quint32 currentOffset = headerSize;
    for (const QByteArray& pngData : pngDataList) {
        offsets.append(currentOffset);
        currentOffset += pngData.size();
    }

    // Write ICONDIR header
    ICONDIR iconDir;
    iconDir.idReserved = 0;
    iconDir.idType = 1; // Icon
    iconDir.idCount = sizes.count();
    file.write(reinterpret_cast<const char*>(&iconDir), sizeof(ICONDIR));

    // Write ICONDIRENTRY for each size
    for (int i = 0; i < sizes.count(); ++i) {
        int size = sizes[i];
        ICONDIRENTRY entry;
        entry.bWidth = (size >= 256) ? 0 : static_cast<quint8>(size);
        entry.bHeight = (size >= 256) ? 0 : static_cast<quint8>(size);
        entry.bColorCount = 0; // True color
        entry.bReserved = 0;
        entry.wPlanes = 1;
        entry.wBitCount = 32; // 32-bit RGBA
        entry.dwBytesInRes = pngDataList[i].size();
        entry.dwImageOffset = offsets[i];
        file.write(reinterpret_cast<const char*>(&entry), sizeof(ICONDIRENTRY));
    }

    // Write PNG data for each size
    for (const QByteArray& pngData : pngDataList) {
        file.write(pngData);
    }

    file.close();
    return true;
}

QByteArray IcoHandler::createPngData(const QImage& image, int size)
{
    // Scale image to target size
    QImage scaled = image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Create a square image with transparency
    QImage square(size, size, QImage::Format_ARGB32);
    square.fill(Qt::transparent);

    // Center the scaled image
    int xOffset = (size - scaled.width()) / 2;
    int yOffset = (size - scaled.height()) / 2;

    for (int y = 0; y < scaled.height(); ++y) {
        for (int x = 0; x < scaled.width(); ++x) {
            square.setPixel(x + xOffset, y + yOffset, scaled.pixel(x, y));
        }
    }

    // Convert to PNG
    QByteArray pngData;
    QBuffer buffer(&pngData);
    buffer.open(QIODevice::WriteOnly);
    if (!square.save(&buffer, "PNG")) {
        return QByteArray();
    }
    buffer.close();

    return pngData;
}
