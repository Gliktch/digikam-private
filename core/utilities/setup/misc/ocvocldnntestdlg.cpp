/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2025-04-26
 * Description : Test OpenCV/OpenCL for DNN usage
 *
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 * SPDX-FileCopyrightText: 2025-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2025-2026 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ocvocldnntestdlg.h"

// C++ includes

#include <exception>

// Qt includes

#include <QtConcurrentRun>
#include <QThread>
#include <QFutureWatcher>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QTextBrowser>
#include <QTextCursor>
#include <QFileInfo>
#include <QStandardPaths>
#include <QImage>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_opencv.h"
#include "digikam_globals.h"
#include "dnnmodelmanager.h"
#include "dnnmodelsface.h"
#include "dnnmodelyunet.h"
#include "actionthreadbase.h"

namespace Digikam
{

class Q_DECL_HIDDEN OpenCVOpenCLDNNTestDlg::Private
{
public:

    Private() = default;

public:

    QWidget*                page                = nullptr;
    QGridLayout*            layoutGrid          = nullptr;
    QLabel*                 message             = nullptr;
    QLabel*                 icon                = nullptr;
    QTextBrowser*           progressText        = nullptr;

    QDialogButtonBox*       buttons             = nullptr;

    bool                    tested              = false;
    QFutureWatcher<bool>    testingWatcher;
};

OpenCVOpenCLDNNTestDlg::OpenCVOpenCLDNNTestDlg(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "AI GPU Test"));

    QString messageString = i18nc("@info",
                                  "This will check if your system is able to use GPU acceleration\n"
                                  "when running the AI models by validating OpenCV and OpenCL have the\n"
                                  "required libraries and drivers installed.\n"
                                  "\n\nDo you want to test your system now?");

    d->page             = new QWidget(this);
    d->layoutGrid       = new QGridLayout(d->page);

    d->icon             = new QLabel(d->page);
    d->icon->setPixmap(QIcon::fromTheme(QLatin1String("edit-image-face-show")).pixmap(QSize(64, 64)));

    d->message          = new QLabel(messageString, d->page);
    d->message->setWordWrap(true);

    d->progressText     = new QTextBrowser(d->page);
    d->progressText->setVisible(false);  // Initially hidden until test starts

    d->buttons          = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button start the AI GPU test", "Start"));
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@action:button start the AI GPU test", "Begin test"));

    d->layoutGrid->addWidget(d->icon,          0, 0, 1, 1);
    d->layoutGrid->addWidget(d->message,       0, 1, 1, 3);
    d->layoutGrid->addWidget(d->buttons,       1, 1, 1, 3);
    d->layoutGrid->addWidget(d->progressText,  2, 1, 1, 3);
    d->layoutGrid->setRowStretch(2, 10);
    d->layoutGrid->setColumnStretch(2, 10);

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotStart()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(slotCancel()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    // Connect the notification signal to its slot for progress updates

    connect(this, SIGNAL(signalNotification(QString)),
            this, SLOT(slotNotification(QString)));

    setLayout(d->layoutGrid);

    adjustSize();
}

OpenCVOpenCLDNNTestDlg::~OpenCVOpenCLDNNTestDlg()
{
    delete d;
}

void OpenCVOpenCLDNNTestDlg::slotStart()
{
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    d->buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button start the AI GPU test", "Testing..."));
    d->buttons->button(QDialogButtonBox::Cancel)->setEnabled(false);

    // Clear and show the progress text area

    d->progressText->clear();
    d->progressText->setVisible(true);
    d->progressText->append(i18n("Starting OpenCV OpenCL DNN test..."));
    adjustSize();

    // run the test in a separate thread

    d->testingWatcher.setFuture(QtConcurrent::run(
                                QThreadPool::globalInstance(),

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                &OpenCVOpenCLDNNTestDlg::runTest,
                                this

#else

                                this,
                                &OpenCVOpenCLDNNTestDlg::runTest

#endif

                            ));

    connect(&d->testingWatcher, &QFutureWatcher<bool>::finished,
            this, [this]()
            {
                // Get result directly from the watcher when it's finished

                slotTestFinished(d->testingWatcher.result());
            }
    );

    disconnect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
               this, SLOT(slotStart()));

    disconnect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
               this, SLOT(slotCancel()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotClose()));
}

void OpenCVOpenCLDNNTestDlg::slotCancel()
{
    reject();
}

void OpenCVOpenCLDNNTestDlg::slotClose()
{
    if (d->tested)
    {
        accept();
    }
    else
    {
        reject();
    }
}

void OpenCVOpenCLDNNTestDlg::slotTestFinished(bool result)
{
    d->tested = result;

    // Show test results in the progress text

    QString resultMessage;

    if (d->tested)
    {
        resultMessage = i18n("Test completed successfully! Your system can use GPU acceleration for the AI models.");
    }
    else
    {
        resultMessage = i18n("Test failed. Your system may not be able to use GPU acceleration for the AI models.\n\n"
                             "Note for the NVidia GPU users: please uses the \"Help\" button to read "
                             "the online documentation and configure you device to be detectable and "
                             "usable at digiKam run-time.");
    }

    d->progressText->append(QLatin1String("\n") + resultMessage);

    d->buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button start the AI GPU test", "Close"));
    d->buttons->button(QDialogButtonBox::Ok)->setToolTip(i18nc("@action:button start the AI GPU test", "Close the test dialog"));
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void OpenCVOpenCLDNNTestDlg::slotNotification(const QString& message)
{
    // Append to progress text instead of replacing the message

    d->progressText->append(message);

    // Auto-scroll to the bottom

    QTextCursor cursor = d->progressText->textCursor();
    cursor.movePosition(QTextCursor::End);
    d->progressText->setTextCursor(cursor);
}

bool OpenCVOpenCLDNNTestDlg::runTest()
{
    ActionThreadBase::setCurrentThreadName(QString::fromLatin1("OpenCLDNN::%1").arg(QLatin1String(__FUNCTION__)));       // To customize thread name

    bool result = false;

    try
    {
        // turn on OpenCL

        Q_EMIT signalNotification(i18n("Checking for OpenCL GPU acceleration compatibility with AI models..."));

        cv::ocl::setUseOpenCL(true);

        if (cv::ocl::haveOpenCL())
        {
            Q_EMIT signalNotification(i18n("OpenCL is available on this system."));

            cv::ocl::Device device = cv::ocl::Device::getDefault();

            if (
                !device.empty()         &&
                device.available()      &&
                device.imageSupport()   &&
                (cv::ocl::Device::TYPE_CPU != device.type())
               )
            {
                Q_EMIT signalNotification(i18n("OpenCL acceleration device is available."));

                Q_EMIT signalNotification(i18n("Using device: %1", QString::fromStdString(device.name())));
            }
            else
            {
                throw std::runtime_error(i18n("OpenCL acceleration device is not available.").toStdString());
            }
        }
        else
        {
            throw std::runtime_error(i18n("OpenCL is not available on this system.").toStdString());
        }

        // load the test file as QImage

        Q_EMIT signalNotification(i18n("Loading DNN Model test image..."));

        const DNNModelBase* const testImage = DNNModelManager::instance()->getModel(
                                                                                    QLatin1String("DNNTestImage"),
                                                                                    DNNModelUsage::DNNUsageFaceDetection
                                                                                   );

        if (!QFileInfo::exists(testImage->getModelPath()))
        {
            throw std::runtime_error(i18n("DNN model test image not found.").toStdString());
        }

        QImage inputImage(testImage->getModelPath());

        if (inputImage.isNull())
        {
            throw std::runtime_error(i18n("Failed to load DNN model test image from %1",
                                     testImage->getModelPath()).toStdString());
        }

        Q_EMIT signalNotification(i18n("Preparing the DNN Model image for processing..."));

        if (inputImage.format() != QImage::Format_BGR888)
        {
            inputImage = inputImage.convertToFormat(QImage::Format_BGR888);
        }

        // create a cv::UMat image from the QImage

        cv::Mat cvImage  = cv::Mat(
                                   inputImage.height(),
                                   inputImage.width(),
                                   CV_8UC3,
                                   inputImage.scanLine(0),
                                   inputImage.bytesPerLine()
                                  );

        cv::UMat cvUImage = cvImage.getUMat(cv::ACCESS_READ);

        cv::UMat borderFace;

        cv::copyMakeBorder(cvUImage, borderFace,
                           60, 60,
                           60, 60,
                           cv::BORDER_CONSTANT,
                           cv::Scalar(0, 0, 0));

        // find the face

        Q_EMIT signalNotification(i18n("Initializing face detection model..."));

        DNNModelYuNet* detectorModel = static_cast<DNNModelYuNet*>(
                                                                   DNNModelManager::instance()->getModel(QLatin1String("YuNet"),
                                                                   DNNModelUsage::DNNUsageFaceDetection)
                                                                  );

        Q_EMIT signalNotification(i18n("Running face detection..."));

        QMutexLocker detectorLock(&detectorModel->mutex);

        // Detect face using YuNet to get landmarks.

        cv::Mat faceLandmark;

        detectorModel->getNet()->setInputSize(borderFace.size());
        detectorModel->getNet()->setScoreThreshold(detectorModel->getThreshold(7));
        detectorModel->getNet()->detect(borderFace, faceLandmark);

        detectorLock.unlock();

        // extract face features

        Q_EMIT signalNotification(i18n("Initializing feature extraction model..."));

        DNNModelSFace* model = static_cast<DNNModelSFace*>(
                                                           DNNModelManager::instance()->getModel(QLatin1String("SFace"),
                                                           DNNModelUsage::DNNUsageFaceRecognition)
                                                          );

        Q_EMIT signalNotification(i18n("Preparing face for feature extraction..."));

        cv::UMat alignedFace;

        QMutexLocker extractorLock(&model->mutex);

        model->getNet()->alignCrop(borderFace, faceLandmark, alignedFace);

        borderFace.release();
        faceLandmark.release();

        cv::UMat uface_descriptors;

        Q_EMIT signalNotification(i18n("Extracting face features..."));

        model->getNet()->feature(alignedFace, uface_descriptors);

        if (0 == uface_descriptors.rows)
        {
            throw std::runtime_error(i18n("Failed to extract face features.").toStdString());
        }

        // everything passed successfully

        Q_EMIT signalNotification(i18n("All tests completed successfully!"));

        result = true;
    }
    catch (const cv::Exception& e)
    {
        Q_EMIT signalNotification(i18n("OpenCV error: %1", QString::fromLatin1(e.what())));

        result = false;
    }
    catch (const std::exception& e)
    {
        Q_EMIT signalNotification(i18n("Error: %1", QString::fromLatin1(e.what())));

        result = false;
    }

    return result;
}

void OpenCVOpenCLDNNTestDlg::slotHelp()
{
    openOnlineDocumentation(QLatin1String("setup_application"),
                            QLatin1String("miscs_settings"),
                            QLatin1String("system_settings"));
}

} // namespace Digikam

#include "moc_ocvocldnntestdlg.cpp"
