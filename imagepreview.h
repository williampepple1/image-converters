#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief Widget to preview selected images
 */
class ImagePreview : public QWidget
{
    Q_OBJECT

public:
    explicit ImagePreview(QWidget *parent = nullptr);

    void setImage(const QString& filePath);
    void clear();

private:
    QLabel* m_imageLabel;
    QLabel* m_infoLabel;
    int m_maxPreviewSize;
};

#endif // IMAGEPREVIEW_H
