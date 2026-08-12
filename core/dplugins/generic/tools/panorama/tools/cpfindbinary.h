/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetects cpfind binary program and version
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QRegularExpression>

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CPFindBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit CPFindBinary(QObject* const parent = nullptr);
    ~CPFindBinary() override = default;

protected:

    bool parseHeader(const QString& output) override;

private:

    QRegularExpression headerRegExp;
};

} // namespace DigikamGenericPanoramaPlugin
