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

void SetupMetadata::Private::readSettings()
{
    MetaEngineSettings* const mSettings = MetaEngineSettings::instance();

    if (!mSettings)
    {
        return;
    }

    MetaEngineSettingsContainer set = mSettings->settings();

    if (set.rotationBehavior & MetaEngineSettingsContainer::RotatingPixels)
    {
        rotateByContents->setChecked(true);
    }
    else
    {
        rotateByFlag->setChecked(true);
    }

    allowRotateByMetadata->setChecked(set.rotationBehavior & MetaEngineSettingsContainer::RotateByMetadataFlag);
    allowLossyRotate->setChecked(set.rotationBehavior & MetaEngineSettingsContainer::RotateByLossyRotation);

    exifAutoRotateOriginal       = set.exifRotate;
    exifRotateBox->setChecked(exifAutoRotateOriginal);
    exifSetOrientationBox->setChecked(set.exifSetOrientation);

    saveTagsBox->setChecked(set.saveTags);
    saveCommentsBox->setChecked(set.saveComments);
    saveRatingBox->setChecked(set.saveRating);
    savePickLabelBox->setChecked(set.savePickLabel);
    saveColorLabelBox->setChecked(set.saveColorLabel);
    saveDateTimeBox->setChecked(set.saveDateTime);
    saveTemplateBox->setChecked(set.saveTemplate);
    saveFaceTags->setChecked(set.saveFaceTags);
    savePosition->setChecked(set.savePosition);

    readWithExifToolBox->setChecked(set.readWithExifTool);
    writeWithExifToolBox->setChecked(set.writeWithExifTool);
    writeDngFilesBox->setChecked(set.writeDngFiles);
    writeRawFilesBox->setChecked(set.writeRawFiles);

    useLazySync->setChecked(set.useLazySync);
    readXMPSidecarBox->setChecked(set.useXMPSidecar4Reading);
    sidecarFileNameBox->setChecked(set.useCompatibleFileName);
    updateFileTimeStampBox->setChecked(set.updateFileTimeStamp);
    rescanImageIfModifiedBox->setChecked(set.rescanImageIfModified);

    if (set.metadataWritingMode == MetaEngine::WRITE_TO_FILE_ONLY)
    {
        writeXMPSidecarBox->setChecked(false);
    }
    else
    {
        writeXMPSidecarBox->setChecked(true);
        writingModeCombo->setCurrentIndex(writingModeCombo->findData(set.metadataWritingMode));
    }

    extensionsEdit->setText(set.sidecarExtensions.join(QLatin1Char(' ')));

    exifToolView->setExifToolDirectory(set.exifToolPath);

#ifdef HAVE_KFILEMETADATA

    ApplicationSettings* const aSettings = ApplicationSettings::instance();

    if (!aSettings)
    {
        return;
    }

    saveToBalooBox->setChecked(aSettings->getSyncDigikamToBaloo());
    readFromBalooBox->setChecked(aSettings->getSyncBalooToDigikam());

#endif // HAVE_KFILEMETADATA

}

} // namespace Digikam
