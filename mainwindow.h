#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include "imageconverter.h"
#include "conversionworker.h"

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

    // Conversion progress slots
    void onConversionStarted();
    void onConversionProgress(int current, int total, const QString& currentFile);
    void onConversionFileCompleted(const ConversionResult& result);
    void onConversionFinished(const QList<ConversionResult>& results);
    void onConversionError(const QString& message);

private:
    Ui::MainWindow *ui;
    QStringList m_selectedFiles;
    QString m_outputFolder;
    ImageConverter *m_converter;
    ConversionController *m_conversionController;

    void updateFileList();
    void updateConvertButtonState();
    void showConversionResults(const QList<ConversionResult>& results);
    void updateFormatAvailability();
    void setUIEnabled(bool enabled);
};

#endif // MAINWINDOW_H
