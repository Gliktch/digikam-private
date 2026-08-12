/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-05-13
 * Description : Testing tool for different variations in dnn face detection of face engines
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
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>
#include <QImage>
#include <QElapsedTimer>
#include <QLabel>
#include <QPen>
#include <QPainter>

// Local includes

#include "digikam_debug.h"
#include "opencvdnnfacedetector.h"
#include "facedetector.h"

using namespace Digikam;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(const QDir& directory, QWidget* const parent = nullptr);
    ~MainWindow() override;

private:

    QPixmap showCVMat(const cv::Mat& cvimage)           const;
    QList<QRectF> detectFaces(const QString& imagePath) const;
    void extractFaces(const QImage& img, QImage& imgScaled, const QList<QRectF>& faces);

    QWidget* setupFullImageArea();
    QWidget* setupPaddedImage();
    QWidget* setupCroppedFaceArea();
    QWidget* setupImageList(const QDir& directory);

private Q_SLOTS:

    void slotDetectFaces(const QListWidgetItem* imageItem);

private:

    OpenCVDNNFaceDetector* m_detector           = nullptr;

    QLabel*                m_fullImage          = nullptr;
    QLabel*                m_paddedImage        = nullptr;
    QListWidget*           m_imageListView      = nullptr;
    QListWidget*           m_croppedfaceList    = nullptr;
};

MainWindow::MainWindow(const QDir& directory, QWidget* const parent)
    : QMainWindow(parent)
{
    setWindowTitle(QLatin1String("Face detection Test"));

    m_detector                          = new OpenCVDNNFaceDetector(DetectorNNModel::DNNDetectorYuNet);

    // Image area

    QWidget* const imageArea            = new QWidget(this);

    // TODO add control panel to adjust detection hyper parameters

    QWidget* const controlPanel         = new QWidget;

    QHBoxLayout* const processingLayout = new QHBoxLayout(imageArea);
    processingLayout->addWidget(setupFullImageArea());
    processingLayout->addWidget(setupPaddedImage());
    processingLayout->addWidget(setupCroppedFaceArea());
    processingLayout->addWidget(controlPanel);

    QSizePolicy spHigh(QSizePolicy::Preferred, QSizePolicy::Expanding);
    imageArea->setSizePolicy(spHigh);

    QWidget*     const mainWidget       = new QWidget(this);
    QVBoxLayout* const mainLayout       = new QVBoxLayout(mainWidget);

    mainLayout->addWidget(imageArea);
    mainLayout->addWidget(setupImageList(directory));

    setCentralWidget(mainWidget);
}

MainWindow::~MainWindow()
{
    delete m_detector;
    delete m_fullImage;
    delete m_paddedImage;
    delete m_imageListView;
    delete m_croppedfaceList;
}

void MainWindow::slotDetectFaces(const QListWidgetItem* imageItem)
{
    QString imagePath = imageItem->text();
    qCDebug(DIGIKAM_TESTS_LOG) << "Loading " << imagePath;
    QImage img(imagePath);
    QImage imgScaled(img.scaled(416, 416, Qt::KeepAspectRatio));

    // Clear faces layout.

    QListWidgetItem* wItem = nullptr;

    while ((wItem = m_croppedfaceList->item(0)) != nullptr)
    {
        delete wItem;
    }

    QList<QRectF> faces    = detectFaces(imagePath);

    extractFaces(img, imgScaled, faces);

    // Only setPixmap after finishing drawing bboxes around detected faces.

    m_fullImage->setPixmap(QPixmap::fromImage(imgScaled));
}

QPixmap MainWindow::showCVMat(const cv::Mat& cvimage) const
{
    if ((cvimage.cols * cvimage.rows) != 0)
    {
        cv::Mat rgb;
        QPixmap p;
        cv::cvtColor(cvimage, rgb, (-2 * cvimage.channels() + 10));
        p.convertFromImage(QImage(rgb.data, rgb.cols, rgb.rows, QImage::Format_RGB888));

        return p;
    }

    return QPixmap();
}

QList<QRectF> MainWindow::detectFaces(const QString& imagePath) const
{
    QImage img(imagePath);
    /*
        cv::Mat img;
        img = cv::imread(imagePath.toStdString());

        if (!img.data )
        {
            qCDebug(DIGIKAM_TESTS_LOG) <<  "Open cv Could not open or find the image";
        }
    */
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
        elapsedDetection      = timer.elapsed();

        // Debug padded image.

        m_paddedImage->setPixmap(showCVMat(cvImage));

        qCDebug(DIGIKAM_TESTS_LOG) << "(Input CV) Found " << absRects.size() << " faces, in " << elapsedDetection << "ms";
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

    qCDebug(DIGIKAM_TESTS_LOG) << "Coordinates of detected faces : ";

    for (const QRectF& r : std::as_const(faces))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << r;
    }

    QPainter painter(&imgScaled);
    QPen paintPen(Qt::green);
    paintPen.setWidth(1);
    painter.setPen(paintPen);

    for (const QRectF& rr : std::as_const(faces))
    {
        QRect  rectDraw = FaceDetector::toAbsoluteRect(rr, imgScaled.size());
        QRect  rect     = FaceDetector::toAbsoluteRect(rr, img.size());
        QImage part     = img.copy(rect);

        // Show cropped faces.

        QIcon croppedFace(QPixmap::fromImage(part.scaled(qMin(img.size().width(), 100),
                                                         qMin(img.size().width(), 100),
                                                         Qt::KeepAspectRatio)));

        m_croppedfaceList->addItem(new QListWidgetItem(croppedFace, QLatin1String("")));
        painter.drawRect(rectDraw);
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

QWidget* MainWindow::setupPaddedImage()
{
    m_paddedImage = new QLabel;
    m_paddedImage->setScaledContents(true);

    QSizePolicy spImage(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spImage.setHorizontalStretch(3);
    m_paddedImage->setSizePolicy(spImage);

    return m_paddedImage;
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

    m_croppedfaceList            = new QListWidget(this);

    m_croppedfaceList->setViewMode(QListView::IconMode);
    m_croppedfaceList->setIconSize(QSize(200, 150));
    m_croppedfaceList->setResizeMode(QListWidget::Adjust);
    m_croppedfaceList->setFlow(QListView::TopToBottom);
    m_croppedfaceList->setWrapping(false);
    m_croppedfaceList->setDragEnabled(false);

    /*
        connect(m_croppedfaceList, &QListWidget::currentItemChanged,
                this,            &MainWindow::slotDetectFaces);
    */

    facesArea->setWidget(m_croppedfaceList);

    return facesArea;
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

    m_imageListView              = new QListWidget(this);
    m_imageListView->setViewMode(QListView::IconMode);
    m_imageListView->setIconSize(QSize(200, 150));
    m_imageListView->setResizeMode(QListWidget::Adjust);
    m_imageListView->setFlow(QListView::LeftToRight);
    m_imageListView->setWrapping(false);
    m_imageListView->setDragEnabled(false);

    QStringList subjects         = directory.entryList(QDir::Files | QDir::NoDotAndDotDot);

    for (QStringList::const_iterator iter  = subjects.cbegin();
         iter != subjects.cend();
         ++iter)
    {
        QString filePath            = directory.filePath(*iter);
        QListWidgetItem* const item = new QListWidgetItem(QIcon(filePath), filePath);

        m_imageListView->addItem(item);
    }

    connect(m_imageListView, &QListWidget::currentItemChanged,
            this, &MainWindow::slotDetectFaces);

    itemsArea->setWidget(m_imageListView);

    return itemsArea;
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
    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.

    // Options for commandline parser.

    QCommandLineParser* const parser = parseOptions(app);

    if (! parser->isSet(QLatin1String("dataset")))
    {
        qWarning("Data set is not set !!!");

        return 1;
    }

    QDir dataset(parser->value(QLatin1String("dataset")));

    MainWindow* const window = new MainWindow(dataset, nullptr);
    window->show();

    return app.exec();
}

#include "benchmark_dnndetection_gui.moc"
