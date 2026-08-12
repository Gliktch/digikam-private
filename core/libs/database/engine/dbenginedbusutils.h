/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : Database engine hint data containers for Dbus
 *
 * SPDX-FileCopyrightText: 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

class QDBusArgument;

#define DECLARE_METATYPE_FOR_DBUS(x)                                                \
Q_DECLARE_METATYPE(x)                                                               \
                                                                                    \
inline QDBusArgument& operator<<(QDBusArgument& argument, const x& changeset)       \
{                                                                                   \
    changeset >> argument;                                                          \
    return argument;                                                                \
}                                                                                   \
                                                                                    \
inline const QDBusArgument& operator>>(const QDBusArgument& argument, x& changeset) \
{                                                                                   \
    changeset << argument;                                                          \
    return argument;                                                                \
}
