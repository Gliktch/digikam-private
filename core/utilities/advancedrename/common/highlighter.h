/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-19
 * Description : syntax highlighter for AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QSyntaxHighlighter>
#include <QTextDocument>

namespace Digikam
{

class Parser;

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:

    Highlighter(QTextDocument* const document,
                Parser* const _parser);
    ~Highlighter()                           override;

protected:

    void highlightBlock(const QString& text) override;

private:

    Highlighter(const Highlighter&);
    Highlighter& operator=(const Highlighter&);

    void setupHighlightingGrammar();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
