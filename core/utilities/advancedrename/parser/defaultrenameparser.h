/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the default parser for the AdvancedRename utility,
 *               includes all renaming options
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "parser.h"

namespace Digikam
{

class DefaultRenameParser : public Parser
{

public:

    explicit DefaultRenameParser(QWidget* const widget)
        : Parser(widget)
    {
    };

private:

    /// @note disabled
    DefaultRenameParser(const DefaultRenameParser&)            = delete;
    DefaultRenameParser& operator=(const DefaultRenameParser&) = delete;
};

} // namespace Digikam
