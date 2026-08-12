/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-09-23
 * Description : assign labels metadata batch tool.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assignlabels.h"

// Qt includes

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QStyle>
#include <QLayout>
#include <QCheckBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dimg.h"
#include "dinfointerface.h"
#include "dmetadata.h"
#include "dpluginbqm.h"
#include "ratingwidget.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
#include "dfileoperations.h"
#include "dlayoutbox.h"

namespace DigikamBqmAssignLabelsPlugin
{

class Q_DECL_HIDDEN AssignLabels::Private
{
public:

    Private() = default;

public:

    QCheckBox*          setRating           = nullptr;
    RatingWidget*       ratingWidget        = nullptr;

    QCheckBox*          setColor            = nullptr;
    ColorLabelSelector* colorLabelSelector  = nullptr;

    QCheckBox*          setPick             = nullptr;
    PickLabelSelector*  pickLabelSelector   = nullptr;

    bool                changeSettings      = true;
};

AssignLabels::AssignLabels(QObject* const parent)
    : BatchTool(QLatin1String("AssignLabels"), MetadataTool, parent),
      d        (new Private)
{
}

AssignLabels::~AssignLabels()
{
    delete d;
}

BatchTool* AssignLabels::clone(QObject* const parent) const
{
    return new AssignLabels(parent);
}

void AssignLabels::registerSettingsWidget()
{
    const int spacing     = layoutSpacing();
    DVBox* const vbox     = new DVBox;

    DHBox* const pickBox  = new DHBox(vbox);
    d->setPick = new QCheckBox(i18n("Pick Label:"), pickBox);
    d->pickLabelSelector  = new PickLabelSelector(pickBox);
    pickBox->layout()->setAlignment(d->pickLabelSelector, Qt::AlignVCenter | Qt::AlignRight);

    DHBox* const colorBox = new DHBox(vbox);
    d->setColor = new QCheckBox(i18n("Color Label:"), colorBox);
    d->colorLabelSelector = new ColorLabelSelector(colorBox);
    colorBox->layout()->setAlignment(d->colorLabelSelector, Qt::AlignVCenter | Qt::AlignRight);

    DHBox* const rateBox  = new DHBox(vbox);
    d->setRating = new QCheckBox(i18n("Rating:"), rateBox);
    d->ratingWidget       = new RatingWidget(rateBox);
    rateBox->layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter | Qt::AlignRight);

    QWidget* const space  = new QWidget(vbox);
    vbox->setStretchFactor(space, 10);
    vbox->setContentsMargins(spacing, spacing, spacing, spacing);

    m_settingsWidget      = vbox;

    connect(d->setPick, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->pickLabelSelector, SIGNAL(signalPickLabelChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->setColor, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->colorLabelSelector, SIGNAL(signalColorLabelChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(d->setRating, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AssignLabels::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("SetPick"),     false);
    settings.insert(QLatin1String("PickLabel"),   (int)NoPickLabel);
    settings.insert(QLatin1String("SetColor"),    false);
    settings.insert(QLatin1String("ColorLabel"),  (int)NoColorLabel);
    settings.insert(QLatin1String("SetRating"),   false);
    settings.insert(QLatin1String("RatingValue"), (int)NoRating);

    return settings;
}

void AssignLabels::slotAssignSettings2Widget()
{
    d->changeSettings = false;

    bool setPick     = settings().value(QLatin1String("SetPick")).toBool();
    PickLabel pick   = (PickLabel)settings().value(QLatin1String("PickLabel")).toInt();
    bool setColor    = settings().value(QLatin1String("SetColor")).toBool();
    ColorLabel color = (ColorLabel)settings().value(QLatin1String("ColorLabel")).toInt();
    bool setRating   = settings().value(QLatin1String("SetRating")).toBool();
    int rating       = settings().value(QLatin1String("RatingValue")).toInt();

    d->setPick->setChecked(setPick);
    d->pickLabelSelector->setPickLabel(pick);
    d->setColor->setChecked(setColor);
    d->colorLabelSelector->setColorLabel(color);
    d->setRating->setChecked(setRating);
    d->ratingWidget->setRating(rating);

    d->changeSettings = true;
}

void AssignLabels::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        settings.insert(QLatin1String("SetPick"),     d->setPick->isChecked());
        settings.insert(QLatin1String("PickLabel"),   (int)d->pickLabelSelector->pickLabel());
        settings.insert(QLatin1String("SetColor"),    d->setColor->isChecked());
        settings.insert(QLatin1String("ColorLabel"),  (int)d->colorLabelSelector->colorLabel());
        settings.insert(QLatin1String("SetRating"),   d->setRating->isChecked());
        settings.insert(QLatin1String("RatingValue"), d->ratingWidget->rating());

        BatchTool::slotSettingsChanged(settings);
    }
}

bool AssignLabels::toolOperations()
{
    bool ret = true;
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (image().isNull())
    {
        if (!meta->load(inputUrl().toLocalFile()))
        {
            return false;
        }
    }
    else
    {
        meta->setData(image().getMetadata());
    }

    bool setPick     = settings().value(QLatin1String("SetPick")).toBool();
    PickLabel pick   = (PickLabel)settings().value(QLatin1String("PickLabel")).toInt();
    bool setColor    = settings().value(QLatin1String("SetColor")).toBool();
    ColorLabel color = (ColorLabel)settings().value(QLatin1String("ColorLabel")).toInt();
    bool setRating   = settings().value(QLatin1String("SetRating")).toBool();
    int rating       = settings().value(QLatin1String("RatingValue")).toInt();

    if (setPick)
    {
        meta->setItemPickLabel(pick);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Assign Picklabel:" << pick;
    }

    if (setColor)
    {
        meta->setItemColorLabel(color);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Assign Colorlabel:" << color;
    }

    if (setRating)
    {
        meta->setItemRating(rating);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Assign Rating:" << rating;
    }

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret &= DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (ret)
        {
            ret &= meta->save(outputUrl().toLocalFile());
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Save metadata to file:" << ret;
        }
    }
    else
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Save metadata to image";
        image().setMetadata(meta->data());
        ret &= savefromDImg();
    }

    return ret;
}

} // namespace DigikamBqmAssignLabelsPlugin

#include "moc_assignlabels.cpp"
