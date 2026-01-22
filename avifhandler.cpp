#include "avifhandler.h"

#include <QFile>
#include <QDebug>

#ifdef HAVE_LIBAVIF
#include <avif/avif.h>
#endif

AvifHandler::AvifHandler()
{
}

AvifHandler::~AvifHandler()
{
}

bool AvifHandler::isAvailable()
{
#ifdef HAVE_LIBAVIF
    return true;
#else
    return false;
#endif
}

bool AvifHandler::read(const QString& filePath, QImage& image, QString& errorMessage)
{
#ifdef HAVE_LIBAVIF
    // Read file into memory
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessage = "Failed to open file for reading";
        return false;
    }
    QByteArray fileData = file.readAll();
    file.close();

    // Create decoder
    avifDecoder* decoder = avifDecoderCreate();
    if (!decoder) {
        errorMessage = "Failed to create AVIF decoder";
        return false;
    }

    // Parse the file
    avifResult result = avifDecoderSetIOMemory(decoder,
        reinterpret_cast<const uint8_t*>(fileData.constData()),
        fileData.size());
    if (result != AVIF_RESULT_OK) {
        errorMessage = QString("Failed to set decoder input: %1").arg(avifResultToString(result));
        avifDecoderDestroy(decoder);
        return false;
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        errorMessage = QString("Failed to parse AVIF: %1").arg(avifResultToString(result));
        avifDecoderDestroy(decoder);
        return false;
    }

    // Decode the first image
    result = avifDecoderNextImage(decoder);
    if (result != AVIF_RESULT_OK) {
        errorMessage = QString("Failed to decode AVIF: %1").arg(avifResultToString(result));
        avifDecoderDestroy(decoder);
        return false;
    }

    // Convert to RGBA
    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, decoder->image);
    rgb.format = AVIF_RGB_FORMAT_RGBA;
    rgb.depth = 8;

    avifRGBImageAllocatePixels(&rgb);
    result = avifImageYUVToRGB(decoder->image, &rgb);
    if (result != AVIF_RESULT_OK) {
        errorMessage = QString("Failed to convert to RGB: %1").arg(avifResultToString(result));
        avifRGBImageFreePixels(&rgb);
        avifDecoderDestroy(decoder);
        return false;
    }

    // Create QImage
    image = QImage(rgb.width, rgb.height, QImage::Format_RGBA8888);
    for (uint32_t y = 0; y < rgb.height; ++y) {
        memcpy(image.scanLine(y), rgb.pixels + y * rgb.rowBytes, rgb.width * 4);
    }

    // Cleanup
    avifRGBImageFreePixels(&rgb);
    avifDecoderDestroy(decoder);

    return true;
#else
    errorMessage = "AVIF support not compiled. Install libavif and rebuild with HAVE_LIBAVIF defined.";
    Q_UNUSED(filePath);
    Q_UNUSED(image);
    return false;
#endif
}

bool AvifHandler::write(const QString& filePath, const QImage& image, int quality, QString& errorMessage)
{
#ifdef HAVE_LIBAVIF
    // Convert image to RGBA format if needed
    QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);

    // Create AVIF image
    avifImage* avifImg = avifImageCreate(rgbaImage.width(), rgbaImage.height(), 8, AVIF_PIXEL_FORMAT_YUV444);
    if (!avifImg) {
        errorMessage = "Failed to create AVIF image";
        return false;
    }

    // Create RGB image for conversion
    avifRGBImage rgb;
    avifRGBImageSetDefaults(&rgb, avifImg);
    rgb.format = AVIF_RGB_FORMAT_RGBA;
    rgb.depth = 8;
    rgb.pixels = const_cast<uint8_t*>(rgbaImage.bits());
    rgb.rowBytes = rgbaImage.bytesPerLine();

    // Convert RGB to YUV
    avifResult result = avifImageRGBToYUV(avifImg, &rgb);
    if (result != AVIF_RESULT_OK) {
        errorMessage = QString("Failed to convert to YUV: %1").arg(avifResultToString(result));
        avifImageDestroy(avifImg);
        return false;
    }

    // Create encoder
    avifEncoder* encoder = avifEncoderCreate();
    if (!encoder) {
        errorMessage = "Failed to create AVIF encoder";
        avifImageDestroy(avifImg);
        return false;
    }

    // Set quality (convert 0-100 to AVIF's 0-63 quantizer, where 0 is best quality)
    int q = quality > 0 ? quality : 80;
    encoder->quality = q;
    encoder->qualityAlpha = q;
    encoder->speed = AVIF_SPEED_DEFAULT;

    // Encode
    avifRWData output = AVIF_DATA_EMPTY;
    result = avifEncoderWrite(encoder, avifImg, &output);
    if (result != AVIF_RESULT_OK) {
        errorMessage = QString("Failed to encode AVIF: %1").arg(avifResultToString(result));
        avifEncoderDestroy(encoder);
        avifImageDestroy(avifImg);
        return false;
    }

    // Write to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        errorMessage = "Failed to open file for writing";
        avifRWDataFree(&output);
        avifEncoderDestroy(encoder);
        avifImageDestroy(avifImg);
        return false;
    }

    qint64 written = file.write(reinterpret_cast<const char*>(output.data), output.size);
    file.close();

    if (written != static_cast<qint64>(output.size)) {
        errorMessage = "Failed to write complete file";
        avifRWDataFree(&output);
        avifEncoderDestroy(encoder);
        avifImageDestroy(avifImg);
        return false;
    }

    // Cleanup
    avifRWDataFree(&output);
    avifEncoderDestroy(encoder);
    avifImageDestroy(avifImg);

    return true;
#else
    errorMessage = "AVIF support not compiled. Install libavif and rebuild with HAVE_LIBAVIF defined.";
    Q_UNUSED(filePath);
    Q_UNUSED(image);
    Q_UNUSED(quality);
    return false;
#endif
}
