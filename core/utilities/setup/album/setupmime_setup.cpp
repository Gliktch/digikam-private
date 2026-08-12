/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmime_p.h"

namespace Digikam
{

void SetupMime::setupMime()
{
    const int spacing         = layoutSpacing();

    QVBoxLayout* const layout = new QVBoxLayout(d->panel);

    // --------------------------------------------------------

    QLabel* const explanationLabel = new QLabel;
    explanationLabel->setText(i18n("<p>Add new file types to show as album items.</p>"
                                   "<p>digiKam attempts to support all of the image formats that digital cameras produce, "
                                   "while being able to handle a few other important video and audio formats.</p> "
                                   "<p>You can add to the already-appreciable list of formats that digiKam handles by "
                                   "adding the extension of the type you want to add. "
                                   "Multiple extensions need to be separated by a semicolon or space.</p>"
                                   "<p><b><u>Note:</u> changes done in this view will perform "
                                   "a database rescan in the background.</b></p>"));
    explanationLabel->setWordWrap(true);

    // --------------------------------------------------------

    QGroupBox* const imageFileFilterBox = new QGroupBox(i18n("Image Files"), d->panel);
    QGridLayout* const grid1            = new QGridLayout(imageFileFilterBox);
    QLabel* const logoLabel1            = new QLabel(imageFileFilterBox);
    logoLabel1->setPixmap(QIcon::fromTheme(QLatin1String("image-jpeg")).pixmap(48));

    d->imageFileFilterLabel = new QLabel(imageFileFilterBox);
    d->imageFileFilterLabel->setText(i18n("Additional &image file extensions (<a href='image'>Currently-supported types</a>):"));

    DHBox* const hbox1      = new DHBox(imageFileFilterBox);
    d->imageFileFilterEdit  = new QLineEdit(hbox1);
    d->imageFileFilterEdit->setWhatsThis(i18n("<p>Here you can add the extensions of image files (including RAW files) "
                                              "to be displayed in the Album view. Just put \"xyz abc\" "
                                              "to display files with the xyz and abc extensions in your Album view.</p>"
                                              "<p>You can also remove file formats that are shown by default "
                                              "by putting a minus sign in front of the extension: e.g. \"-gif\" would remove all GIF files "
                                              "from your Album view and any trace of them in your database. "
                                              "They would not be deleted, just not shown in digiKam.</p>"
                                              "<p><b>Warning:</b> Removing files from the database means losing "
                                              "all of their tags and ratings.</p>"));
    d->imageFileFilterEdit->setClearButtonEnabled(true);
    d->imageFileFilterEdit->setPlaceholderText(i18n("Enter additional image file extensions."));
    d->imageFileFilterLabel->setBuddy(d->imageFileFilterEdit);
    hbox1->setStretchFactor(d->imageFileFilterEdit, 10);

    grid1->addWidget(logoLabel1,              0, 0, 2, 1);
    grid1->addWidget(d->imageFileFilterLabel, 0, 1, 1, 1);
    grid1->addWidget(hbox1,                   1, 1, 1, 1);
    grid1->setColumnStretch(1, 10);
    grid1->setSpacing(spacing);

    // --------------------------------------------------------

    QGroupBox* const movieFileFilterBox = new QGroupBox(i18n("Video Files"), d->panel);
    QGridLayout* const grid2            = new QGridLayout(movieFileFilterBox);

    QLabel* const logoLabel2 = new QLabel(movieFileFilterBox);
    logoLabel2->setPixmap(QIcon::fromTheme(QLatin1String("video-x-matroska")).pixmap(48));

    d->movieFileFilterLabel  = new QLabel(movieFileFilterBox);
    d->movieFileFilterLabel->setText(i18n("Additional &video file extensions (<a href='video'>Currently-supported types</a>):"));

    DHBox* const hbox2       = new DHBox(movieFileFilterBox);
    d->movieFileFilterEdit   = new QLineEdit(hbox2);
    d->movieFileFilterEdit->setWhatsThis(i18n("<p>Here you can add extra extensions of video files "
                                              "to be displayed in your Album view. Just write \"xyz abc\" "
                                              "to support files with the *.xyz and *.abc extensions. "
                                              "Clicking on these files will "
                                              "play them in an embedded video player.</p>"
                                              "<p>You can also remove file formats that are supported by default "
                                              "by putting a minus sign in front of the extension: e.g. \"-avi\" would remove "
                                              "all AVI files from your Album view and any trace of them in your database. "
                                              "They would not be deleted, just not shown in digiKam.</p>"
                                              "<p><b>Warning:</b> Removing files from the database means losing "
                                              "all of their tags and ratings.</p>"));
    d->movieFileFilterEdit->setClearButtonEnabled(true);
    d->movieFileFilterEdit->setPlaceholderText(i18n("Enter additional video file extensions."));
    d->movieFileFilterLabel->setBuddy(d->movieFileFilterEdit);
    hbox2->setStretchFactor(d->movieFileFilterEdit, 10);

    grid2->addWidget(logoLabel2,                0, 0, 2, 1);
    grid2->addWidget(d->movieFileFilterLabel,   0, 1, 1, 1);
    grid2->addWidget(hbox2,                     1, 1, 1, 1);
    grid2->setColumnStretch(1, 10);
    grid2->setSpacing(spacing);

    // --------------------------------------------------------

    QGroupBox* const audioFileFilterBox = new QGroupBox(i18n("Audio Files"), d->panel);
    QGridLayout* const grid3            = new QGridLayout(audioFileFilterBox);

    QLabel* const logoLabel3 = new QLabel(audioFileFilterBox);
    logoLabel3->setPixmap(QIcon::fromTheme(QLatin1String("audio-x-mpeg")).pixmap(48));

    d->audioFileFilterLabel  = new QLabel(audioFileFilterBox);
    d->audioFileFilterLabel->setText(i18n("Additional &audio file extensions (<a href='audio'>Currently-supported types</a>):"));

    DHBox* const hbox3       = new DHBox(audioFileFilterBox);
    d->audioFileFilterEdit   = new QLineEdit(hbox3);
    d->audioFileFilterEdit->setWhatsThis(i18n("<p>Here you can add extra extensions of audio files "
                                              "to be displayed in your Album view. Just write \"xyz abc\" "
                                              "to support files with the *.xyz and *.abc extensions. "
                                              "Clicking on these files will "
                                              "play them in an embedded audio player.</p>"
                                              "<p>You can also remove file formats that are supported by default "
                                              "by putting a minus sign in front of the extension: e.g. \"-ogg\" would "
                                              "remove all OGG files from your Album view and any trace of them in your database. "
                                              "They would not be deleted, just not shown in digiKam.</p>"
                                              "<p><b>Warning:</b> Removing files from the database means losing "
                                              "all of their tags and ratings.</p>"));
    d->audioFileFilterEdit->setClearButtonEnabled(true);
    d->audioFileFilterEdit->setPlaceholderText(i18n("Enter additional audio file extensions."));
    d->audioFileFilterLabel->setBuddy(d->audioFileFilterEdit);
    hbox3->setStretchFactor(d->audioFileFilterEdit, 10);

    grid3->addWidget(logoLabel3,              0, 0, 2, 1);
    grid3->addWidget(d->audioFileFilterLabel, 0, 1, 1, 1);
    grid3->addWidget(hbox3,                   1, 1, 1, 1);
    grid3->setColumnStretch(1, 10);
    grid3->setSpacing(spacing);

    // --------------------------------------------------------

    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);
    layout->addWidget(explanationLabel);
    layout->addWidget(imageFileFilterBox);
    layout->addWidget(movieFileFilterBox);
    layout->addWidget(audioFileFilterBox);
    layout->addStretch();

    // --------------------------------------------------------

    connect(d->imageFileFilterLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentImageSettings()));

    connect(d->movieFileFilterLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentMovieSettings()));

    connect(d->audioFileFilterLabel, SIGNAL(linkActivated(QString)),
            this, SLOT(slotShowCurrentAudioSettings()));

    // --------------------------------------------------------

    readSettings();
}

} // namespace Digikam
