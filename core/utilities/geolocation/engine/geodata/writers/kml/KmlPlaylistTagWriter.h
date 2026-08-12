/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "GeoTagWriter.h"
#include "GeoDataTourControl.h"

namespace Marble
{

class GeoDataSoundCue;
class GeoDataWait;

class KmlPlaylistTagWriter : public GeoTagWriter
{
public:

    bool write(const GeoNode* node, GeoWriter& writer) const override;

private:

    static void writeTourPrimitive(const GeoDataTourPrimitive* primitive, GeoWriter& writer);
    static void writeTourControl(const GeoDataTourControl& tourControl, GeoWriter& writer);
    static void writeWait(const GeoDataWait& wait, GeoWriter& writer);
    static void writeSoundCue(const GeoDataSoundCue& cue, GeoWriter& writer);
    static QString playModeToString(GeoDataTourControl::PlayMode playMode);
};

} // namespace Marble
