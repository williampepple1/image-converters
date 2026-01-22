#ifndef AVIFHANDLER_H
#define AVIFHANDLER_H

#include <QImage>
#include <QString>

/**
 * @brief Handler for AVIF image format using libavif
 *
 * This class provides read and write capabilities for AVIF images.
 * It requires libavif to be installed on the system.
 */
class AvifHandler
{
public:
    AvifHandler();
    ~AvifHandler();

    /**
     * @brief Check if libavif is available
     * @return true if libavif is available and can be used
     */
    static bool isAvailable();

    /**
     * @brief Read an AVIF image file
     * @param filePath Path to the input file
     * @param image Output QImage to store the loaded image
     * @param errorMessage Output error message if loading fails
     * @return true if successful, false otherwise
     */
    static bool read(const QString& filePath, QImage& image, QString& errorMessage);

    /**
     * @brief Write a QImage to AVIF format
     * @param filePath Path to the output file
     * @param image The QImage to save
     * @param quality Quality setting (0-100, default 80)
     * @param errorMessage Output error message if saving fails
     * @return true if successful, false otherwise
     */
    static bool write(const QString& filePath, const QImage& image, int quality, QString& errorMessage);
};

#endif // AVIFHANDLER_H
