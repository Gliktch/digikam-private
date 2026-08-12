/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-14
 * Description : Files downloader
 *
 * SPDX-FileCopyrightText: 2020-2024 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filesdownloader_p.h"

namespace Digikam
{

bool FilesDownloader::checkDownloadFiles() const
{
    QString path = getFacesEnginePath();

    if (path.isEmpty())
    {
        return false;
    }

    for (const DownloadInfo& info : std::as_const(d->files))
    {
        QFileInfo fileInfo(path + QLatin1Char('/') + info.name);

        if (!fileInfo.exists() || (fileInfo.size() != info.size))
        {
            return false;
        }
    }

    return true;
}

void FilesDownloader::startDownload()
{
    setWindowTitle(i18nc("@title:window", "Download Required Model Files"));
    setMinimumHeight(580);
    setMinimumWidth(650);

    QWidget* const mainWidget = new QWidget(this);
    QVBoxLayout* const vBox   = new QVBoxLayout(mainWidget);

    d->buttons                = new QDialogButtonBox(QDialogButtonBox::Help |
                                                     QDialogButtonBox::Ok   |
                                                     QDialogButtonBox::Open |
                                                     QDialogButtonBox::Close,
                                                     mainWidget);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setText(i18n("Download"));
    d->buttons->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QLatin1String("edit-download")));

    d->buttons->button(QDialogButtonBox::Open)->setToolTip(i18n("Open local directory used for the data files storage."));

    d->infoLabel        = new QLabel(mainWidget);
    d->loadLabel        = new QLabel(mainWidget);
    d->sizeLabel        = new QLabel(mainWidget);
    d->pathLabel        = new QLabel(mainWidget);
    d->pathLabel->setText(i18n("Location of the face management model files:"));

    d->infoLabel->setWordWrap(true);
    d->loadLabel->setWordWrap(true);

    d->selector         = new DFileSelector(mainWidget);
    d->selector->setFileDlgMode(QFileDialog::Directory);
    d->selector->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->selector->setFileDlgTitle(i18nc("@title:window", "Face Management Model Target"));
    d->selector->setWhatsThis(i18n("Sets the target address to download the face management model files to."));

    d->aiAutoToolsCheck = new QCheckBox(i18n("Use AI Auto-Tools features"),       mainWidget);
    d->faceEngineCheck  = new QCheckBox(i18n("Use Face Management feature"),      mainWidget);
    d->aestheticCheck   = new QCheckBox(i18n("Use Aesthetic Detection feature"),  mainWidget);
    d->autoTagsCheck    = new QCheckBox(i18n("Use AutoTags Assignment feature"),  mainWidget);

    d->progress         = new QProgressBar(mainWidget);
    d->progress->setFormat(i18nc("%p is the percent value, % is the percent sign", "%p%"));
    d->progress->setMinimum(0);
    d->progress->setMaximum(1);
    d->progress->setValue(0);

    d->nameLabel        = new QLabel(mainWidget);

    vBox->addWidget(d->infoLabel);
    vBox->addWidget(d->loadLabel);
    vBox->addWidget(d->sizeLabel);
    vBox->addStretch(1);
    vBox->addWidget(d->pathLabel);
    vBox->addWidget(d->selector);
    vBox->addStretch(1);
    vBox->addWidget(d->aiAutoToolsCheck);
    vBox->addWidget(d->faceEngineCheck);
    vBox->addWidget(d->aestheticCheck);
    vBox->addWidget(d->autoTagsCheck);
    vBox->addStretch(1);
    vBox->addWidget(d->nameLabel);
    vBox->addWidget(d->progress);
    vBox->addWidget(d->buttons);

    setLayout(vBox);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotDownload()));

    connect(d->buttons->button(QDialogButtonBox::Open), SIGNAL(clicked()),
            this, SLOT(slotOpenLocalRepo()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    connect(d->buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(reject()));

    {
        SystemSettings appSystem(qApp->applicationName());

        d->aiAutoToolsCheck->setChecked(appSystem.enableAIAutoTools);
        d->faceEngineCheck->setChecked(appSystem.enableFaceEngine);
        d->aestheticCheck->setChecked(appSystem.enableAesthetic);
        d->autoTagsCheck->setChecked(appSystem.enableAutoTags);
    }

    {
        d->selector->setFileDlgPath(getFacesEnginePath());
    }

    connect(d->selector->lineEdit(), SIGNAL(textEdited(QString)),
            this, SLOT(slotFacesEnginePathChanged()));

    connect(d->selector, SIGNAL(signalUrlSelected(QUrl)),
            this, SLOT(slotFacesEnginePathChanged()));

    connect(d->aiAutoToolsCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdateDownloadInfo()));

    connect(d->faceEngineCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdateDownloadInfo()));

    connect(d->aestheticCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdateDownloadInfo()));

    connect(d->autoTagsCheck, SIGNAL(toggled(bool)),
            this, SLOT(slotUpdateDownloadInfo()));

    slotUpdateDownloadInfo();

    dialogExec(this);
}

void FilesDownloader::download()
{
    if (!d->netMngr)
    {
        d->netMngr = new QNetworkAccessManager(this);
        d->netMngr->setRedirectPolicy(QNetworkRequest::ManualRedirectPolicy);

        connect(d->netMngr, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(slotDownloaded(QNetworkReply*)));
    }

    QString serverUrl(d->fallback ? d->fallbackUrl
                                  : d->downloadUrl);

    QUrl request(serverUrl           +
                 d->currentInfo.path +
                 d->currentInfo.name);

    d->redirects = 0;
    createRequest(request);
}

void FilesDownloader::nextDownload()
{
    QTimer::singleShot(100, this, SLOT(slotDownload()));
}

void FilesDownloader::createRequest(const QUrl& url)
{
    d->progress->setMaximum(d->currentInfo.size);
    d->progress->setValue(0);
    printDownloadInfo(url);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, d->userAgent);

    d->redirects++;
    d->reply = d->netMngr->get(request);

    connect(d->reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(slotDownloadProgress(qint64,qint64)));

    connect(d->reply, SIGNAL(sslErrors(QList<QSslError>)),
            d->reply, SLOT(ignoreSslErrors()));
}

void FilesDownloader::printDownloadInfo(const QUrl& url)
{
    QString text = QString::fromUtf8("%1 (%2://%3) (%4/%5)")
                   .arg(d->currentInfo.name)
                   .arg(url.scheme())
                   .arg(url.host())
                   .arg(d->count)
                   .arg(d->total);

    d->nameLabel->setText(text);
}

bool FilesDownloader::downloadExists(const DownloadInfo& info) const
{
    QString path = getFacesEnginePath() +
                   QString::fromLatin1("/%1").arg(info.name);

    return (!path.isEmpty() && (QFileInfo(path).size() == info.size));
}

QString FilesDownloader::getFacesEnginePath()
{
    SystemSettings system(QLatin1String("digikam"));

    return system.getFacesEnginePath();
}

void FilesDownloader::createDownloadInfo()
{
    d->files.clear();

    SystemSettings appSystem(qApp->applicationName());

    if (appSystem.enableFaceEngine)
    {
        d->files << DNNModelManager::instance()->getDownloadInformation(DNNModelUsage::DNNUsageFaceDetection);
        d->files << DNNModelManager::instance()->getDownloadInformation(DNNModelUsage::DNNUsageFaceRecognition);
    }

    if (appSystem.enableAesthetic)
    {
        d->files << DNNModelManager::instance()->getDownloadInformation(DNNModelUsage::DNNUsageAesthetics);
    }

    if (appSystem.enableAIAutoTools)
    {
        d->files << DNNModelManager::instance()->getDownloadInformation(DNNModelUsage::DNNUsageAutoRotate);
    }

    if (appSystem.enableAutoTags)
    {
        d->files << DNNModelManager::instance()->getDownloadInformation(DNNModelUsage::DNNUsageObjectDetection);
        d->files << DNNModelManager::instance()->getDownloadInformation(DNNModelUsage::DNNUsageImageClassification);
    }
}

void FilesDownloader::deleteUnusedFiles() const
{
    QString path = getFacesEnginePath();

    const QStringList obsoleteList({QLatin1String("res10_300x300_ssd_iter_140000_fp16.caffemodel"),
                                    QLatin1String("face_detection_yunet_2023mar_int8.onnx"),
                                    QLatin1String("yolov3-wider_16000.weights"),
                                    QLatin1String("yolov5n_batch_16_s320.onnx"),
                                    QLatin1String("yolov5x_batch_16_s320.onnx"),
                                    QLatin1String("openface_nn4.small2.v1.t7"),
                                    QLatin1String("shapepredictor.dat"),
                                    QLatin1String("deploy.prototxt"),
                                    QLatin1String("yolov3-face.cfg"),
                                    QLatin1String("resnet50.onnx")});

    if (!path.isEmpty())
    {
        // Get a QDir object for the directory

        QDir dir(path);

        d->nameLabel->setText(i18n("Clean up unused AI/DNN model files..."));
        d->progress->setMaximum(dir.entryInfoList(QDir::Files).size());
        d->progress->setValue(0);

        // iterate over all files in the directory and remove those that are not in the list

        const auto lst = dir.entryInfoList(QDir::Files);

        for (const QFileInfo& info : lst)
        {
            d->progress->setValue(d->progress->value() + 1);

            for (const QString& obs : std::as_const(obsoleteList))
            {
                if (info.fileName() == obs)
                {
                    QFile::remove(info.filePath());

                    break;
                }
            }

            QThread::msleep(200);
        }
    }
}

} // namespace Digikam
