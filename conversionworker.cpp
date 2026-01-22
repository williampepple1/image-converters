#include "conversionworker.h"

#include <QFileInfo>

// ConversionWorker implementation
ConversionWorker::ConversionWorker(QObject *parent)
    : QObject(parent)
    , m_targetFormat(ImageConverter::Format::PNG)
    , m_quality(-1)
    , m_cancelled(false)
    , m_converter(new ImageConverter(this))
{
}

ConversionWorker::~ConversionWorker()
{
}

void ConversionWorker::setFiles(const QStringList& files)
{
    m_files = files;
}

void ConversionWorker::setOutputFolder(const QString& folder)
{
    m_outputFolder = folder;
}

void ConversionWorker::setTargetFormat(ImageConverter::Format format)
{
    m_targetFormat = format;
}

void ConversionWorker::setQuality(int quality)
{
    m_quality = quality;
}

void ConversionWorker::process()
{
    m_cancelled = false;
    emit started();

    QList<ConversionResult> results;
    int total = m_files.size();

    for (int i = 0; i < total && !m_cancelled; ++i) {
        const QString& file = m_files[i];
        QFileInfo info(file);

        emit progress(i + 1, total, info.fileName());

        ConversionResult result = m_converter->convert(file, m_outputFolder, m_targetFormat, m_quality);
        results.append(result);

        emit fileCompleted(result);
    }

    if (m_cancelled) {
        emit error("Conversion cancelled by user");
    }

    emit finished(results);
}

void ConversionWorker::cancel()
{
    m_cancelled = true;
}

// ConversionController implementation
ConversionController::ConversionController(QObject *parent)
    : QObject(parent)
    , m_thread(nullptr)
    , m_worker(nullptr)
    , m_running(false)
{
}

ConversionController::~ConversionController()
{
    if (m_running) {
        cancelConversion();
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
    }
}

void ConversionController::startConversion(const QStringList& files, const QString& outputFolder,
                                            ImageConverter::Format format, int quality)
{
    if (m_running) {
        emit error("Conversion already in progress");
        return;
    }

    // Create thread and worker
    m_thread = new QThread();
    m_worker = new ConversionWorker();
    m_worker->moveToThread(m_thread);

    // Set conversion parameters
    m_worker->setFiles(files);
    m_worker->setOutputFolder(outputFolder);
    m_worker->setTargetFormat(format);
    m_worker->setQuality(quality);

    // Connect signals
    connect(m_thread, &QThread::started, m_worker, &ConversionWorker::process);
    connect(m_worker, &ConversionWorker::started, this, &ConversionController::started);
    connect(m_worker, &ConversionWorker::progress, this, &ConversionController::progress);
    connect(m_worker, &ConversionWorker::fileCompleted, this, &ConversionController::fileCompleted);
    connect(m_worker, &ConversionWorker::finished, this, &ConversionController::onWorkerFinished);
    connect(m_worker, &ConversionWorker::error, this, &ConversionController::error);

    // Cleanup on finish
    connect(m_worker, &ConversionWorker::finished, m_thread, &QThread::quit);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    m_running = true;
    m_thread->start();
}

void ConversionController::cancelConversion()
{
    if (m_worker) {
        m_worker->cancel();
    }
}

bool ConversionController::isRunning() const
{
    return m_running;
}

void ConversionController::onWorkerFinished(const QList<ConversionResult>& results)
{
    m_running = false;
    m_thread = nullptr;
    m_worker = nullptr;
    emit finished(results);
}
