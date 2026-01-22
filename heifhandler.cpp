#include "heifhandler.h"

#include <QFile>
#include <QDebug>

#ifdef HAVE_LIBHEIF
#include <libheif/heif.h>
#endif

HeifHandler::HeifHandler()
{
}

HeifHandler::~HeifHandler()
{
}

bool HeifHandler::isAvailable()
{
#ifdef HAVE_LIBHEIF
    return true;
#else
    return false;
#endif
}

bool HeifHandler::read(const QString& filePath, QImage& image, QString& errorMessage)
{
#ifdef HAVE_LIBHEIF
    // Create HEIF context
    heif_context* ctx = heif_context_alloc();
    if (!ctx) {
        errorMessage = "Failed to allocate HEIF context";
        return false;
    }

    // Read file
    heif_error error = heif_context_read_from_file(ctx, filePath.toUtf8().constData(), nullptr);
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to read HEIF file: %1").arg(error.message);
        heif_context_free(ctx);
        return false;
    }

    // Get primary image handle
    heif_image_handle* handle = nullptr;
    error = heif_context_get_primary_image_handle(ctx, &handle);
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to get image handle: %1").arg(error.message);
        heif_context_free(ctx);
        return false;
    }

    // Decode image
    heif_image* heifImage = nullptr;
    error = heif_decode_image(handle, &heifImage, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, nullptr);
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to decode image: %1").arg(error.message);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return false;
    }

    // Get image dimensions
    int width = heif_image_get_width(heifImage, heif_channel_interleaved);
    int height = heif_image_get_height(heifImage, heif_channel_interleaved);

    // Get image data
    int stride;
    const uint8_t* data = heif_image_get_plane_readonly(heifImage, heif_channel_interleaved, &stride);

    // Create QImage
    image = QImage(width, height, QImage::Format_RGBA8888);
    for (int y = 0; y < height; ++y) {
        memcpy(image.scanLine(y), data + y * stride, width * 4);
    }

    // Cleanup
    heif_image_release(heifImage);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    return true;
#else
    errorMessage = "HEIF support not compiled. Install libheif and rebuild with HAVE_LIBHEIF defined.";
    Q_UNUSED(filePath);
    Q_UNUSED(image);
    return false;
#endif
}

bool HeifHandler::write(const QString& filePath, const QImage& image, int quality, QString& errorMessage)
{
#ifdef HAVE_LIBHEIF
    // Convert image to RGBA format if needed
    QImage rgbaImage = image.convertToFormat(QImage::Format_RGBA8888);

    // Create HEIF context
    heif_context* ctx = heif_context_alloc();
    if (!ctx) {
        errorMessage = "Failed to allocate HEIF context";
        return false;
    }

    // Get encoder
    heif_encoder* encoder = nullptr;
    heif_error error = heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to get HEIF encoder: %1").arg(error.message);
        heif_context_free(ctx);
        return false;
    }

    // Set quality
    heif_encoder_set_lossy_quality(encoder, quality > 0 ? quality : 90);

    // Create HEIF image
    heif_image* heifImage = nullptr;
    error = heif_image_create(rgbaImage.width(), rgbaImage.height(),
                               heif_colorspace_RGB, heif_chroma_interleaved_RGBA, &heifImage);
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to create HEIF image: %1").arg(error.message);
        heif_encoder_release(encoder);
        heif_context_free(ctx);
        return false;
    }

    // Add plane
    error = heif_image_add_plane(heifImage, heif_channel_interleaved,
                                  rgbaImage.width(), rgbaImage.height(), 32);
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to add image plane: %1").arg(error.message);
        heif_image_release(heifImage);
        heif_encoder_release(encoder);
        heif_context_free(ctx);
        return false;
    }

    // Copy image data
    int stride;
    uint8_t* data = heif_image_get_plane(heifImage, heif_channel_interleaved, &stride);
    for (int y = 0; y < rgbaImage.height(); ++y) {
        memcpy(data + y * stride, rgbaImage.scanLine(y), rgbaImage.width() * 4);
    }

    // Encode image
    heif_image_handle* handle = nullptr;
    error = heif_context_encode_image(ctx, heifImage, encoder, nullptr, &handle);
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to encode image: %1").arg(error.message);
        heif_image_release(heifImage);
        heif_encoder_release(encoder);
        heif_context_free(ctx);
        return false;
    }

    // Write to file
    error = heif_context_write_to_file(ctx, filePath.toUtf8().constData());
    if (error.code != heif_error_Ok) {
        errorMessage = QString("Failed to write file: %1").arg(error.message);
        heif_image_handle_release(handle);
        heif_image_release(heifImage);
        heif_encoder_release(encoder);
        heif_context_free(ctx);
        return false;
    }

    // Cleanup
    heif_image_handle_release(handle);
    heif_image_release(heifImage);
    heif_encoder_release(encoder);
    heif_context_free(ctx);

    return true;
#else
    errorMessage = "HEIF support not compiled. Install libheif and rebuild with HAVE_LIBHEIF defined.";
    Q_UNUSED(filePath);
    Q_UNUSED(image);
    Q_UNUSED(quality);
    return false;
#endif
}
