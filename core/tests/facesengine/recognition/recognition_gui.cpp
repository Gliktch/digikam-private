/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-05-20
 * Description : Testing tool for dnn face recognition of face engines
 *
 * SPDX-FileCopyrightText: 2020 by Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QCommandLineParser>
#include <QMainWindow>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>
#include <QImage>
#include <QElapsedTimer>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QLineEdit>
#include <QPushButton>
#include <QHash>
#include <QRandomGenerator>

// Local includes

#include "digikam_debug.h"
#include "opencvdnnfacedetector.h"
#include "facedetector.h"
#include "dnnopenfaceextractor.h"
#include "dnnsfaceextractor.h"
#include "opencvdnnfacerecognizer.h"
#include "identity.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"
#include "facialrecognition_wrapper.h"

using namespace Digikam;

// TODO: Recognition is incorrect where human are wearing glasses.

static QVector<QListWidgetItem*> splitData(const QDir& dataDir, float splitRatio,
                                           QHash<QString, QVector<QImage> >& trainSet,
                                           QHash<QString, QVector<QImage> >& testSet)
{

    QVector<QListWidgetItem*> imageItems;

    // Each subdirectory in data directory should match with a label.

    QFileInfoList subDirs = dataDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name);

    for (int i = 0 ; i < subDirs.size() ; ++i)
    {
        QDir subDir(subDirs[i].absoluteFilePath());

        QString label           = subDirs[i].fileName();
        QFileInfoList filesInfo = subDir.entryInfoList(QDir::Files | QDir::Readable);

        // Suffle dataset.

        QList<QFileInfo>::iterator it = filesInfo.begin();
        QList<QFileInfo>::iterator it1;

        for (int j = 0 ; j < filesInfo.size() ; ++j)
        {
            int inc = QRandomGenerator::global()->bounded(filesInfo.size());

            it1     = filesInfo.begin();
            it1    += inc;

            std::swap(*(it++), *(it1));
        }

        // Split train/test.

        for (int j = 0 ; i < filesInfo.size() ; ++j)
        {
            QImage img(filesInfo[j].absoluteFilePath());

            if (j < (filesInfo.size() * splitRatio))
            {
                if (!img.isNull())
                {
                    trainSet[label].append(img);
                    imageItems.append(new QListWidgetItem(QIcon(filesInfo[j].absoluteFilePath()), filesInfo[j].absoluteFilePath()));
                }
            }

            else
            {
                if (!img.isNull())
                {
                    testSet[label].append(img);
                    imageItems.append(new QListWidgetItem(QIcon(filesInfo[j].absoluteFilePath()), filesInfo[j].absoluteFilePath()));
                }
            }
        }
    }

    return imageItems;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(const QDir& directory, QWidget* const parent = nullptr);
    ~MainWindow() override;

private:

    QPixmap showCVMat(const cv::Mat& cvimage);
    QList<QRectF> detectFaces(const QString& imagePath);
    void extractFaces(const QImage& img, QImage& imgScaled, const QList<QRectF>& faces);

    QWidget* setupFullImageArea();
    QWidget* setupCroppedFaceArea();
    QWidget* setupPreprocessedFaceArea();
    QWidget* setupAlignedFaceArea();
    QWidget* setupControlPanel();
    QWidget* setupImageList(const QDir& directory);

private Q_SLOTS:

    void slotDetectFaces(const QListWidgetItem* imageItem);
    void slotSaveIdentity();
    void slotIdentify(int index);

private:

    OpenCVDNNFaceDetector*    m_detector            = nullptr;
    FacialRecognitionWrapper* recognitionWrapper    = nullptr;
    OpenCVDNNFaceRecognizer*  m_recognizer          = nullptr;
    DNNFaceExtractorBase*     m_extractor           = nullptr;
    QVector<cv::Mat>          m_preprocessedFaces;
    Identity                  m_currentIdenity;


    QLabel*                   m_fullImage           = nullptr;
    QListWidget*              m_imageListView       = nullptr;
    QListWidget*              m_croppedfaceList     = nullptr;
    /*
        QVBoxLayout*              m_preprocessedList    = nullptr;
    */
    QListWidget*              m_preprocessedList    = nullptr;
    QListWidget*              m_alignedList         = nullptr;

    // Control panel.

    QLineEdit*                m_imageLabel          = nullptr;
    QLabel*                   m_similarityLabel     = nullptr;
    QLabel*                   m_recognizationInfo   = nullptr;
    QPushButton*              m_applyButton         = nullptr;
};

MainWindow::MainWindow(const QDir& directory, QWidget* const parent)
    : QMainWindow(parent)
{
    DbEngineParameters prm = DbEngineParameters::parametersFromConfig();
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);

    setWindowTitle(QLatin1String("Face recognition Test"));

    recognitionWrapper     = new FacialRecognitionWrapper();

    m_detector             = new OpenCVDNNFaceDetector(DetectorNNModel::DNNDetectorYuNet);
    m_recognizer           = new OpenCVDNNFaceRecognizer(OpenCVDNNFaceRecognizer::Tree, FaceScanSettings::FaceRecognitionModel::SFace);
    m_extractor            = new DNNSFaceExtractor();

    // Image area.

    QWidget*     const imageArea        = new QWidget(this);
    QHBoxLayout*       processingLayout = new QHBoxLayout(imageArea);

    processingLayout->addWidget(setupFullImageArea());
    processingLayout->addWidget(setupCroppedFaceArea());
    processingLayout->addWidget(setupPreprocessedFaceArea());
    processingLayout->addWidget(setupAlignedFaceArea());
    processingLayout->addWidget(setupControlPanel());

    QSizePolicy spHigh(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spHigh.setVerticalPolicy(QSizePolicy::Expanding);
    imageArea->setSizePolicy(spHigh);

    QWidget*     const mainWidget = new QWidget(this);
    QVBoxLayout* const mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->addWidget(imageArea);
    mainLayout->addWidget(setupImageList(directory));

    setCentralWidget(mainWidget);
}

MainWindow::~MainWindow()
{
    delete recognitionWrapper;
    delete m_detector;
    delete m_recognizer;
    delete m_extractor;

    delete m_fullImage;
    delete m_imageListView;
    delete m_croppedfaceList;
    delete m_preprocessedList;
    delete m_alignedList;

    delete m_recognizationInfo;
    delete m_imageLabel;
    delete m_similarityLabel;
    delete m_applyButton;
}

void MainWindow::slotDetectFaces(const QListWidgetItem* imageItem)
{
    QString imagePath = imageItem->text();

    qCDebug(DIGIKAM_TESTS_LOG) << "Loading" << imagePath;

    QImage img(imagePath);
    QImage imgScaled(img.scaled(416, 416, Qt::KeepAspectRatio));

    m_imageLabel->clear();
    m_recognizationInfo->clear();

    disconnect(m_alignedList, &QListWidget::currentRowChanged,
               this, &MainWindow::slotIdentify);

    // Clear faces layout.

    QListWidgetItem* wItem = nullptr;

    while ((wItem = m_croppedfaceList->item(0)) != nullptr)
    {
        delete wItem;
    }

    while ((wItem = m_preprocessedList->item(0)) != nullptr)
    {
        delete wItem;
    }

    while ((wItem = m_alignedList->item(0)) != nullptr)
    {
        delete wItem;
    }

    QList<QRectF> faces = detectFaces(imagePath);

    extractFaces(img, imgScaled, faces);

    // Only setPixmap after finishing drawing bboxes around detected faces.

    m_fullImage->setPixmap(QPixmap::fromImage(imgScaled));

    connect(m_alignedList, &QListWidget::currentRowChanged,
            this, &MainWindow::slotIdentify);
}

QPixmap MainWindow::showCVMat(const cv::Mat& cvimage)
{
    if (cvimage.cols * cvimage.rows != 0)
    {
        cv::Mat rgb;
        QPixmap p;
        cv::cvtColor(cvimage, rgb, (-2 * cvimage.channels() + 10));
        p.convertFromImage(QImage(rgb.data, rgb.cols, rgb.rows, QImage::Format_RGB888));

        return p;
    }

    return QPixmap();
}

QList<QRectF> MainWindow::detectFaces(const QString& imagePath)
{
    QImage img(imagePath);

    QList<QRectF> faces;

    try
    {
        QElapsedTimer timer;
        unsigned int elapsedDetection = 0;
        timer.start();

        // NOTE: detection with filePath won't work when format is not standard.
        // NOTE: unexpected behaviour with detecFaces(const QString&).

        cv::Size paddedSize(0, 0);
        cv::Mat cvImage       = m_detector->prepareForDetection(img, paddedSize);
        QList<QRect> absRects = m_detector->detectFaces(cvImage, paddedSize);
        faces                 = FaceDetector::toRelativeRects(absRects,
                                                              QSize(cvImage.cols - 2 * paddedSize.width,
                                                                    cvImage.rows - 2 * paddedSize.height));
        elapsedDetection = timer.elapsed();

        qCDebug(DIGIKAM_TESTS_LOG) << "(Input CV) Found" << absRects.size()
                                   << "faces, in" << elapsedDetection << "ms";
    }

    catch (cv::Exception& e)
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "MainWindow::detectFaces: cv::Exception:" << e.what();
    }

    catch (...)
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "MainWindow::detectFaces: Default exception from OpenCV";
    }

    return faces;
}

void MainWindow::extractFaces(const QImage& img, QImage& imgScaled, const QList<QRectF>& faces)
{
    if (faces.isEmpty())
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "No face detected";
        return;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "Coordinates of detected faces:";

    for (const QRectF& r : std::as_const(faces))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << r;
    }

    QPainter painter(&imgScaled);
    QPen paintPen(Qt::green);
    paintPen.setWidth(1);
    painter.setPen(paintPen);

    m_preprocessedFaces.clear();

    for (const QRectF& rr : std::as_const(faces))
    {
        QRect rectDraw      = FaceDetector::toAbsoluteRect(rr, imgScaled.size());
        QRect rect          = FaceDetector::toAbsoluteRect(rr, img.size());
        QImage part         = img.copy(rect);

        // Show cropped faces.

        QIcon croppedFace(QPixmap::fromImage(part.scaled(qMin(img.size().width(), 100),
                                                         qMin(img.size().width(), 100),
                                                         Qt::KeepAspectRatio)));

        m_croppedfaceList->addItem(new QListWidgetItem(croppedFace, QLatin1String("")));
        painter.drawRect(rectDraw);

        // Show preprocessed faces.

        cv::Mat cvPreprocessedFace = m_recognizer->prepareForRecognition(part);
        m_preprocessedList->addItem(new QListWidgetItem(QIcon(showCVMat(cvPreprocessedFace)), QLatin1String("")));

        m_preprocessedFaces << cvPreprocessedFace;

        // Show aligned faces.

        cv::Mat cvAlignedFace = m_extractor->alignFace(cvPreprocessedFace);
        m_alignedList->addItem(new QListWidgetItem(QIcon(showCVMat(cvAlignedFace)), QLatin1String("")));
    }
}

QWidget* MainWindow::setupFullImageArea()
{
    m_fullImage = new QLabel;
    m_fullImage->setScaledContents(true);

    QSizePolicy spImage(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spImage.setHorizontalStretch(3);
    m_fullImage->setSizePolicy(spImage);

    return m_fullImage;
}

QWidget* MainWindow::setupCroppedFaceArea()
{
    QScrollArea* const facesArea = new QScrollArea(this);
    facesArea->setWidgetResizable(true);
    facesArea->setAlignment(Qt::AlignRight);
    facesArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    facesArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QSizePolicy spImage(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spImage.setHorizontalStretch(1);

    facesArea->setSizePolicy(spImage);

    m_croppedfaceList = new QListWidget(this);

    m_croppedfaceList->setViewMode(QListView::IconMode);
    m_croppedfaceList->setIconSize(QSize(200, 150));
    m_croppedfaceList->setResizeMode(QListWidget::Adjust);
    m_croppedfaceList->setFlow(QListView::TopToBottom);
    m_croppedfaceList->setWrapping(false);
    m_croppedfaceList->setDragEnabled(false);

    facesArea->setWidget(m_croppedfaceList);

    return facesArea;
}

QWidget* MainWindow::setupPreprocessedFaceArea()
{
    // Preprocessed face area.

    QScrollArea* const preprocessedFacesArea = new QScrollArea(this);
    preprocessedFacesArea->setWidgetResizable(true);
    preprocessedFacesArea->setAlignment(Qt::AlignRight);
    preprocessedFacesArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    preprocessedFacesArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QSizePolicy spImage(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spImage.setHorizontalStretch(1);

    preprocessedFacesArea->setSizePolicy(spImage);

    m_preprocessedList = new QListWidget();

    m_preprocessedList->setViewMode(QListView::IconMode);
    m_preprocessedList->setIconSize(QSize(200, 150));
    m_preprocessedList->setResizeMode(QListWidget::Adjust);
    m_preprocessedList->setFlow(QListView::TopToBottom);
    m_preprocessedList->setWrapping(false);
    m_preprocessedList->setDragEnabled(false);

    preprocessedFacesArea->setWidget(m_preprocessedList);

    return preprocessedFacesArea;
}

QWidget* MainWindow::setupAlignedFaceArea()
{
    // Aligned face area.

    QScrollArea* const alignedFacesArea = new QScrollArea(this);
    alignedFacesArea->setWidgetResizable(true);
    alignedFacesArea->setAlignment(Qt::AlignRight);
    alignedFacesArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    alignedFacesArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QSizePolicy spImage(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spImage.setHorizontalStretch(1);

    alignedFacesArea->setSizePolicy(spImage);

    m_alignedList = new QListWidget(this);

    m_alignedList->setViewMode(QListView::IconMode);
    m_alignedList->setIconSize(QSize(200, 150));
    m_alignedList->setResizeMode(QListWidget::Adjust);
    m_alignedList->setFlow(QListView::TopToBottom);
    m_alignedList->setWrapping(false);
    m_alignedList->setDragEnabled(false);

    alignedFacesArea->setWidget(m_alignedList);

    return alignedFacesArea;
}

QWidget* MainWindow::setupControlPanel()
{
    QWidget* const controlPanel = new QWidget();

    QSizePolicy spImage(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spImage.setHorizontalStretch(2);

    controlPanel->setSizePolicy(spImage);

    m_recognizationInfo       = new QLabel(this);
    m_imageLabel              = new QLineEdit(this);
    m_similarityLabel         = new QLabel(this);
    m_applyButton             = new QPushButton(this);
    m_applyButton->setText(QLatin1String("Apply"));

    QFormLayout* const layout = new QFormLayout(this);
    layout->addRow(new QLabel(QLatin1String("Recognized :")), m_recognizationInfo);
    layout->addRow(new QLabel(QLatin1String("Identity :")),  m_imageLabel);
    layout->addRow(new QLabel(QLatin1String("Similarity distance :")), m_similarityLabel);
    layout->insertRow(3, m_applyButton);

    controlPanel->setLayout(layout);

    connect(m_applyButton, &QPushButton::clicked,
            this, &MainWindow::slotSaveIdentity);

    return controlPanel;
}

QWidget* MainWindow::setupImageList(const QDir& directory)
{
    // Itemlist area.

    QScrollArea* const itemsArea = new QScrollArea;
    itemsArea->setWidgetResizable(true);
    itemsArea->setAlignment(Qt::AlignBottom);
    itemsArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    itemsArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QSizePolicy spLow(QSizePolicy::Preferred, QSizePolicy::Fixed);
    itemsArea->setSizePolicy(spLow);

    m_imageListView = new QListWidget(this);
    m_imageListView->setViewMode(QListView::IconMode);
    m_imageListView->setIconSize(QSize(200, 150));
    m_imageListView->setResizeMode(QListWidget::Adjust);
    m_imageListView->setFlow(QListView::LeftToRight);
    m_imageListView->setWrapping(false);
    m_imageListView->setDragEnabled(false);

    QHash<QString, QVector<QImage> > trainSet, testSet;

    QVector<QListWidgetItem*> items = splitData(directory, 0.5, trainSet, testSet);

    for (int i = 0 ; i < items.size() ; ++i)
    {
        m_imageListView->addItem(items[i]);
    }

    connect(m_imageListView, &QListWidget::currentItemChanged,
            this, &MainWindow::slotDetectFaces);

    itemsArea->setWidget(m_imageListView);

    return itemsArea;
}

void MainWindow::slotIdentify(int /*index*/)
{
    /*
        // TODO: fix this.
        m_currentIdenity = m_recognizer->findIdenity(m_preprocessedFaces[index]);
    */
    if (m_currentIdenity.isNull())
    {
        m_recognizationInfo->setText(QLatin1String("Cannot recognized"));
    }

    else
    {
        m_recognizationInfo->setText(QLatin1String("Recognized"));
        m_imageLabel->setText(m_currentIdenity.attribute(QLatin1String("fullName")));
    }
}

void MainWindow::slotSaveIdentity()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "assign identity" << m_imageLabel->text();
    m_currentIdenity.setAttribute(QLatin1String("fullName"), m_imageLabel->text());
    /*
        m_recognizer->saveIdentity(m_currentIdenity, false);
    */
}

QCommandLineParser* parseOptions(const QCoreApplication& app)
{
    QCommandLineParser* const parser = new QCommandLineParser();
    parser->addOption(QCommandLineOption(QLatin1String("dataset"), QLatin1String("Data set folder"), QLatin1String("path relative to data folder")));
    parser->addHelpOption();
    parser->process(app);

    return parser;
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("digikam"));

    // Options for commandline parser.

    QCommandLineParser* const parser = parseOptions(app);

    if (!parser->isSet(QLatin1String("dataset")))
    {
        qWarning(DIGIKAM_TESTS_LOG) << "Data set is not set !!!";

        return 1;
    }

    QDir dataset(parser->value(QLatin1String("dataset")));

    MainWindow* const window = new MainWindow(dataset, nullptr);
    window->show();

    return app.exec();
}

#include "recognition_gui.moc"
