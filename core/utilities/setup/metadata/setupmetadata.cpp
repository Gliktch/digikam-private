/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2003-2004 by Ralf Holzer <ralf at well dot com>
 * SPDX-FileCopyrightText: 2003-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmetadata_p.h"

namespace Digikam
{

SetupMetadata::SetupMetadata(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->tab                          = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    // --------------------------------------------------------

    appendBehaviorTab();

    // --------------------------------------------------------

    appendSidecarsTab();

    // --------------------------------------------------------

    appendRotationTab();

    // --------------------------------------------------------

    appendDisplayTab();

    // --------------------------------------------------------

    d->exifToolView = new ExifToolConfPanel(d->tab);
    d->tab->insertTab(ExifTool, d->exifToolView, i18nc("@title:tab", "ExifTool"));

    // --------------------------------------------------------

    appendBalooTab();

    //--------------Advanced Metadata Configuration --------------

    d->advTab = new AdvancedMetadataTab(this);
    d->tab->insertTab(AdvancedConfig, d->advTab, i18nc("@title:tab advanced metadata config view", "Advanced"));

    // --------------------------------------------------------

    connect(d->exifRotateBox, SIGNAL(toggled(bool)),
            this, SLOT(slotExifAutoRotateToggled(bool)));

    connect(d->writeWithExifToolBox, SIGNAL(toggled(bool)),
            this, SLOT(slotWriteWithExifToolToggled(bool)));

    connect(d->exifToolView, SIGNAL(signalExifToolSettingsChanged(bool)),
            this, SLOT(slotExifToolSettingsChanged(bool)));

    // --------------------------------------------------------

    d->readSettings();
    slotSidecarReadWriteToggled();
    slotWriteWithExifToolToggled(d->writeWithExifToolBox->isChecked());

    // --------------------------------------------------------

    // Connect this slot later read settings.

    connect(d->writeRawFilesBox, SIGNAL(toggled(bool)),
            this, SLOT(slotWriteRawFilesToggled(bool)));

    connect(d->sidecarFileNameBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSidecarFileNameToggled(bool)));

    connect(d->readXMPSidecarBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSidecarReadWriteToggled()));

    connect(d->writeXMPSidecarBox, SIGNAL(toggled(bool)),
            this, SLOT(slotSidecarReadWriteToggled()));

    connect(d->extensionsEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotExtensionsEdited(QString)));

    // --------------------------------------------------------

    QTimer::singleShot(0, d->exifToolView, SLOT(slotStartFoundExifTool()));
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::setActiveTab(MetadataTab tab)
{
    d->tab->setCurrentIndex(tab);
}

void SetupMetadata::setActiveSubTab(MetadataSubTab tab)
{
    d->displaySubTab->setCurrentIndex(tab);
}

SetupMetadata::MetadataTab SetupMetadata::activeTab() const
{
    return (MetadataTab)d->tab->currentIndex();
}

SetupMetadata::MetadataSubTab SetupMetadata::activeSubTab() const
{
    return (MetadataSubTab)d->displaySubTab->currentIndex();
}

void SetupMetadata::applySettings()
{
    MetaEngineSettings* const mSettings = MetaEngineSettings::instance();

    if (!mSettings)
    {
        return;
    }

    MetaEngineSettingsContainer set;

    set.rotationBehavior = MetaEngineSettingsContainer::RotateByInternalFlag;

    if (d->allowRotateByMetadata->isChecked())
    {
        set.rotationBehavior |= MetaEngineSettingsContainer::RotateByMetadataFlag;
    }

    if (d->rotateByContents->isChecked())
    {
        set.rotationBehavior |= MetaEngineSettingsContainer::RotateByLosslessRotation;

        if (d->allowLossyRotate->isChecked())
        {
            set.rotationBehavior |= MetaEngineSettingsContainer::RotateByLossyRotation;
        }
    }

    set.exifRotate            = d->exifRotateBox->isChecked();
    set.exifSetOrientation    = d->exifSetOrientationBox->isChecked();

    set.saveComments          = d->saveCommentsBox->isChecked();
    set.saveDateTime          = d->saveDateTimeBox->isChecked();
    set.savePickLabel         = d->savePickLabelBox->isChecked();
    set.saveColorLabel        = d->saveColorLabelBox->isChecked();
    set.saveRating            = d->saveRatingBox->isChecked();
    set.saveTags              = d->saveTagsBox->isChecked();
    set.saveTemplate          = d->saveTemplateBox->isChecked();
    set.saveFaceTags          = d->saveFaceTags->isChecked();
    set.savePosition          = d->savePosition->isChecked();

    set.useLazySync           = d->useLazySync->isChecked();
    set.readWithExifTool      = d->readWithExifToolBox->isChecked();
    set.writeWithExifTool     = d->writeWithExifToolBox->isChecked();
    set.writeDngFiles         = d->writeDngFilesBox->isChecked();
    set.writeRawFiles         = d->writeRawFilesBox->isChecked();
    set.useXMPSidecar4Reading = d->readXMPSidecarBox->isChecked();
    set.useCompatibleFileName = d->sidecarFileNameBox->isChecked();

    if (d->writeXMPSidecarBox->isChecked())
    {
        set.metadataWritingMode = (MetaEngine::MetadataWritingMode)
                                  d->writingModeCombo->itemData(d->writingModeCombo->currentIndex()).toInt();
    }
    else
    {
        set.metadataWritingMode = MetaEngine::WRITE_TO_FILE_ONLY;
    }

    set.updateFileTimeStamp   = d->updateFileTimeStampBox->isChecked();
    set.rescanImageIfModified = d->rescanImageIfModifiedBox->isChecked();

    set.sidecarExtensions     = Setup::cleanUserFilterString(d->extensionsEdit->text(), true);
    set.sidecarExtensions.removeAll(QLatin1String("xmp"));
    set.sidecarExtensions.removeDuplicates();

    set.exifToolPath          = d->exifToolView->exifToolDirectory();

    mSettings->setSettings(set);

#ifdef HAVE_KFILEMETADATA

    ApplicationSettings* const aSettings = ApplicationSettings::instance();

    if (!aSettings)
    {
        return;
    }

    aSettings->setSyncDigikamToBaloo(d->saveToBalooBox->isChecked());
    aSettings->setSyncBalooToDigikam(d->readFromBalooBox->isChecked());

    aSettings->saveSettings();

#endif // HAVE_KFILEMETADATA

    d->tagsCfgPanel->applySettings();

    d->advTab->applySettings();
}

bool SetupMetadata::exifAutoRotateHasChanged() const
{
    return (d->exifAutoRotateOriginal != d->exifRotateBox->isChecked());
}

void SetupMetadata::slotSidecarReadWriteToggled()
{
    bool writeEnabled = d->writeXMPSidecarBox->isChecked();

    if (writeEnabled)
    {
        d->readXMPSidecarBox->setChecked(true);
    }

    bool fileBoxOn = (writeEnabled                    ||
                      d->readXMPSidecarBox->isChecked());

    d->readXMPSidecarBox->setEnabled(!writeEnabled);
    d->writingModeCombo->setEnabled(writeEnabled);
    d->sidecarFileNameBox->setEnabled(fileBoxOn);
}

void SetupMetadata::slotSidecarFileNameToggled(bool b)
{
    // Show info if sidecar file name for commercial programs was switched on, and only once.

    if (b && !d->sidecarFileNameShowedInfo)
    {
        d->sidecarFileNameShowedInfo = true;
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info",
                                       "You should only activate this option if you are exchanging "
                                       "image metadata with programs that use the sidecar format "
                                       "BASENAME.xmp. If you have images with the same base name "
                                       "(e.g. Image.CR2 and Image.JPG) then the sidecar can no "
                                       "longer be uniquely assigned and will be used for both "
                                       "images, which can lead to metadata problems."));
    }
}

void SetupMetadata::slotExifAutoRotateToggled(bool b)
{
    // Show info if rotation was switched off, and only once.

    if (!b && !d->exifAutoRotateShowedInfo && exifAutoRotateHasChanged())
    {
        d->exifAutoRotateShowedInfo = true;
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info",
                                       "Switching off exif auto rotation will most probably show "
                                       "your images in a wrong orientation, so only change this "
                                       "option if you explicitly require this."));
    }
}

void SetupMetadata::slotWriteWithExifToolToggled(bool b)
{
    d->writeDngFilesBox->setEnabled(b);
    d->writeRawFilesBox->setEnabled(b);
}

void SetupMetadata::slotWriteRawFilesToggled(bool b)
{
    // Show info if write metadata to raw files was switched on

    if (b)
    {
        QApplication::beep();

        QPointer<QMessageBox> msgBox1 = new QMessageBox(QMessageBox::Warning,
                 qApp->applicationName(),
                 i18nc("@info",
                       "Do you really want to enable metadata writing to RAW files? "
                       "digiKam delegates this task to the ExifTool backend.\n"
                       "With different RAW formats, problems are known which can "
                       "lead to the corrupt of RAW files.\n"
                       "If you decide to do so, make a backup of your RAW files. "
                       "We strongly recommend not to enable this option."),
                 QMessageBox::Yes | QMessageBox::No, this);

        msgBox1->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Yes I Understand"));
        msgBox1->setDefaultButton(QMessageBox::No);

        int result1 = msgBox1->exec();
        delete msgBox1;

        if (result1 == QMessageBox::Yes)
        {
            QPointer<QMessageBox> msgBox2 = new QMessageBox(QMessageBox::Warning,
                     qApp->applicationName(),
                     i18nc("@info", "You would rather disable writing metadata to RAW files?"),
                     QMessageBox::Yes | QMessageBox::No, this);

            int result2 = msgBox2->exec();
            delete msgBox2;

            if (result2 == QMessageBox::No)
            {
                return;
            }
        }

        d->writeRawFilesBox->setChecked(false);
    }
}

void SetupMetadata::slotExtensionsEdited(const QString& text)
{
    // Check sidecar extensions for forbidden RAW extensions

    if (!text.isEmpty())
    {
        QList<QString> rawList   = s_rawFileExtensionsdWithDesc().keys();
        QStringList extensions   = Setup::cleanUserFilterString(text, true);
        QStringList::iterator it = extensions.begin();

        for ( ; it != extensions.end() ; )
        {
            if (rawList.contains((*it).toLower()))
            {
                if (d->extensionsMsgBoxResult == QMessageBox::Help)
                {
                    QApplication::beep();

                    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                                                                   qApp->applicationName(),
                                                                   i18nc("@info",
                                                                   "A RAW file extension has been added to the list of "
                                                                   "supported sidecars. There is a risk of accidentally "
                                                                   "deleting RAW images. Are you sure you want to do that?"),
                                                                   QMessageBox::Yes | QMessageBox::No, this);

                    msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Yes I Understand"));
                    msgBox->setDefaultButton(QMessageBox::No);

                    d->extensionsMsgBoxResult = msgBox->exec();
                    delete msgBox;
                }

                if (d->extensionsMsgBoxResult == QMessageBox::No)
                {
                    it = extensions.erase(it);
                    d->extensionsEdit->setText(extensions.join(QLatin1Char(' ')));

                    continue;
                }
            }

            ++it;
        }
    }
}

} // namespace Digikam

#include "moc_setupmetadata.cpp"
