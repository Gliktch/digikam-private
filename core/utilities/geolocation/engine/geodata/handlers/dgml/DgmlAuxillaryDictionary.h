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

// Lists all known DGML 2.0 auxiliary strings

// This tags are compared with the backend attribute from the
// <layer> tag in the .dgml file.
// When the backend value is parsed, it is turned
// tolower so if the values in this class are not lowercase
// it wont see they are the same

namespace Marble
{

namespace dgml
{

extern const char dgmlValue_true[];
extern const char dgmlValue_on[];
extern const char dgmlValue_texture[];
extern const char dgmlValue_vector[];
extern const char dgmlValue_vectortile[];
extern const char dgmlValue_geodata[];

} // namespace dgml

} // namespace Marble
