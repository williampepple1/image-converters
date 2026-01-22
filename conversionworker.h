#ifndef CONVERSIONWORKER_H
#define CONVERSIONWORKER_H

#include <QObject>
#include <QThread>
#include <QStringList>
#include "imageconverter.h"

/**
 * @brief Worker class for batch image conversion in a separate thread
 */
class ConversionWorker : public QObject
{
    Q_OBJECT

public:
    explicit ConversionWorker(QObject *parent = nullptr);
    ~ConversionWorker();

    void setFiles(const QStringList& files);
    void setOutputFolder(const QString& folder);
    void setTargetFormat(ImageConverter::Format format);
    void setQuality(int quality);

public slots:
    void process();
    void cancel();

signals:
    void started();
    void progress(int current, int total, const QString& currentFile);
    void fileCompleted(const ConversionResult& result);
    void finished(const QList<ConversionResult>& results);
    void error(const QString& message);

private:
    QStringList m_files;
    QString m_outputFolder;
    ImageConverter::Format m_targetFormat;
    int m_quality;
    bool m_cancelled;
    ImageConverter* m_converter;
};

/**
 * @brief Controller class to manage the worker thread
 */
class ConversionController : public QObject
{
    Q_OBJECT

public:
    explicit ConversionController(QObject *parent = nullptr);
    ~ConversionController();

    void startConversion(const QStringList& files, const QString& outputFolder,
                         ImageConverter::Format format, int quality = -1);
    void cancelConversion();
    bool isRunning() const;

signals:
    void started();
    void progress(int current, int total, const QString& currentFile);
    void fileCompleted(const ConversionResult& result);
    void finished(const QList<ConversionResult>& results);
    void error(const QString& message);

private slots:
    void onWorkerFinished(const QList<ConversionResult>& results);

private:
    QThread* m_thread;
    ConversionWorker* m_worker;
    bool m_running;
};

#endif // CONVERSIONWORKER_H
