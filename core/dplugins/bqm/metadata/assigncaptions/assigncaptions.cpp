/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-09-24
 * Description : assign captions metadata batch tool.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "assigncaptions.h"

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
#include "dfileoperations.h"
#include "dinfointerface.h"
#include "dmetadata.h"
#include "dpluginbqm.h"
#include "altlangstredit.h"
#include "dexpanderbox.h"
#include "dlayoutbox.h"
#include "captionedit.h"

namespace DigikamBqmAssignCaptionsPlugin
{

class Q_DECL_HIDDEN AssignCaptions::Private
{
public:

    Private() = default;

public:

    QCheckBox*          setTitles       = nullptr;
    AltLangStrEdit*     titlesWidget    = nullptr;

    QCheckBox*          setCaptions     = nullptr;
    CaptionEdit*        captionsWidget  = nullptr;

    QCheckBox*          cleanupCB       = nullptr;

    DLineWidget*        lineWidget      = nullptr;
    QLabel*             infoLabel       = nullptr;

    bool                changeSettings  = true;
};

AssignCaptions::AssignCaptions(QObject* const parent)
    : BatchTool(QLatin1String("AssignCaptions"), MetadataTool, parent),
      d        (new Private)
{
}

AssignCaptions::~AssignCaptions()
{
    delete d;
}

BatchTool* AssignCaptions::clone(QObject* const parent) const
{
    return new AssignCaptions(parent);
}

void AssignCaptions::registerSettingsWidget()
{
    const int spacing     = layoutSpacing();


    DVBox* const vbox     = new DVBox;
    vbox->setContentsMargins(spacing, spacing, spacing, spacing);

    d->titlesWidget       = new AltLangStrEdit(vbox, 0);
    d->setTitles          = new QCheckBox(i18n("Titles:"), d->titlesWidget);
    d->setTitles->setChecked(false);
    d->titlesWidget->setPlaceholderText(i18nc("@info", "Enter title text here."));
    d->titlesWidget->setTitleWidget(d->setTitles);
    d->titlesWidget->slotEnabledInternalWidgets(false);

    d->captionsWidget     = new CaptionEdit(vbox);

    d->setCaptions        = new QCheckBox(i18n("Captions:"), d->captionsWidget);
    d->setCaptions->setChecked(false);
    d->captionsWidget->setPlaceholderText(i18nc("@info", "Enter caption text here."));
    d->captionsWidget->altLangStrEdit()->setTitleWidget(d->setCaptions);
    d->captionsWidget->altLangStrEdit()->slotEnabledInternalWidgets(false);
    d->captionsWidget->authorEdit()->setEnabled(false);

    d->cleanupCB          = new QCheckBox(i18n("Clean up and overwrite metadata"), vbox);
    d->cleanupCB->setToolTip(i18nc("@info", "If you turn on this options, titles and captions "
                                   "will be cleaned and replaced by the new values,\nelse "
                                   "old values will be merged with new values."));

    d->lineWidget         = new DLineWidget(Qt::Horizontal, vbox),

    d->infoLabel          = new QLabel(i18nc("@label", "Use the variable $ORGTEXT to insert the "
                                                       "existing text into the new text."), vbox);
    d->infoLabel->setWordWrap(true);

    m_settingsWidget      = vbox;

    connect(d->setTitles, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->setTitles, SIGNAL(toggled(bool)),
            d->titlesWidget, SLOT(slotEnabledInternalWidgets(bool)));

    connect(d->titlesWidget, SIGNAL(signalChanged(QString,QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->titlesWidget, SIGNAL(signalValueAdded(QString,QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->titlesWidget, SIGNAL(signalValueDeleted(QString)),
            this, SLOT(slotSettingsChanged()));

    connect(d->setCaptions, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    connect(d->setCaptions, SIGNAL(toggled(bool)),
            d->captionsWidget->altLangStrEdit(), SLOT(slotEnabledInternalWidgets(bool)));

    connect(d->setCaptions, SIGNAL(toggled(bool)),
            d->captionsWidget->authorEdit(), SLOT(setEnabled(bool)));

    connect(d->captionsWidget, SIGNAL(signalModified()),
            this, SLOT(slotSettingsChanged()));

    connect(d->cleanupCB, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AssignCaptions::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert(QLatin1String("SetTitles"),     false);
    settings.insert(QLatin1String("TitleValues"),   QString());
    settings.insert(QLatin1String("SetCaptions"),   false);
    settings.insert(QLatin1String("CaptionValues"), QString());
    settings.insert(QLatin1String("CleanUp"),       false);

    return settings;
}

void AssignCaptions::slotAssignSettings2Widget()
{
    d->changeSettings             = false;

    bool setTitles                = settings().value(QLatin1String("SetTitles")).toBool();
    MetaEngine::AltLangMap titles = altLangMapFromString(settings().value(QLatin1String("TitleValues")).toString());
    bool setCaptions              = settings().value(QLatin1String("SetCaptions")).toBool();
    CaptionsMap captions          = captionsMapFromString(settings().value(QLatin1String("CaptionValues")).toString());
    bool cleanup                  = settings().value(QLatin1String("CleanUp")).toBool();

    d->setTitles->setChecked(setTitles);
    d->setCaptions->setChecked(setCaptions);

    if (d->titlesWidget->values() != titles)
    {
        d->titlesWidget->setValues(titles);
    }

    if (d->captionsWidget->values() != captions)
    {
        d->captionsWidget->setValues(captions);
    }

    d->cleanupCB->setChecked(cleanup);

    d->changeSettings             = true;
}

void AssignCaptions::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;

        settings.insert(QLatin1String("SetTitles"),     d->setTitles->isChecked());
        settings.insert(QLatin1String("TitleValues"),   altLangMapToString(d->titlesWidget->values()));
        settings.insert(QLatin1String("SetCaptions"),   d->setCaptions->isChecked());
        settings.insert(QLatin1String("CaptionValues"), captionsMapToString(d->captionsWidget->values()));
        settings.insert(QLatin1String("CleanUp"),       d->cleanupCB->isChecked());

        BatchTool::slotSettingsChanged(settings);
    }
}

QString AssignCaptions::captionsMapToString(const CaptionsMap& cmap) const
{
    QString str;

    for (CaptionsMap::const_iterator it = cmap.constBegin() ;
         it != cmap.constEnd() ; ++it)
    {
        if (it != cmap.constBegin())
        {
            str.append(QLatin1String("||"));
        }

        str.append(QString::fromUtf8("%1||%2||%3||%4")
                   .arg(it.key()).arg((*it).caption)
                   .arg((*it).author).arg((*it).date.toString(Qt::ISODate)));
    }

    return str;
}

CaptionsMap AssignCaptions::captionsMapFromString(const QString& str) const
{
    CaptionsMap cmap;
    QStringList entryList = str.split(QLatin1String("||"),
                                      Qt::KeepEmptyParts);

    if ((entryList.size() % 4) == 0)
    {
        for (int i = 0 ; i < entryList.size() ; i += 4)
        {
            CaptionValues val;
            val.caption = entryList.at(i + 1);
            val.author  = entryList.at(i + 2);
            val.date    = QDateTime::fromString(entryList.at(i + 3),
                                                Qt::ISODate);
            cmap.insert(entryList.at(i), val);
        }
    }

    return cmap;
}

QString AssignCaptions::altLangMapToString(const MetaEngine::AltLangMap& amap) const
{
    QString str;

    for (MetaEngine::AltLangMap::const_iterator it = amap.constBegin() ;
         it != amap.constEnd() ; ++it)
    {
        if (it != amap.constBegin())
        {
            str.append(QLatin1String("||"));
        }

        str.append(QString::fromUtf8("%1||%2")
                   .arg(it.key()).arg(it.value()));
    }

    return str;
}

MetaEngine::AltLangMap AssignCaptions::altLangMapFromString(const QString& str) const
{
    MetaEngine::AltLangMap amap;
    QStringList entryList = str.split(QLatin1String("||"),
                                      Qt::KeepEmptyParts);

    if ((entryList.size() % 2) == 0)
    {
        for (int i = 0 ; i < entryList.size() ; i += 2)
        {
            amap.insert(entryList.at(i), entryList.at(i + 1));
        }
    }

    return amap;
}

bool AssignCaptions::toolOperations()
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

    bool setTitles                = settings().value(QLatin1String("SetTitles")).toBool();
    MetaEngine::AltLangMap titles = altLangMapFromString(settings().value(QLatin1String("TitleValues")).toString());
    bool setCaptions              = settings().value(QLatin1String("SetCaptions")).toBool();
    CaptionsMap captions          = captionsMapFromString(settings().value(QLatin1String("CaptionValues")).toString());
    bool cleanup                  = settings().value(QLatin1String("CleanUp")).toBool();
    const QString variableOrgText(QLatin1String("$ORGTEXT"));

    if (setTitles)
    {
        CaptionsMap orgTitlesMap;

        if (!cleanup)
        {
            orgTitlesMap = meta->getItemTitles();
        }

        MetaEngine::AltLangMap orgTitles = orgTitlesMap.toAltLangMap();
        MetaEngine::AltLangMap::const_iterator it;

        for (it = titles.constBegin() ; it != titles.constEnd() ; ++it)
        {
            QString value = it.value();
            value.replace(variableOrgText,
                          orgTitles.value(it.key()));
            orgTitles.insert(it.key(), value);
        }

        CaptionsMap newTitlesMap;
        newTitlesMap.fromAltLangMap(orgTitles);
        meta->setItemTitles(newTitlesMap);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Assign Titles:" << orgTitles;
    }

    if (setCaptions)
    {
        CaptionsMap orgCaptionsMap;

        if (!cleanup)
        {
            orgCaptionsMap = meta->getItemComments();
        }

        CaptionsMap::const_iterator it;

        for (it = captions.constBegin() ; it != captions.constEnd() ; ++it)
        {
            CaptionValues value = it.value();
            value.caption.replace(variableOrgText,
                                  orgCaptionsMap.value(it.key()).caption);
            value.author.replace(variableOrgText,
                                 orgCaptionsMap.value(it.key()).author);
            orgCaptionsMap.insert(it.key(), value);
        }

        meta->setItemComments(orgCaptionsMap);
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Assign Captions:" << orgCaptionsMap;
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

} // namespace DigikamBqmAssignCaptionsPlugin

#include "moc_assigncaptions.cpp"
