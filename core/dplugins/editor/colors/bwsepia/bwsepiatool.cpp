/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bwsepiatool.h"

// Qt includes

#include <QGridLayout>
#include <QToolTip>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dnuminput.h"
#include "colorgradientwidget.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "curvesbox.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "bwsepiafilter.h"
#include "bwsepiasettings.h"

namespace DigikamEditorBWSepiaToolPlugin
{

class Q_DECL_HIDDEN BWSepiaTool::Private
{

public:

    Private() = default;

public:

    int                  histoSegments          = 0;

    const QString configGroupName               = QLatin1String("convertbw Tool");
    const QString configHistogramChannelEntry   = QLatin1String("Histogram Channel");
    const QString configHistogramScaleEntry     = QLatin1String("Histogram Scale");

    BWSepiaSettings*     bwsepiaSettings;

    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};

// -----------------------------------------------------------------------------------

BWSepiaTool::BWSepiaTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("convertbw"));
    setToolHelp(QLatin1String("blackandwhitetool.anchor"));
    setInitPreview(true);

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs);

    ImageIface iface;
    d->histoSegments   = iface.original()->sixteenBit() ? 65535 : 255;
    d->bwsepiaSettings = new BWSepiaSettings(d->gboxSettings->plainPage(), iface.original());

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->bwsepiaSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->bwsepiaSettings->curvesBox(), SIGNAL(signalPickerChanged(int)),
            this, SLOT(slotPickerColorButtonActived(int)));

    connect(d->previewWidget, SIGNAL(signalSpotPositionChangedFromOriginal(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromOriginal(Digikam::DColor,QPoint)));

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            this, SLOT(slotSpotColorChanged(Digikam::DColor)));
}

BWSepiaTool::~BWSepiaTool()
{
    delete d;
}

void BWSepiaTool::slotPickerColorButtonActived(int type)
{
    if (type == CurvesBox::NoPicker)
    {
        return;
    }

    d->previewWidget->setCapturePointMode(true);
}

void BWSepiaTool::slotSpotColorChanged(const DColor& color)
{
    int maxColor = qMax(qMax(color.red(), color.green()), color.blue());

    switch (d->bwsepiaSettings->curvesBox()->picker())
    {
        case CurvesBox::BlackTonal:
        {
            // Black tonal curves point.

            int segment = 42 * d->histoSegments / 256;
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(LuminosityChannel,  1, QPoint(maxColor,      segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(RedChannel,         1, QPoint(color.red(),   segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(GreenChannel,       1, QPoint(color.green(), segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(BlueChannel,        1, QPoint(color.blue(),  segment));
            d->bwsepiaSettings->curvesBox()->resetPickers();
            break;
        }

        case CurvesBox::GrayTonal:
        {
            // Gray tonal curves point.

            int segment = 128 * d->histoSegments / 256;
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(LuminosityChannel,  8, QPoint(maxColor,      segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(RedChannel,         8, QPoint(color.red(),   segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(GreenChannel,       8, QPoint(color.green(), segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(BlueChannel,        8, QPoint(color.blue(),  segment));
            d->bwsepiaSettings->curvesBox()->resetPickers();
            break;
        }

        case CurvesBox::WhiteTonal:
        {
            // White tonal curves point.

            int segment = 213 * d->histoSegments / 256;
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(LuminosityChannel, 15, QPoint(maxColor,      segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(RedChannel,        15, QPoint(color.red(),   segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(GreenChannel,      15, QPoint(color.green(), segment));
            d->bwsepiaSettings->curvesBox()->curves()->setCurvePoint(BlueChannel,       15, QPoint(color.blue(),  segment));
            d->bwsepiaSettings->curvesBox()->resetPickers();
            break;
        }

        default:
        {
            d->bwsepiaSettings->curvesBox()->setCurveGuide(color);
            return;
        }
    }

    // Calculate Red, green, blue curves.

    for (int i = LuminosityChannel ; i <= BlueChannel ; ++i)
    {
        d->bwsepiaSettings->curvesBox()->curves()->curvesCalculateCurve(i);
    }

    d->bwsepiaSettings->curvesBox()->repaint();
    d->bwsepiaSettings->curvesBox()->resetPickers();

    d->previewWidget->setCapturePointMode(false);
    slotPreview();
}

void BWSepiaTool::slotColorSelectedFromOriginal(const DColor& color, const QPoint& pos)
{
    QString colorHex     = QString::fromLatin1("#%1%2%3")
        .arg(color.red(),   2, 16, QLatin1Char('0'))
        .arg(color.green(), 2, 16, QLatin1Char('0'))
        .arg(color.blue(),  2, 16, QLatin1Char('0'));

    QString tooltipText  = QString::fromUtf8(
        "<table>"
        "  <tr>"
        "    <td bgcolor='%1' width='40' height='40' style='border:1px solid black;'></td>"
        "    <td><font color='white'>Pos: (%2, %3)<br>RGB: (%4, %5, %6)</font></td>"
        "  </tr>"
        "</table>"
        ).arg(colorHex)
         .arg(pos.x()).arg(pos.y())
         .arg(color.red()).arg(color.green()).arg(color.blue());

    QPoint globalPos     = QCursor::pos();
    QToolTip::showText(globalPos, tooltipText, d->previewWidget);

    d->bwsepiaSettings->curvesBox()->setCurveGuide(color);
}

void BWSepiaTool::slotInit()
{
    EditorToolThreaded::slotInit();
    d->bwsepiaSettings->startPreviewFilters();
}

void BWSepiaTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);


    // we need to call the set methods here, otherwise the curve will not be updated correctly

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->bwsepiaSettings->readSettings(group);
    slotScaleChanged();
}

void BWSepiaTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    d->bwsepiaSettings->writeSettings(group);

    group.sync();
}

void BWSepiaTool::slotResetSettings()
{
    d->bwsepiaSettings->resetToDefault();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotPreview();
}

void BWSepiaTool::preparePreview()
{
    BWSepiaContainer settings = d->bwsepiaSettings->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new BWSepiaFilter(&preview, this, settings));
}

void BWSepiaTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void BWSepiaTool::prepareFinal()
{
    BWSepiaContainer settings = d->bwsepiaSettings->settings();

    ImageIface iface;
    setFilter(new BWSepiaFilter(iface.original(), this, settings));
}

void BWSepiaTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Convert to Black and White"),
                      filter()->filterAction(),
                      filter()->getTargetImage());
}

void BWSepiaTool::slotLoadSettings()
{
    d->bwsepiaSettings->loadSettings();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotPreview();
}

void BWSepiaTool::slotSaveAsSettings()
{
    d->bwsepiaSettings->saveAsSettings();
}

void BWSepiaTool::slotScaleChanged()
{
    d->bwsepiaSettings->setScaleType(d->gboxSettings->histogramBox()->scale());
}

} // namespace DigikamEditorBWSepiaToolPlugin

#include "moc_bwsepiatool.cpp"
