/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-15
 * Description : a tool to replace part of the image using another
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017      by Shaza Ismail Kaoud <shaza dot ismail dot k at gmail dot com>
 * SPDX-FileCopyrightText: 2019      by Ahmed Fathi <ahmed dot fathi dot abdelmageed at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "healingclonetool.h"

// C++ includes

#include <stack>

// Qt includes

#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QPoint>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dnuminput.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "itempropertiestxtlabel.h"
#include "healingclonetoolwidget.h"

namespace DigikamEditorHealingCloneToolPlugin
{

class Q_DECL_HIDDEN HealingCloneTool::Private
{
public:

    Private() = default;

public:

    const QString configGroupName                               = QLatin1String("Healing Clone Tool");
    const QString configRadiusAdjustmentEntry                   = QLatin1String("RadiusAdjustment");
    const QString configBlurAdjustmentEntry                     = QLatin1String("BlurAdjustment");

    const QSize                          btnSize                = QSize(50, 50);
    const QSize                          iconSize               = QSize(30, 30);

    DIntNumInput*                        radiusInput            = nullptr;
    DDoubleNumInput*                     blurPercent            = nullptr;
    HealingCloneToolWidget*              previewWidget          = nullptr;
    EditorToolSettings*                  gboxSettings           = nullptr;
    QPushButton*                         srcButton              = nullptr;
    QPushButton*                         lassoButton            = nullptr;
    QPushButton*                         moveButton             = nullptr;
    QPushButton*                         undoCloneButton        = nullptr;
    QPushButton*                         redoCloneButton        = nullptr;

    DImg                                 cloneImg;

    std::stack<DImg>                     undoStack;
    std::stack<DImg>                     redoStack;

    bool                                 resetLassoPoint        = true;
    bool                                 insideLassoOperation   = false;

    QPoint                               previousLassoPoint;
    QPoint                               startLassoPoint;

    std::vector<DColor>                  lassoColors;
    std::vector<QPoint>                  lassoPoints;
    QPolygon                             lassoPolygon;

    std::vector<std::vector<bool>>       lassoFlags;
    std::map<std::pair<int,int>, DColor> lassoColorsMap;
};

// --------------------------------------------------------

HealingCloneTool::HealingCloneTool(QObject* const parent)
    : EditorTool(parent),
      d         (new Private)
{
    setObjectName(QLatin1String("healing clone"));
    setToolHelp(QLatin1String("healingclonetool.anchor"));

    d->gboxSettings  = new EditorToolSettings(nullptr);
    d->previewWidget = new HealingCloneToolWidget;
    refreshImage();

    d->previewWidget->setFocusPolicy(Qt::StrongFocus);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::NoPreviewMode);

    // --------------------------------------------------------

    QLabel* const label = new QLabel(i18n("Brush Radius:"));
    d->radiusInput      = new DIntNumInput();
    d->radiusInput->setRange(0, 200, 1);
    d->radiusInput->setDefaultValue(50);
    d->radiusInput->setWhatsThis(i18n("A radius of 0 has no effect, "
                                      "1 and above determine the brush radius "
                                      "that determines the size of parts copied in the image. \nShortcut :: [ and ]"));
    d->radiusInput->setToolTip(i18n("A radius of 0 has no effect, "
                                    "1 and above determine the brush radius "
                                    "that determines the size of parts copied in the image. \nShortcut :: [ and ]"));

    d->previewWidget->setBrushValue(d->radiusInput->value());

    // --------------------------------------------------------

    QLabel* const label2 = new QLabel(i18n("Radial Blur Percent:"));
    d->blurPercent       = new DDoubleNumInput();
    d->blurPercent->setRange(0, 100, 0.1);
    d->blurPercent->setDefaultValue(0);
    d->blurPercent->setWhatsThis(i18n("A percent of 0 has no effect, values "
                                      "above 0 represent a factor for mixing "
                                      "the destination color with source color "
                                      "this is done radially i.e. the inner part of "
                                      "the brush radius is totally from source and mixing "
                                      "with destination is done gradually till the outer part "
                                      "of the circle."));

    // --------------------------------------------------------

    d->srcButton = new QPushButton();
    d->srcButton->setFixedSize(d->btnSize);
    d->srcButton->setIcon(QIcon::fromTheme(QLatin1String("crosshairs")));
    d->srcButton->setIconSize(d->iconSize);
    d->srcButton->setWhatsThis(i18n("Select Source Point.\nShortcut: S"));
    d->srcButton->setToolTip(i18n("Select Source Point.\nShortcut: S"));

    // --------------------------------------------------------

    d->lassoButton = new QPushButton();
    d->lassoButton->setFixedSize(d->btnSize);
    d->lassoButton->setIcon(QIcon::fromTheme(QLatin1String("edit-select-lasso")));
    d->lassoButton->setIconSize(d->iconSize);
    d->lassoButton->setWhatsThis(i18n("Polygon Selection With Lasso.\nShortcut: L\n"
                                      "To Continue polygon, either press L or double click\n"
                                      "To Cancel, press ESC"));
    d->lassoButton->setToolTip(i18n("Polygon Selection With Lasso.\nShortcut: L\n"
                                    "To Continue polygon, either press L or double click\n"
                                    "To Cancel, press ESC"));

    // --------------------------------------------------------

    d->moveButton = new QPushButton();
    d->moveButton->setFixedSize(d->btnSize);
    d->moveButton->setIcon(QIcon::fromTheme(QLatin1String("transform-browse")));
    d->moveButton->setIconSize(d->iconSize);
    d->moveButton->setWhatsThis(i18n("Move Image.\nShortcut: M"));
    d->moveButton->setToolTip(i18n("Move Image.\nShortcut: M"));

    // --------------------------------------------------------

    d->undoCloneButton = new QPushButton();
    d->undoCloneButton->setFixedSize(d->btnSize);
    d->undoCloneButton->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));
    d->undoCloneButton->setIconSize(d->iconSize);
    d->undoCloneButton->setWhatsThis(i18n("Undo clone operation.\nShortcut: CTRL+Z"));
    d->undoCloneButton->setToolTip(i18n("Undo clone operation.\nShortcut: CTRL+Z"));

    // --------------------------------------------------------

    d->redoCloneButton = new QPushButton();
    d->redoCloneButton->setFixedSize(d->btnSize);
    d->redoCloneButton->setIcon(QIcon::fromTheme(QLatin1String("edit-redo")));
    d->redoCloneButton->setIconSize(d->iconSize);
    d->redoCloneButton->setWhatsThis(i18n("Redo clone operation.\nShortcut: CTRL+Y"));
    d->redoCloneButton->setToolTip(i18n("Redo clone operation.\nShortcut: CTRL+Y"));

    // --------------------------------------------------------

    QString help = i18n("<p>How To Use:<br><br>"
                        "* Press <b>s</b> to switch to source-selection mode, and select source point on image.<br>"
                        "* Press <b>s</b> again and start cloning.<br>"
                        "* Press <b>[</b> and <b>]</b> to change brush size.<br>"
                        "* Press <b>CTRL+Mousewheel</b> to zoom in/out.<br>"
                        "* Press <b>m</b> to pan the image if image is larger than viewport.<br>"
                        "* Press <b>l</b> to start lasso mode. Start drawing lasso boundary either "
                        "continuously or discretely, then double-click or press l again to close the boundary.<br>"
                        "* Inside lasso mode, you can clone only inside the lasso region.</p>");

    DTextBrowser* const label3 = new DTextBrowser(help);
    label3->setLinesNumber(20);

    // Tool Buttons

    const int spacing              = d->gboxSettings->spacingHint();
    QGridLayout* const grid        = new QGridLayout();
    QGroupBox* const iconsGroupBox = new QGroupBox();
    QHBoxLayout* const iconsHBox   = new QHBoxLayout();
    iconsHBox->setSpacing(0);
    iconsHBox->addWidget(d->srcButton);
    iconsHBox->addWidget(d->lassoButton);
    iconsHBox->addWidget(d->moveButton);
    iconsHBox->addWidget(d->undoCloneButton);
    iconsHBox->addWidget(d->redoCloneButton);
    iconsGroupBox->setLayout(iconsHBox);

    // ---

    grid->addWidget(iconsGroupBox);
    grid->addWidget(new DLineWidget(Qt::Horizontal, d->gboxSettings->plainPage()), 3, 0, 1, 2);
    grid->addWidget(label,          4, 0, 1, 2);
    grid->addWidget(d->radiusInput, 5, 0, 1, 2);
    grid->addWidget(label2,         6, 0, 1, 2);
    grid->addWidget(d->blurPercent, 7, 0, 1, 2);
    grid->addWidget(new DLineWidget(Qt::Horizontal, d->gboxSettings->plainPage()), 8, 0, 1, 2);
    grid->addWidget(label3,         9, 0, 1, 2);
    grid->setRowStretch(10, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(grid);

    // --------------------------------------------------------

    setToolSettings(d->gboxSettings);

    // --------------------------------------------------------

    d->lassoColors.push_back(DColor(Qt::red));
    d->lassoColors.push_back(DColor(Qt::white));
    d->lassoColors.push_back(DColor(Qt::black));
    d->lassoColors.push_back(DColor(Qt::yellow));
    d->lassoColors.push_back(DColor(Qt::blue));
    d->lassoColors.push_back(DColor(Qt::yellow));

    // --------------------------------------------------------

    connect(d->radiusInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotRadiusChanged(int)));

    connect(d->srcButton, SIGNAL(clicked(bool)),
            d->previewWidget, SLOT(slotSetSourcePoint()));

    connect(d->moveButton, SIGNAL(clicked(bool)),
            d->previewWidget, SLOT(slotMoveImage()));

    connect(d->lassoButton, SIGNAL(clicked(bool)),
            d->previewWidget, SLOT(slotLassoSelect()));

    connect(d->undoCloneButton, SIGNAL(clicked(bool)),
            this, SLOT(slotUndoClone()));

    connect(d->redoCloneButton, SIGNAL(clicked(bool)),
            this, SLOT(slotRedoClone()));

    connect(d->previewWidget, SIGNAL(signalClone(QPoint,QPoint)),
            this, SLOT(slotReplace(QPoint,QPoint)));

    connect(d->previewWidget, SIGNAL(signalLasso(QPoint)),
            this, SLOT(slotLasso(QPoint)));

    connect(d->previewWidget, SIGNAL(signalResetLassoPoint()),
            this, SLOT(slotResetLassoPoints()));

    connect(d->previewWidget, SIGNAL(signalContinuePolygon()),
            this, SLOT(slotContinuePolygon()));

    connect(d->previewWidget, SIGNAL(signalIncreaseBrushRadius()),
            this, SLOT(slotIncreaseBrushRadius()));

    connect(d->previewWidget, SIGNAL(signalDecreaseBrushRadius()),
            this, SLOT(slotDecreaseBrushRadius()));

    // Undo - redo

    connect(d->previewWidget, SIGNAL(signalPushToUndoStack()),
            this, SLOT(slotPushToUndoStack()));

    connect(d->previewWidget, SIGNAL(signalUndoClone()),
            this, SLOT(slotUndoClone()));

    connect(d->previewWidget, SIGNAL(signalRedoClone()),
            this, SLOT(slotRedoClone()));

    d->cloneImg = d->previewWidget->getOriginalImage();
}

HealingCloneTool::~HealingCloneTool()
{
    delete d;
}

void HealingCloneTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->radiusInput->setValue(group.readEntry(d->configRadiusAdjustmentEntry, d->radiusInput->defaultValue()));
    d->blurPercent->setValue(group.readEntry(d->configBlurAdjustmentEntry,   d->blurPercent->defaultValue()));
}

void HealingCloneTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configRadiusAdjustmentEntry, d->radiusInput->value());
    group.writeEntry(d->configBlurAdjustmentEntry,   d->blurPercent->value());
    config->sync();
}

void HealingCloneTool::finalRendering()
{
    ImageIface iface;
    FilterAction action(QLatin1String("digikam:healingCloneTool"), 1);
    iface.setOriginal(i18n("healingClone"), action, d->cloneImg);
}

void HealingCloneTool::slotResetSettings()
{
    d->radiusInput->blockSignals(true);
    d->radiusInput->slotReset();
    d->radiusInput->blockSignals(false);
}

void HealingCloneTool::slotResized()
{
    toolView()->update();
}

void HealingCloneTool::slotReplace(const QPoint& srcPoint, const QPoint& dstPoint)
{
    clone(&d->cloneImg, srcPoint, dstPoint);
}

void HealingCloneTool::slotRadiusChanged(int r)
{
    d->previewWidget->setBrushValue(r);
}

void HealingCloneTool::clone(DImg* const img,
                             const QPoint& srcPoint,
                             const QPoint& dstPoint)
{
    double blurPercent = d->blurPercent->value() / 100;
    int    radius      = d->radiusInput->value();

    for (int i = -1 * radius ; i < radius ; ++i)
    {
        for (int j = -1 * radius ; j < radius ; ++j)
        {
            int rPercent = (i * i) + (j * j);

            if (rPercent < (radius * radius)) // Check for inside the circle
            {
                if ((srcPoint.x() + i < 0) || (srcPoint.x() + i >= (int)img->width())  ||
                    (srcPoint.y() + j < 0) || (srcPoint.y() + j >= (int)img->height()) ||
                    (dstPoint.x() + i < 0) || (dstPoint.x() + i >= (int)img->width())  ||
                    (dstPoint.y() + j < 0) || (dstPoint.y() + j >= (int)img->height()))
                {
                    continue;
                }

                DColor cSrc = img->getPixelColor(srcPoint.x() + i, srcPoint.y() + j);

                if (d->insideLassoOperation && !d->lassoPoints.empty())
                {
                    if (d->lassoFlags.at(dstPoint.x() + i).at(dstPoint.y() + j))
                    {
                        continue;
                    }

                    bool isInside = d->lassoPolygon.containsPoint(QPoint(dstPoint.x() + i,
                                                                         dstPoint.y() + j),
                                                                  Qt::OddEvenFill);

                     if (!isInside)
                     {
                            continue;
                     }

                     if (d->lassoFlags.at(srcPoint.x() + i).at(srcPoint.y() + j))
                     {
                         cSrc = d->lassoColorsMap[std::make_pair(srcPoint.x() + i,
                                                                 srcPoint.y() + j)];
                     }
                }

                double rP   = blurPercent * rPercent / (radius * radius);

                DColor cDst = img->getPixelColor(dstPoint.x() + i, dstPoint.y() + j);
                cSrc.multiply(1 - rP);
                cDst.multiply(rP);
                cSrc.blendAdd(cDst);
                img->setPixelColor(dstPoint.x() + i, dstPoint.y() + j, cSrc);
                d->previewWidget->setCloneVectorChanged(true);
            }
        }
    }

    d->previewWidget->updateImage(*img);
}

void HealingCloneTool::updateLasso(const std::vector<QPoint>& points)
{
    uint radius              = 5;
    static uint colorCounter = 0;

    for (const QPoint& p : std::as_const(points))
    {
        for (uint i = 0 ; i < radius ; ++i)
        {
            for (uint j = 0 ; j < radius ; ++j)
            {
                uint x_shifted = p.x() + i;
                uint y_shifted = p.y() + j;
                DColor c       = d->cloneImg.getPixelColor(x_shifted, y_shifted);

                d->lassoColorsMap.insert(std::make_pair(std::make_pair(x_shifted, y_shifted), c));
                d->cloneImg.setPixelColor(x_shifted, y_shifted, d->lassoColors[(colorCounter) % d->lassoColors.size()]);
                d->lassoFlags.at(x_shifted).at(y_shifted) = true;
                colorCounter++;
            }
        }
    }

    d->previewWidget->updateImage(d->cloneImg);
}

void HealingCloneTool::slotLasso(const QPoint& dst)
{
    if (d->resetLassoPoint)
    {
        d->previousLassoPoint = dst;
        d->resetLassoPoint    = false;
        d->startLassoPoint    = dst;
    }

    std::vector<QPoint> points = interpolate(d->previousLassoPoint, dst);
    d->lassoPoints.push_back(dst);
    d->previousLassoPoint      = dst;
    updateLasso(points);
    d->previewWidget->setIsLassoPointsVectorEmpty(d->lassoPoints.empty());
}

std::vector<QPoint> HealingCloneTool::interpolate(const QPoint& start, const QPoint& end)
{
    std::vector<QPoint> points;
    points.push_back(start);
    QPointF distanceVec = QPoint(end.x()-start.x(), end.y() - start.y());
    double distance     = sqrt(distanceVec.x() * distanceVec.x() + distanceVec.y() * distanceVec.y());

    // Creating a unit vector

    distanceVec.setX(distanceVec.x() / distance);
    distanceVec.setY(distanceVec.y() / distance);
    int steps           = (int) distance;

    for (int i = 0 ; i < steps ; ++i)
    {
        points.push_back(QPoint(start.x() + i * distanceVec.x(),
                                start.y() + i * distanceVec.y()));
    }

    points.push_back(end);

    return points;
}

void HealingCloneTool::removeLassoPixels()
{
    std::map<std::pair<int,int>, DColor>::iterator it;

    for (it = d->lassoColorsMap.begin() ; it != d->lassoColorsMap.end() ; ++it)
    {
        std::pair<int,int> xy = it->first;
        DColor color          = it->second;
        d->cloneImg.setPixelColor(xy.first, xy.second, color);
    }

    d->previewWidget->updateImage(d->cloneImg);
}

void HealingCloneTool::redrawLassoPixels()
{
    int colorCounter = 0;
    std::map<std::pair<int,int>, DColor>::iterator it;

    for (it = d->lassoColorsMap.begin() ; it != d->lassoColorsMap.end() ; ++it)
    {
        colorCounter++;
        DColor color          = d->lassoColors[(colorCounter) % d->lassoColors.size()];
        std::pair<int,int> xy = it->first;
        d->cloneImg.setPixelColor(xy.first, xy.second, color);
    }

    d->previewWidget->updateImage(d->cloneImg);
}

void HealingCloneTool::slotResetLassoPoints()
{
    removeLassoPixels();
    d->resetLassoPoint      = true;
    d->lassoPoints.clear();
    d->insideLassoOperation = true;
    d->lassoPolygon.clear();
    d->lassoColorsMap.clear();
    initializeLassoFlags();
    d->previewWidget->setIsLassoPointsVectorEmpty(d->lassoPoints.empty());
}

void HealingCloneTool::slotContinuePolygon()
{
    if (d->lassoPoints.empty())
    {
        return;
    }

    const QPoint& start        = d->previousLassoPoint;
    const QPoint& end          = d->startLassoPoint;
    std::vector<QPoint> points = interpolate(start, end);
    updateLasso(points);

    d->lassoPoints.push_back(start);

    QVector<QPoint> polygon;

    for (const QPoint& point : std::as_const(d->lassoPoints))
    {
        polygon.append(point);
    }

    d->lassoPolygon = QPolygon(polygon);
}

void HealingCloneTool::slotIncreaseBrushRadius()
{
    int size = d->radiusInput->value();
    d->radiusInput->setValue(size + 1);
}

void HealingCloneTool::slotDecreaseBrushRadius()
{
    int size = d->radiusInput->value();
    d->radiusInput->setValue(size - 1);
}

void HealingCloneTool::initializeLassoFlags()
{
    int w    = d->cloneImg.width();
    int h    = d->cloneImg.height();
    d->lassoFlags.resize(w);

    for (int i = 0 ; i < w ; ++i)
    {
        d->lassoFlags.at(i).resize(h);
    }

    for (int i = 0 ; i < w ; ++i)
    {
        for (int j = 0 ; j < h ; ++j)
        {
            d->lassoFlags.at(i).at(j) = false;
        }
    }
}

void HealingCloneTool::slotPushToUndoStack()
{
    d->redoStack = std::stack<DImg>();
    removeLassoPixels();
    d->undoStack.push(d->previewWidget->getOriginalImage());
    redrawLassoPixels();
}

void HealingCloneTool::slotUndoClone()
{
    if (d->undoStack.empty())
    {
        return;
    }

    removeLassoPixels();
    d->redoStack.push(d->previewWidget->getOriginalImage());
    d->cloneImg = d->undoStack.top();
    d->undoStack.pop();
    d->previewWidget->updateImage(d->cloneImg);
    redrawLassoPixels();
}

void HealingCloneTool::slotRedoClone()
{
/*
    slotResetLassoPoints();
*/
    if (d->redoStack.empty())
    {
        return;
    }

    removeLassoPixels();
    d->undoStack.push(d->previewWidget->getOriginalImage());

    d->cloneImg = d->redoStack.top();
    d->redoStack.pop();
    d->previewWidget->updateImage(d->cloneImg);
    redrawLassoPixels();
}

void HealingCloneTool::refreshImage()
{
    ImageRegionWidget* const wgt = dynamic_cast<ImageRegionWidget*>(d->previewWidget);

    if (wgt)
    {
        QRectF test                       = wgt->sceneRect();
        const ImageRegionItem* const item = dynamic_cast<ImageRegionItem*>(wgt->item());

        if (item)
        {
            int w = item->boundingRect().width();
            int h = item->boundingRect().height();

            test.setWidth(10);
            test.setHeight(10);
            wgt->fitInView(test, Qt::KeepAspectRatio);

            test.setWidth(w);
            test.setHeight(h);
            wgt->fitInView(test, Qt::KeepAspectRatio);
        }
    }
}

} // namespace DigikamEditorHealingCloneToolPlugin

#include "moc_healingclonetool.cpp"
