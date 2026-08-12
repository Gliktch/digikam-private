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

void SetupMetadata::appendRotationTab()
{
    QWidget* const rotationPanel      = new QWidget(d->tab);
    QVBoxLayout* const rotationLayout = new QVBoxLayout;

    d->rotationGroup                       = new QGroupBox;
    QGridLayout* const rotationGroupLayout = new QGridLayout;

    QLabel* const rotationExplanation = new QLabel(i18nc("@label", "When rotating a file"));
    QLabel* const rotationIcon        = new QLabel;
    rotationIcon->setPixmap(QIcon::fromTheme(QLatin1String("transform-rotate")).pixmap(32));

    d->rotateByFlag          = new QRadioButton(i18nc("@option:radio", "Rotate by only setting a flag"));
    d->rotateByContents      = new QRadioButton(i18nc("@option:radio", "Rotate by changing the content if possible"));
    d->allowLossyRotate      = new QCheckBox(i18nc("@option:check", "Even allow lossy rotation if necessary"));
    d->allowRotateByMetadata = new QCheckBox(i18nc("@option:check", "Write flag to metadata if possible"));

    connect(d->rotateByContents, SIGNAL(toggled(bool)),
            d->allowLossyRotate, SLOT(setEnabled(bool)));

    d->rotateByFlag->setChecked(false);
    d->rotateByContents->setChecked(false);
    d->allowLossyRotate->setEnabled(false);
    d->allowLossyRotate->setChecked(false);
    d->allowRotateByMetadata->setChecked(true);

    d->rotateByFlag->setToolTip(i18nc("@info:tooltip",
                                      "Rotate files only by changing a flag, not touching the pixel data"));
    d->rotateByFlag->setWhatsThis(xi18nc("@info:whatsthis",
                                         "<para>A file can be rotated in two ways:<nl/> "
                                         "You can change the contents, rearranging the individual pixels of the image data.<nl/> "
                                         "Or you can set a flag that the file is to be rotated before it is shown.</para> "
                                         "<para>Select this option if you always want to set only a flag. "
                                         "This is less obtrusive, but requires support if the file is accessed with another software. "
                                         "Ensure to allow setting the flag in the metadata if you want to share your files "
                                         "outside digiKam.</para>"));

    d->rotateByContents->setToolTip(i18nc("@info:tooltip",
                                          "If possible rotate files by changing the pixel data"));
    d->rotateByContents->setWhatsThis(xi18nc("@info:whatsthis",
                                             "<para>A file can be rotated in two ways:<nl/> "
                                             "You can change the contents, rearranging the individual pixels of the image data.<nl/> "
                                             "Or you can set a flag that the file is to be rotated before it is shown.</para> "
                                             "<para>Select this option if you want the file to be rotated by changing the content. "
                                             "This is a lossless operation for JPEG files. For other formats it is a lossy operation, "
                                             "which you need to enable explicitly. "
                                             "It is not support for RAW and other read-only formats, "
                                             "which will be rotated by flag only.</para>"));

    d->allowLossyRotate->setToolTip(i18nc("@info:tooltip",
                                          "Rotate files by changing the pixel data even if the operation will incur quality loss"));
    d->allowLossyRotate->setWhatsThis(i18nc("@info:whatsthis",
                                            "For some file formats which apply lossy compression, "
                                            "data will be lost each time the content is rotated. "
                                            "Check this option to allow lossy rotation. "
                                            "If not enabled, these files will be rotated by flag."));

    d->allowRotateByMetadata->setToolTip(i18nc("@info:tooltip",
                                               "When rotating a file by setting a flag, also change this flag in the file's metadata"));
    d->allowRotateByMetadata->setWhatsThis(i18nc("@info:whatsthis",
                                                 "File metadata typically contains a flag describing "
                                                 "that a file shall be shown rotated. "
                                                 "Enable this option to allow editing this field. "));

    rotationGroupLayout->addWidget(rotationIcon,             0, 0, 1, 1);
    rotationGroupLayout->addWidget(rotationExplanation,      0, 1, 1, 2);
    rotationGroupLayout->addWidget(d->rotateByFlag,          1, 0, 1, 3);
    rotationGroupLayout->addWidget(d->rotateByContents,      2, 0, 1, 3);
    rotationGroupLayout->addWidget(d->allowLossyRotate,      3, 2, 1, 1);
    rotationGroupLayout->addWidget(d->allowRotateByMetadata, 4, 0, 1, 3);
    rotationGroupLayout->setColumnStretch(3, 10);

    d->rotationGroup->setLayout(rotationGroupLayout);

    // --------------------------------------------------------

    d->rotationAdvGroup                  = new QGroupBox;
    QGridLayout* const rotationAdvLayout = new QGridLayout;

    QLabel* const rotationAdvExpl        = new QLabel(i18nc("@label", "Rotate actions"));
    QLabel* const rotationAdvIcon        = new QLabel;
    rotationAdvIcon->setPixmap(QIcon::fromTheme(QLatin1String("configure")).pixmap(32));

    d->exifRotateBox                     = new QCheckBox;
    d->exifRotateBox->setText(i18nc("@option", "Show images/thumbnails &rotated according to orientation tag."));
    d->exifSetOrientationBox             = new QCheckBox;
    d->exifSetOrientationBox->setText(i18nc("@option", "Set orientation tag to normal after rotate/flip."));

    rotationAdvLayout->addWidget(rotationAdvIcon,          0, 0, 1, 1);
    rotationAdvLayout->addWidget(rotationAdvExpl,          0, 1, 1, 1);
    rotationAdvLayout->addWidget(d->exifRotateBox,         1, 0, 1, 3);
    rotationAdvLayout->addWidget(d->exifSetOrientationBox, 2, 0, 1, 3);
    rotationAdvLayout->setColumnStretch(2, 10);
    d->rotationAdvGroup->setLayout(rotationAdvLayout);

    // --------------------------------------------------------

    QLabel* const rotationNote = new QLabel(i18nc("@info", "Note: These settings affect the album view "
                                                           "and not the image editor. The image editor always "
                                                           "changes the image data during the rotation."));
    rotationNote->setWordWrap(true);
    rotationNote->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    rotationLayout->addWidget(d->rotationGroup);
    rotationLayout->addWidget(d->rotationAdvGroup);
    rotationLayout->addWidget(rotationNote);
    rotationLayout->addStretch();
    rotationPanel->setLayout(rotationLayout);

    d->tab->insertTab(Rotation, rotationPanel, i18nc("@title:tab", "Rotation"));
}

} // namespace Digikam
