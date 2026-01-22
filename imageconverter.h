#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QString>
#include <QStringList>
#include <QObject>

struct ConversionResult {
    QString inputFile;
    QString outputFile;
    bool success;
    QString errorMessage;
};

class ImageConverter : public QObject
{
    Q_OBJECT

public:
    enum class Format {
        JPEG,
        PNG,
        WebP,
        GIF,
        TIFF,
        BMP,
        HEIC,
        AVIF,
        ICO
    };

    explicit ImageConverter(QObject *parent = nullptr);

    // Convert a single file
    ConversionResult convert(const QString& inputPath, const QString& outputFolder, Format targetFormat, int quality = -1);

    // Get file extension for format
    static QString getExtension(Format format);

    // Get format from combo box index
    static Format formatFromIndex(int index);

    // Check if format is supported for reading
    static bool canRead(const QString& filePath);

signals:
    void conversionProgress(int current, int total);
    void conversionComplete(const QList<ConversionResult>& results);

private:
    QString generateOutputPath(const QString& inputPath, const QString& outputFolder, Format targetFormat);
};

#endif // IMAGECONVERTER_H
