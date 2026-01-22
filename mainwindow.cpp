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
{
    ui->setupUi(this);

    // Connect signals
    connect(ui->selectFilesBtn, &QPushButton::clicked, this, &MainWindow::onSelectFilesClicked);
    connect(ui->clearFilesBtn, &QPushButton::clicked, this, &MainWindow::onClearFilesClicked);
    connect(ui->outputFolderBtn, &QPushButton::clicked, this, &MainWindow::onOutputFolderClicked);
    connect(ui->convertBtn, &QPushButton::clicked, this, &MainWindow::onConvertClicked);

    // Initial state
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

    // Disable UI during conversion
    ui->convertBtn->setEnabled(false);
    ui->selectFilesBtn->setEnabled(false);
    ui->statusbar->showMessage("Converting...");

    // Get target format
    ImageConverter::Format targetFormat = ImageConverter::formatFromIndex(ui->formatComboBox->currentIndex());

    // Perform conversions
    QList<ConversionResult> results;
    int total = m_selectedFiles.size();

    for (int i = 0; i < total; ++i) {
        const QString& file = m_selectedFiles[i];
        ui->statusbar->showMessage(QString("Converting %1 of %2...").arg(i + 1).arg(total));

        ConversionResult result = m_converter->convert(file, m_outputFolder, targetFormat);
        results.append(result);

        // Process events to keep UI responsive
        QApplication::processEvents();
    }

    // Re-enable UI
    ui->convertBtn->setEnabled(true);
    ui->selectFilesBtn->setEnabled(true);

    // Show results
    showConversionResults(results);
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
