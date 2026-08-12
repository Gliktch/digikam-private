/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-05-15
 * Description : a dialog to see preview ICC color correction
 *               before to apply color profile.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "iccprofile.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class DImg;
class IccProfile;
class IccTransform;

class DIGIKAM_EXPORT ColorCorrectionDlg : public QDialog
{
    Q_OBJECT

public:

    enum Mode
    {
        ProfileMismatch,
        MissingProfile,
        UncalibratedColor
    };

public:

    explicit ColorCorrectionDlg(Mode mode,
                                const DImg& preview,
                                const QString& file,
                                QWidget* const parent = nullptr);
    ~ColorCorrectionDlg()                                 override;

    ICCSettingsContainer::Behavior behavior()        const;
    IccProfile specifiedProfile()                    const;

private Q_SLOTS:

    void slotWorkspaceProfInfo();
    void slotImageProfInfo();

    void imageProfileToggled(bool);
    void imageProfileChanged();
    void missingProfileToggled(bool);
    void missingProfileChanged();
    void usedProfileToggled(bool);
    void usedProfileChanged();
    void inputProfileChanged();
    void slotHelp();
    void slotOk();

private:

    ICCSettingsContainer::Behavior currentBehavior() const;

    QLayout* createHeading()                         const;
    QLayout* createProfilesInfo()                    const;
    QLayout* createPreviews()                        const;
    QWidget* createAssumeOptions()                   const;
    QWidget* createOptions()                         const;

    void updateInfo();
    void updateUsedProfileUI();
    void updateImageProfileUI();

    void readSettings();
    void setSettings();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
