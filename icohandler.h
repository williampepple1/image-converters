#ifndef ICOHANDLER_H
#define ICOHANDLER_H

#include <QImage>
#include <QString>
#include <QList>

/**
 * @brief Handler for ICO (Windows Icon) format
 *
 * This class provides read and write capabilities for ICO files.
 * When writing, it creates multi-size icons with standard Windows icon sizes.
 */
class IcoHandler
{
public:
    // Standard icon sizes for Windows icons
    static const QList<int> STANDARD_SIZES;

    IcoHandler();
    ~IcoHandler();

    /**
     * @brief Check if ICO handling is available
     * @return true (always available as it's implemented natively)
     */
    static bool isAvailable();

    /**
     * @brief Read an ICO file (uses Qt's native ICO reading)
     * @param filePath Path to the input file
     * @param image Output QImage (largest size in the ICO)
     * @param errorMessage Output error message if loading fails
     * @return true if successful, false otherwise
     */
    static bool read(const QString& filePath, QImage& image, QString& errorMessage);

    /**
     * @brief Write a QImage to ICO format with multiple sizes
     * @param filePath Path to the output file
     * @param image The source QImage to convert
     * @param sizes List of sizes to include (default: 16, 32, 48, 256)
     * @param errorMessage Output error message if saving fails
     * @return true if successful, false otherwise
     */
    static bool write(const QString& filePath, const QImage& image,
                      const QList<int>& sizes, QString& errorMessage);

    /**
     * @brief Write with default sizes (16, 32, 48, 256)
     */
    static bool write(const QString& filePath, const QImage& image, QString& errorMessage);

private:
    // ICO file format structures
    #pragma pack(push, 1)
    struct ICONDIR {
        quint16 idReserved;   // Reserved (must be 0)
        quint16 idType;       // Resource type (1 for icons)
        quint16 idCount;      // Number of images
    };

    struct ICONDIRENTRY {
        quint8  bWidth;       // Width (0 means 256)
        quint8  bHeight;      // Height (0 means 256)
        quint8  bColorCount;  // Number of colors (0 for >= 8bpp)
        quint8  bReserved;    // Reserved
        quint16 wPlanes;      // Color planes
        quint16 wBitCount;    // Bits per pixel
        quint32 dwBytesInRes; // Size of image data
        quint32 dwImageOffset;// Offset to image data
    };
    #pragma pack(pop)

    static QByteArray createPngData(const QImage& image, int size);
};

#endif // ICOHANDLER_H
