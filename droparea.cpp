#include "droparea.h"

#include <QFileInfo>
#include <QUrl>

DropArea::DropArea(QWidget *parent)
    : QListWidget(parent)
{
    setAcceptDrops(true);

    // Supported image extensions
    m_supportedExtensions = {
        "jpg", "jpeg", "png", "webp", "gif",
        "tiff", "tif", "bmp", "heic", "heif",
        "avif", "ico"
    };
}

void DropArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        // Check if any of the URLs are valid image files
        bool hasValidImages = false;
        for (const QUrl& url : event->mimeData()->urls()) {
            if (url.isLocalFile() && isValidImageFile(url.toLocalFile())) {
                hasValidImages = true;
                break;
            }
        }

        if (hasValidImages) {
            event->acceptProposedAction();
            setStyleSheet(styleSheet() + "border: 3px dashed #89b4fa;");
        }
    }
}

void DropArea::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void DropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    // Reset style
    setStyleSheet(styleSheet().replace("border: 3px dashed #89b4fa;", ""));
}

void DropArea::dropEvent(QDropEvent *event)
{
    // Reset style
    setStyleSheet(styleSheet().replace("border: 3px dashed #89b4fa;", ""));

    if (event->mimeData()->hasUrls()) {
        QStringList files;
        for (const QUrl& url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                QString path = url.toLocalFile();
                if (isValidImageFile(path)) {
                    files.append(path);
                }
            }
        }

        if (!files.isEmpty()) {
            emit filesDropped(files);
            event->acceptProposedAction();
        }
    }
}

bool DropArea::isValidImageFile(const QString& path) const
{
    QFileInfo info(path);
    if (!info.isFile()) {
        return false;
    }

    QString ext = info.suffix().toLower();
    return m_supportedExtensions.contains(ext);
}
