#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "imageconverter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectFilesClicked();
    void onClearFilesClicked();
    void onOutputFolderClicked();
    void onConvertClicked();

private:
    Ui::MainWindow *ui;
    QStringList m_selectedFiles;
    QString m_outputFolder;
    ImageConverter *m_converter;

    void updateFileList();
    void updateConvertButtonState();
    void showConversionResults(const QList<ConversionResult>& results);
};

#endif // MAINWINDOW_H
