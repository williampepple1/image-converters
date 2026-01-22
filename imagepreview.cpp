#include "imagepreview.h"

#include <QFileInfo>
#include <QPixmap>

ImagePreview::ImagePreview(QWidget *parent)
    : QWidget(parent)
    , m_maxPreviewSize(150)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setMinimumSize(m_maxPreviewSize, m_maxPreviewSize);
    m_imageLabel->setMaximumSize(m_maxPreviewSize, m_maxPreviewSize);
    m_imageLabel->setStyleSheet(
        "background-color: #313244; "
        "border: 2px solid #45475a; "
        "border-radius: 8px;"
    );

    m_infoLabel = new QLabel(this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("color: #a6adc8; font-size: 10px;");
    m_infoLabel->setWordWrap(true);

    layout->addWidget(m_imageLabel);
    layout->addWidget(m_infoLabel);

    clear();
}

void ImagePreview::setImage(const QString& filePath)
{
    QPixmap pixmap(filePath);

    if (pixmap.isNull()) {
        m_imageLabel->setText("Preview\nnot available");
        m_infoLabel->setText("");
        return;
    }

    // Scale to fit
    QPixmap scaled = pixmap.scaled(
        m_maxPreviewSize - 10,
        m_maxPreviewSize - 10,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    m_imageLabel->setPixmap(scaled);

    // Show image info
    QFileInfo info(filePath);
    QString sizeStr;
    qint64 bytes = info.size();
    if (bytes < 1024) {
        sizeStr = QString("%1 B").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        sizeStr = QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    } else {
        sizeStr = QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    }

    m_infoLabel->setText(QString("%1x%2 | %3")
        .arg(pixmap.width())
        .arg(pixmap.height())
        .arg(sizeStr));
}

void ImagePreview::clear()
{
    m_imageLabel->clear();
    m_imageLabel->setText("Select an image\nto preview");
    m_infoLabel->setText("");
}
