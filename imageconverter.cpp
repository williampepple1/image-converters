#include "imageconverter.h"
#include "heifhandler.h"
#include "avifhandler.h"
#include "icohandler.h"

#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>

ImageConverter::ImageConverter(QObject *parent)
    : QObject(parent)
{
}

ConversionResult ImageConverter::convert(const QString& inputPath, const QString& outputFolder, Format targetFormat, int quality)
{
    ConversionResult result;
    result.inputFile = inputPath;
    result.success = false;

    // Check if input file exists
    QFileInfo inputInfo(inputPath);
    if (!inputInfo.exists()) {
        result.errorMessage = "Input file does not exist";
        return result;
    }

    // Load the image
    QImage image;
    QString inputSuffix = inputInfo.suffix().toLower();

    // Check if input is HEIC/HEIF
    if (inputSuffix == "heic" || inputSuffix == "heif") {
        QString heifError;
        if (!HeifHandler::read(inputPath, image, heifError)) {
            // Try Qt's native loading as fallback (in case of Qt plugin)
            if (!image.load(inputPath)) {
                result.errorMessage = heifError.isEmpty() ?
                    "Failed to load HEIC/HEIF image" : heifError;
                return result;
            }
        }
    }
    // Check if input is AVIF
    else if (inputSuffix == "avif") {
        QString avifError;
        if (!AvifHandler::read(inputPath, image, avifError)) {
            // Try Qt's native loading as fallback (in case of Qt plugin)
            if (!image.load(inputPath)) {
                result.errorMessage = avifError.isEmpty() ?
                    "Failed to load AVIF image" : avifError;
                return result;
            }
        }
    }
    else if (!image.load(inputPath)) {
        result.errorMessage = "Failed to load image. Format may not be supported.";
        return result;
    }

    // Generate output path
    result.outputFile = generateOutputPath(inputPath, outputFolder, targetFormat);

    // Ensure output directory exists
    QFileInfo outputInfo(result.outputFile);
    QDir().mkpath(outputInfo.absolutePath());

    // Determine the format string and quality for saving
    const char* formatStr = nullptr;
    int saveQuality = quality;

    switch (targetFormat) {
        case Format::JPEG:
            formatStr = "JPEG";
            if (saveQuality < 0) saveQuality = 90; // Default JPEG quality
            // Convert to RGB if image has alpha (JPEG doesn't support transparency)
            if (image.hasAlphaChannel()) {
                QImage rgbImage(image.size(), QImage::Format_RGB32);
                rgbImage.fill(Qt::white); // Fill with white background
                QPainter painter(&rgbImage);
                painter.drawImage(0, 0, image);
                painter.end();
                image = rgbImage;
            }
            break;
        case Format::PNG:
            formatStr = "PNG";
            // PNG uses compression level 0-9 (via quality), -1 for default
            if (saveQuality < 0) saveQuality = -1;
            break;
        case Format::GIF:
            formatStr = "GIF";
            // GIF requires indexed color
            if (image.colorCount() == 0 || image.colorCount() > 256) {
                image = image.convertToFormat(QImage::Format_Indexed8);
            }
            break;
        case Format::BMP:
            formatStr = "BMP";
            break;
        case Format::WebP:
            formatStr = "WEBP";
            if (saveQuality < 0) saveQuality = 90;
            break;
        case Format::TIFF:
            formatStr = "TIFF";
            // TIFF uses lossless compression by default
            // Quality parameter is ignored for TIFF (always lossless)
            break;
        case Format::HEIC:
            {
                // Use HeifHandler for HEIC output
                int heicQuality = (saveQuality < 0) ? 90 : saveQuality;
                QString heifError;
                if (HeifHandler::write(result.outputFile, image, heicQuality, heifError)) {
                    result.success = true;
                } else {
                    result.errorMessage = heifError;
                }
                return result;
            }
        case Format::AVIF:
            {
                // Use AvifHandler for AVIF output
                int avifQuality = (saveQuality < 0) ? 80 : saveQuality;
                QString avifError;
                if (AvifHandler::write(result.outputFile, image, avifQuality, avifError)) {
                    result.success = true;
                } else {
                    result.errorMessage = avifError;
                }
                return result;
            }
        case Format::ICO:
            {
                // Use IcoHandler for ICO output with multiple sizes
                QString icoError;
                if (IcoHandler::write(result.outputFile, image, icoError)) {
                    result.success = true;
                } else {
                    result.errorMessage = icoError;
                }
                return result;
            }
    }

    // Save the image
    bool saved = image.save(result.outputFile, formatStr, saveQuality);

    if (saved) {
        result.success = true;
    } else {
        result.errorMessage = "Failed to save image. Check if format is supported.";
    }

    return result;
}

QString ImageConverter::getExtension(Format format)
{
    switch (format) {
        case Format::JPEG: return ".jpg";
        case Format::PNG: return ".png";
        case Format::WebP: return ".webp";
        case Format::GIF: return ".gif";
        case Format::TIFF: return ".tiff";
        case Format::BMP: return ".bmp";
        case Format::HEIC: return ".heic";
        case Format::AVIF: return ".avif";
        case Format::ICO: return ".ico";
    }
    return ".png"; // Default fallback
}

ImageConverter::Format ImageConverter::formatFromIndex(int index)
{
    switch (index) {
        case 0: return Format::JPEG;
        case 1: return Format::PNG;
        case 2: return Format::WebP;
        case 3: return Format::GIF;
        case 4: return Format::TIFF;
        case 5: return Format::BMP;
        case 6: return Format::HEIC;
        case 7: return Format::AVIF;
        case 8: return Format::ICO;
        default: return Format::PNG;
    }
}

bool ImageConverter::canRead(const QString& filePath)
{
    QImageReader reader(filePath);
    return reader.canRead();
}

bool ImageConverter::isFormatSupported(Format format)
{
    QList<QByteArray> supportedFormats = QImageWriter::supportedImageFormats();

    switch (format) {
        case Format::JPEG:
            return supportedFormats.contains("jpg") || supportedFormats.contains("jpeg");
        case Format::PNG:
            return supportedFormats.contains("png");
        case Format::WebP:
            return supportedFormats.contains("webp");
        case Format::GIF:
            return supportedFormats.contains("gif");
        case Format::TIFF:
            return supportedFormats.contains("tiff") || supportedFormats.contains("tif");
        case Format::BMP:
            return supportedFormats.contains("bmp");
        case Format::HEIC:
            return HeifHandler::isAvailable() ||
                   supportedFormats.contains("heic") || supportedFormats.contains("heif");
        case Format::AVIF:
            return AvifHandler::isAvailable() ||
                   supportedFormats.contains("avif");
        case Format::ICO:
            return IcoHandler::isAvailable() ||
                   supportedFormats.contains("ico");
    }
    return false;
}

QString ImageConverter::getFormatName(Format format)
{
    switch (format) {
        case Format::JPEG: return "JPEG";
        case Format::PNG: return "PNG";
        case Format::WebP: return "WebP";
        case Format::GIF: return "GIF";
        case Format::TIFF: return "TIFF";
        case Format::BMP: return "BMP";
        case Format::HEIC: return "HEIC";
        case Format::AVIF: return "AVIF";
        case Format::ICO: return "ICO";
    }
    return "Unknown";
}

QStringList ImageConverter::getSupportedReadFormats()
{
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QStringList result;
    for (const QByteArray& format : formats) {
        result.append(QString::fromLatin1(format).toUpper());
    }
    result.removeDuplicates();
    result.sort();
    return result;
}

QStringList ImageConverter::getSupportedWriteFormats()
{
    QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    QStringList result;
    for (const QByteArray& format : formats) {
        result.append(QString::fromLatin1(format).toUpper());
    }
    result.removeDuplicates();
    result.sort();
    return result;
}

QString ImageConverter::generateOutputPath(const QString& inputPath, const QString& outputFolder, Format targetFormat)
{
    QFileInfo inputInfo(inputPath);
    QString baseName = inputInfo.completeBaseName();
    QString extension = getExtension(targetFormat);

    QString outputDir = outputFolder.isEmpty() ? inputInfo.absolutePath() : outputFolder;

    QString outputPath = outputDir + "/" + baseName + extension;

    // Handle filename conflicts by adding a number suffix
    int counter = 1;
    while (QFileInfo::exists(outputPath) && outputPath != inputPath) {
        outputPath = outputDir + "/" + baseName + "_" + QString::number(counter) + extension;
        counter++;
    }

    return outputPath;
}
