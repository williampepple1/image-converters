#ifndef HEIFHANDLER_H
#define HEIFHANDLER_H

#include <QImage>
#include <QString>

/**
 * @brief Handler for HEIC/HEIF image format using libheif
 *
 * This class provides read and write capabilities for HEIC/HEIF images.
 * It requires libheif to be installed on the system.
 */
class HeifHandler
{
public:
    HeifHandler();
    ~HeifHandler();

    /**
     * @brief Check if libheif is available
     * @return true if libheif is available and can be used
     */
    static bool isAvailable();

    /**
     * @brief Read a HEIC/HEIF image file
     * @param filePath Path to the input file
     * @param image Output QImage to store the loaded image
     * @param errorMessage Output error message if loading fails
     * @return true if successful, false otherwise
     */
    static bool read(const QString& filePath, QImage& image, QString& errorMessage);

    /**
     * @brief Write a QImage to HEIC/HEIF format
     * @param filePath Path to the output file
     * @param image The QImage to save
     * @param quality Quality setting (0-100, default 90)
     * @param errorMessage Output error message if saving fails
     * @return true if successful, false otherwise
     */
    static bool write(const QString& filePath, const QImage& image, int quality, QString& errorMessage);
};

#endif // HEIFHANDLER_H
