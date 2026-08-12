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

#include "digikam_export.h"

namespace Marble
{

namespace dgml
{

// Lists all known DGML 2.0 tags
// https://edu.kde.org/marble/apis/dgml/documentation/dgmltags.html

DIGIKAM_EXPORT extern const char dgmlTag_nameSpace20[];

extern  const char dgmlTag_Available[];
extern  const char dgmlTag_Blending[];
extern  const char dgmlTag_Brush[];
extern  const char dgmlTag_Center[];
extern  const char dgmlTag_Color[];
extern  const char dgmlTag_CustomPlugin[];
extern  const char dgmlTag_Dem[];
extern  const char dgmlTag_Description[];
extern  const char dgmlTag_Discrete[];
extern  const char dgmlTag_Dgml[];
extern  const char dgmlTag_Document[];
extern  const char dgmlTag_DownloadPolicy[];
extern  const char dgmlTag_DownloadUrl[];
extern  const char dgmlTag_Filter[];
extern  const char dgmlTag_Geodata[];
extern  const char dgmlTag_Group[];
extern  const char dgmlTag_Head[];
extern  const char dgmlTag_Heading[];
extern  const char dgmlTag_Icon[];
extern  const char dgmlTag_InstallMap[];
extern  const char dgmlTag_Item[];
extern  const char dgmlTag_Layer[];
extern  const char dgmlTag_Legend[];
extern  const char dgmlTag_License[];
extern  const char dgmlTag_Map[];
extern  const char dgmlTag_Maximum[];
extern  const char dgmlTag_Minimum[];
extern  const char dgmlTag_Name[];
extern  const char dgmlTag_Palette[];
extern  const char dgmlTag_Pen[];
extern  const char dgmlTag_Projection[];
extern  const char dgmlTag_Property[];
extern  const char dgmlTag_Section[];
extern  const char dgmlTag_Settings[];
extern  const char dgmlTag_SourceDir[];
extern  const char dgmlTag_SourceFile[];
extern  const char dgmlTag_StorageLayout[];
extern  const char dgmlTag_Target[];
extern  const char dgmlTag_Text[];
extern  const char dgmlTag_Texture[];
extern  const char dgmlTag_TileSize[];
extern  const char dgmlTag_Theme[];
extern  const char dgmlTag_Value[];
extern  const char dgmlTag_Vector[];
extern  const char dgmlTag_Vectortile[];
extern  const char dgmlTag_Visible[];
extern  const char dgmlTag_Zoom[];
extern  const char dgmlTag_RenderOrder[];

} // namespace dgml

// Helper macro
#define DGML_DEFINE_TAG_HANDLER(Name) GEODATA_DEFINE_TAG_HANDLER(dgml, Dgml, Name, dgmlTag_nameSpace20)

} // namespace Marble
