#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect signals
    connect(ui->selectFilesBtn, &QPushButton::clicked, this, &MainWindow::onSelectFilesClicked);
    connect(ui->clearFilesBtn, &QPushButton::clicked, this, &MainWindow::onClearFilesClicked);
    connect(ui->outputFolderBtn, &QPushButton::clicked, this, &MainWindow::onOutputFolderClicked);
    connect(ui->convertBtn, &QPushButton::clicked, this, &MainWindow::onConvertClicked);

    // Initial state
    updateConvertButtonState();
    ui->statusbar->showMessage("Ready - Select images to convert");
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

    // Placeholder - conversion logic will be implemented in Milestone 2
    QMessageBox::information(this, "Convert",
        QString("Ready to convert %1 file(s) to %2\n\nConversion feature coming in next milestone!")
        .arg(m_selectedFiles.size())
        .arg(ui->formatComboBox->currentText()));
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
