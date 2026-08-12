/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-13
 * Description : Integrated testing tool for bencmark deployed model and hyper parameters
 *               for object detection
 * SPDX-FileCopyrightText: 2023 by Quoc Hung TRAN <quochungtran1999 at gmail dot com>
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
#include <QRandomGenerator>
#include <QRadioButton>
#include <QCheckBox>

// Local includes

#include "digikam_debug.h"
#include "dnnbasedetectormodel.h"
#include "dnnyolodetector.h"
#include "dcombobox.h"

using namespace Digikam;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(const QDir& directory, QWidget* const parent = nullptr);
    ~MainWindow() override;

private:

    QWidget* setupFullImageArea();
    QWidget* setupImageList(const QDir& directory);
    QWidget* setupControlPanel();

private:

    void drawObjects(QImage& img, const QHash<QString, QVector<QRect> >& detectedObjects);

private:

    QHash<QString, QVector<QRect> > detectObjects(const QString& imagePath) const;

private Q_SLOTS:

    void slotDetectObjects(const QListWidgetItem* imageItem);
    void slotApplySettings();
    void slotSettingsChanged(int type);
    void slotObjectSelected();

private:

    QLabel*             m_fullImage       = nullptr;
    QListWidget*        m_imageListView   = nullptr;
    DNNYoloDetector*    m_yoloDetector    = nullptr;
    YoloVersions        m_modelType;
    QList<QString>      m_objectSelected;

    // control pann

    QPushButton*        m_applyButton     = nullptr;
    DComboBox*          m_modelOptionsBox = nullptr;
    QList<QCheckBox*>   m_objectOptions;

Q_SIGNALS:

    void signalApplyForImage(const QListWidgetItem* imageItem);
};

MainWindow::MainWindow(const QDir &directory, QWidget* const parent)
    : QMainWindow(parent)
{
    setWindowTitle(QLatin1String("Object Detection Test"));

    m_yoloDetector                = new DNNYoloDetector(YoloVersions::YOLOV5NANO);

    // Image area

    QWidget* const imageArea      = new QWidget(this);
    QHBoxLayout* processingLayout = new QHBoxLayout(imageArea);

    processingLayout->addWidget(setupFullImageArea());
    processingLayout->addWidget(setupControlPanel());

    QSizePolicy spHigh(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spHigh.setVerticalPolicy(QSizePolicy::Expanding);
    imageArea->setSizePolicy(spHigh);

    QWidget*     const mainWidget = new QWidget(this);
    QVBoxLayout* const mainLayout = new QVBoxLayout(mainWidget);

    mainLayout->addWidget(imageArea);
    mainLayout->addWidget(setupImageList(directory));

    setCentralWidget(mainWidget);

    // set up connection

    connect(m_modelOptionsBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged(int)));

    connect(m_applyButton, SIGNAL(clicked()),
            this, SLOT(slotApplySettings()));

    for (auto opt : std::as_const(m_objectOptions))
    {
        connect(opt, SIGNAL(toggled(bool)),
                this, SLOT(slotObjectSelected()));
    }

    connect(m_imageListView, &QListWidget::currentItemChanged,
            this, &MainWindow::slotDetectObjects);

    connect(this, SIGNAL(signalApplyForImage(const QListWidgetItem*)),
            this, SLOT(slotDetectObjects(const QListWidgetItem*)));
}

MainWindow::~MainWindow()
{
    delete m_imageListView;
    delete m_applyButton;
    delete m_yoloDetector;
    delete m_modelOptionsBox;
}

QHash<QString, QVector<QRect> > MainWindow::detectObjects(const QString& imagePath) const
{
    cv::Mat cvImage = cv::imread(imagePath.toStdString());

    if (cvImage.empty())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Image Path is not available";
        return {};
    }

    return m_yoloDetector->detectObjects(cvImage);
}

void MainWindow::drawObjects(QImage& img, const QHash<QString, QVector<QRect> >& detectedObjects)
{
    if (detectedObjects.empty())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "No object detected";
        return;
    }

    QPainter painter(&img);
    QPen paintPen(Qt::red);
    paintPen.setWidth(2);
    painter.setPen(paintPen);

    // TODO Show cropped tags

    // Decide which object we want to show

    for (auto label : std::as_const(m_objectSelected))
    {
        QString cleanLabel = label.remove(QLatin1Char('&'));

        for (auto rectDraw : detectedObjects[cleanLabel])
        {
            painter.drawRect(rectDraw);
        }
    }
}

void MainWindow::slotDetectObjects(const QListWidgetItem* imageItem)
{
    QString imagePath = imageItem->text();
    qCDebug(DIGIKAM_TESTS_LOG) << "Loading" << imagePath;
    QImage img(imagePath);

    QHash<QString, QVector<QRect> > objects = detectObjects(imagePath);
    drawObjects(img, objects);

    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromLatin1("Inference Time %1").arg(m_yoloDetector->showInferenceTime());
    m_fullImage->setPixmap(QPixmap::fromImage(img));
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

QWidget* MainWindow::setupControlPanel()
{
    QWidget* const controlPanel = new QWidget();

    QSizePolicy spImage(QSizePolicy::Preferred, QSizePolicy::Expanding);
    spImage.setHorizontalStretch(2);
    controlPanel->setSizePolicy(spImage);

    m_applyButton               = new QPushButton(this);
    m_applyButton->setText(QLatin1String("Apply"));

    m_modelOptionsBox           = new DComboBox(this);
    m_modelOptionsBox->insertItem(int(YoloVersions::YOLOV5XLARGE), QLatin1String("yolov5 Xlarge"));
    m_modelOptionsBox->insertItem(int(YoloVersions::YOLOV5NANO),   QLatin1String("yolov5 nano"));
    m_modelOptionsBox->setDefaultIndex(int(YoloVersions::YOLOV5NANO));

    QScrollArea* const objectsSelectArea = new QScrollArea();
    objectsSelectArea->setWidgetResizable(true);
    objectsSelectArea->setAlignment(Qt::AlignRight);
    objectsSelectArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    objectsSelectArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget* const objectsSelectView = new QWidget();
    QVBoxLayout* const vLayout       = new QVBoxLayout(this);
    QList<QString> predClass         = m_yoloDetector->getPredefinedClasses();

    for (int i = 0 ; i < predClass.size() ; i++)
    {
        QCheckBox* checkBox = new QCheckBox(predClass[i], objectsSelectView);
        m_objectOptions.append(checkBox);
        vLayout->addWidget(checkBox);
    }

    objectsSelectView->setLayout(vLayout);
    objectsSelectArea->setWidget(objectsSelectView);

    QFormLayout* const layout = new QFormLayout(this);
    layout->insertRow(0, m_modelOptionsBox);
    layout->insertRow(1, m_applyButton);
    layout->insertRow(2, objectsSelectArea);

    controlPanel->setLayout(layout);

    return controlPanel;
}

void MainWindow::slotObjectSelected()
{
    m_objectSelected.clear();

    for (const auto opt : std::as_const(m_objectOptions))
    {
        if (opt->isChecked())
        {
            m_objectSelected.append(QString::fromLatin1("%1").arg(opt->text()));
        }
    }
}

void MainWindow::slotApplySettings()
{
    m_yoloDetector = new DNNYoloDetector(m_modelType);

    QListWidgetItem* const item = m_imageListView->currentItem();

    if (item && m_imageListView->selectedItems().contains(item))
    {
        Q_EMIT signalApplyForImage(m_imageListView->currentItem());
    }
}

void MainWindow::slotSettingsChanged(int type)
{
    m_modelType = static_cast<YoloVersions> (type);
}

QWidget* MainWindow::setupImageList(const QDir& directory)
{
    // Itemlist erea

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

    // generate list widget items

    QVector<QListWidgetItem*> imageItems;
    QFileInfoList filesInfo = directory.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (int j = 0 ; j < filesInfo.size() ; j++)
    {
        QImage img(filesInfo[j].absoluteFilePath());

        if(! img.isNull())
        {
            imageItems.append(new QListWidgetItem(QIcon(filesInfo[j].absoluteFilePath()),
                                                        filesInfo[j].absoluteFilePath()));
        }
    }

    for (auto item : imageItems)
    {
        m_imageListView->addItem(item);
    }

    // set list view into items area

    itemsArea->setWidget(m_imageListView);

    return itemsArea;
}

QCommandLineParser* parseOptions(const QCoreApplication& app)
{
    QCommandLineParser* const parser = new QCommandLineParser();
    parser->addOption(QCommandLineOption(QLatin1String("dataset"),
                                         QLatin1String("Data set folder"),
                                         QLatin1String("path relative to data folder")));
    parser->addHelpOption();
    parser->process(app);

    return parser;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("digikam"));

    // Options for commandline parser

    QCommandLineParser* const parser = parseOptions(app);

    if (!parser->isSet(QLatin1String("dataset")))
    {
        qWarning(DIGIKAM_TESTS_LOG) << "Data set is not set !!!";
    }

    QDir dataset(parser->value(QLatin1String("dataset")));

    MainWindow* const window = new MainWindow(dataset, nullptr);
    window->show();

    return app.exec();
}

#include "detect_object_gui.moc"
