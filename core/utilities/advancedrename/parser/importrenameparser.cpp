/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : a parser for the AdvancedRename utility used for importing images,
 *               excluding the database options
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importrenameparser.h"

// Local includes

#include "databaseoption.h"
#include "cameranameoption.h"
#include "metadataoption.h"

namespace Digikam
{

ImportRenameParser::ImportRenameParser(QWidget* const widget)
    : Parser(widget)
{
    // unregister options that are not suitable while import

    RulesList oplist = options();

    for (Rule* const option : std::as_const(oplist))
    {
        if (dynamic_cast<DatabaseOption*>(option))
        {
            unregisterOption(option);
        }
    }
}

} // namespace Digikam
