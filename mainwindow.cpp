#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_converter(new ImageConverter(this))
    , m_conversionController(new ConversionController(this))
{
    ui->setupUi(this);

    // Connect UI signals
    connect(ui->selectFilesBtn, &QPushButton::clicked, this, &MainWindow::onSelectFilesClicked);
    connect(ui->clearFilesBtn, &QPushButton::clicked, this, &MainWindow::onClearFilesClicked);
    connect(ui->outputFolderBtn, &QPushButton::clicked, this, &MainWindow::onOutputFolderClicked);
    connect(ui->convertBtn, &QPushButton::clicked, this, &MainWindow::onConvertClicked);

    // Connect conversion controller signals
    connect(m_conversionController, &ConversionController::started,
            this, &MainWindow::onConversionStarted);
    connect(m_conversionController, &ConversionController::progress,
            this, &MainWindow::onConversionProgress);
    connect(m_conversionController, &ConversionController::fileCompleted,
            this, &MainWindow::onConversionFileCompleted);
    connect(m_conversionController, &ConversionController::finished,
            this, &MainWindow::onConversionFinished);
    connect(m_conversionController, &ConversionController::error,
            this, &MainWindow::onConversionError);

    // Initial state
    ui->progressBar->setVisible(false);
    updateConvertButtonState();
    updateFormatAvailability();

    // Show supported formats in status bar
    QStringList writeFormats = ImageConverter::getSupportedWriteFormats();
    ui->statusbar->showMessage(QString("Ready - Supported formats: %1").arg(writeFormats.join(", ")));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSelectFilesClicked()
{
    QStringList filters;
    filters << "All Supported Images (*.jpg *.jpeg *.png *.webp *.gif *.tiff *.tif *.bmp *.heic *.heif *.avif *.ico)"
            << "JPEG Files (*.jpg *.jpeg)"
            << "PNG Files (*.png)"
            << "WebP Files (*.webp)"
            << "GIF Files (*.gif)"
            << "TIFF Files (*.tiff *.tif)"
            << "BMP Files (*.bmp)"
            << "HEIC Files (*.heic *.heif)"
            << "AVIF Files (*.avif)"
            << "ICO Files (*.ico)"
            << "All Files (*)";

    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Images to Convert",
        QString(),
        filters.join(";;")
    );

    if (!files.isEmpty()) {
        m_selectedFiles.append(files);
        m_selectedFiles.removeDuplicates();
        updateFileList();
        updateConvertButtonState();
        ui->statusbar->showMessage(QString("Selected %1 file(s)").arg(m_selectedFiles.size()));
    }
}

void MainWindow::onClearFilesClicked()
{
    m_selectedFiles.clear();
    updateFileList();
    updateConvertButtonState();
    ui->statusbar->showMessage("File list cleared");
}

void MainWindow::onOutputFolderClicked()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        "Select Output Folder",
        m_outputFolder.isEmpty() ? QString() : m_outputFolder
    );

    if (!folder.isEmpty()) {
        m_outputFolder = folder;
        ui->outputPathLabel->setText("Output: " + folder);
        ui->statusbar->showMessage("Output folder set");
    }
}

void MainWindow::onConvertClicked()
{
    if (m_selectedFiles.isEmpty()) {
        QMessageBox::warning(this, "No Files", "Please select files to convert.");
        return;
    }

    if (m_conversionController->isRunning()) {
        // Cancel conversion
        m_conversionController->cancelConversion();
        ui->convertBtn->setText("Cancelling...");
        ui->convertBtn->setEnabled(false);
        return;
    }

    // Get target format
    ImageConverter::Format targetFormat = ImageConverter::formatFromIndex(ui->formatComboBox->currentIndex());

    // Start conversion
    m_conversionController->startConversion(m_selectedFiles, m_outputFolder, targetFormat);
}

void MainWindow::onConversionStarted()
{
    setUIEnabled(false);
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    ui->convertBtn->setText("Cancel");
    ui->convertBtn->setEnabled(true);
    ui->statusbar->showMessage("Starting conversion...");
}

void MainWindow::onConversionProgress(int current, int total, const QString& currentFile)
{
    int percentage = (current * 100) / total;
    ui->progressBar->setValue(percentage);
    ui->statusbar->showMessage(QString("Converting %1 of %2: %3").arg(current).arg(total).arg(currentFile));
}

void MainWindow::onConversionFileCompleted(const ConversionResult& result)
{
    Q_UNUSED(result);
    // Could update list widget to show status per file
}

void MainWindow::onConversionFinished(const QList<ConversionResult>& results)
{
    setUIEnabled(true);
    ui->progressBar->setVisible(false);
    ui->convertBtn->setText("Convert Images");
    showConversionResults(results);
}

void MainWindow::onConversionError(const QString& message)
{
    ui->statusbar->showMessage("Error: " + message);
}

void MainWindow::updateFileList()
{
    ui->fileListWidget->clear();
    for (const QString& file : m_selectedFiles) {
        QFileInfo info(file);
        ui->fileListWidget->addItem(info.fileName() + "  (" + info.path() + ")");
    }
}

void MainWindow::updateConvertButtonState()
{
    ui->convertBtn->setEnabled(!m_selectedFiles.isEmpty());
}

void MainWindow::showConversionResults(const QList<ConversionResult>& results)
{
    int successCount = 0;
    int failCount = 0;
    QStringList errors;

    for (const ConversionResult& result : results) {
        if (result.success) {
            successCount++;
        } else {
            failCount++;
            QFileInfo info(result.inputFile);
            errors.append(QString("%1: %2").arg(info.fileName(), result.errorMessage));
        }
    }

    QString message;
    if (failCount == 0) {
        message = QString("Successfully converted %1 file(s)!").arg(successCount);
        ui->statusbar->showMessage(message);
        QMessageBox::information(this, "Conversion Complete", message);
    } else if (successCount == 0) {
        message = QString("All %1 conversion(s) failed.").arg(failCount);
        ui->statusbar->showMessage(message);
        QMessageBox::warning(this, "Conversion Failed",
            message + "\n\nErrors:\n" + errors.join("\n"));
    } else {
        message = QString("Converted %1 file(s), %2 failed.").arg(successCount).arg(failCount);
        ui->statusbar->showMessage(message);
        QMessageBox::warning(this, "Conversion Partially Complete",
            message + "\n\nErrors:\n" + errors.join("\n"));
    }
}

void MainWindow::updateFormatAvailability()
{
    // Update combo box items to show availability
    struct FormatInfo {
        ImageConverter::Format format;
        QString displayName;
        QString extension;
    };

    QList<FormatInfo> formats = {
        {ImageConverter::Format::JPEG, "JPEG", ".jpg"},
        {ImageConverter::Format::PNG, "PNG", ".png"},
        {ImageConverter::Format::WebP, "WebP", ".webp"},
        {ImageConverter::Format::GIF, "GIF", ".gif"},
        {ImageConverter::Format::TIFF, "TIFF", ".tiff"},
        {ImageConverter::Format::BMP, "BMP", ".bmp"},
        {ImageConverter::Format::HEIC, "HEIC", ".heic"},
        {ImageConverter::Format::AVIF, "AVIF", ".avif"},
        {ImageConverter::Format::ICO, "ICO", ".ico"}
    };

    ui->formatComboBox->clear();

    for (const FormatInfo& info : formats) {
        bool supported = ImageConverter::isFormatSupported(info.format);
        QString itemText = QString("%1 (%2)%3")
            .arg(info.displayName)
            .arg(info.extension)
            .arg(supported ? "" : " [Not Available]");
        ui->formatComboBox->addItem(itemText);

        // Disable unsupported formats in the dropdown
        if (!supported) {
            int index = ui->formatComboBox->count() - 1;
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->formatComboBox->model());
            if (model) {
                QStandardItem* item = model->item(index);
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            }
        }
    }
}

void MainWindow::setUIEnabled(bool enabled)
{
    ui->selectFilesBtn->setEnabled(enabled);
    ui->clearFilesBtn->setEnabled(enabled);
    ui->outputFolderBtn->setEnabled(enabled);
    ui->formatComboBox->setEnabled(enabled);
    ui->fileListWidget->setEnabled(enabled);
}
