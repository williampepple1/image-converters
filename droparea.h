#ifndef DROPAREA_H
#define DROPAREA_H

#include <QListWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

/**
 * @brief A QListWidget that accepts file drops
 */
class DropArea : public QListWidget
{
    Q_OBJECT

public:
    explicit DropArea(QWidget *parent = nullptr);

signals:
    void filesDropped(const QStringList& files);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    bool isValidImageFile(const QString& path) const;
    QStringList m_supportedExtensions;
};

#endif // DROPAREA_H
