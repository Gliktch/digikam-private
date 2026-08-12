/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02-02-2012
 * Description : Face database interface to train identities.
 *
 * SPDX-FileCopyrightText: 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facedb_p.h"

namespace Digikam
{

int FaceDb::addIdentity() const
{
    QVariant id;
    d->db->execSql(QLatin1String("INSERT INTO Identities (`type`) VALUES (0);"),
                   nullptr, &id);

    return id.toInt();
}

void FaceDb::updateIdentity(const Identity& p)
{
    d->db->execSql(QLatin1String("DELETE FROM IdentityAttributes WHERE id=?;"),
                   p.id());

    const QMultiMap<QString, QString> map = p.attributesMap();
    QMultiMap<QString, QString>::const_iterator it;

    for (it = map.constBegin() ; it != map.constEnd() ; ++it)
    {
        d->db->execSql(QLatin1String("INSERT INTO IdentityAttributes (id, attribute, `value`) VALUES (?, ?,?);"),
                       p.id(), it.key(), it.value());
    }
}

void FaceDb::deleteIdentity(int id)
{
    // Triggers do the rest.

    d->db->execSql(QLatin1String("DELETE FROM Identities WHERE id=?;"), id);
}

void FaceDb::deleteIdentity(const QString& uuid)
{
    QList<QVariant> ids;
    d->db->execSql(QLatin1String("SELECT Identities.id FROM Identities LEFT JOIN IdentityAttributes ON "
                                 " Identities.id=IdentityAttributes.id WHERE "
                                 " IdentityAttributes.attribute='uuid' AND IdentityAttributes.value=?;"),
                   uuid, &ids);

    if (ids.size() == 1)
    {
        deleteIdentity(ids.first().toInt());
    }

    else
    {
        qCWarning(DIGIKAM_FACEDB_LOG) << "Cannot delete identity with uuid "
                                      << uuid << ". There are " << ids.size()
                                      << " identities with this uuid.";
    }
}

void FaceDb::clearIdentities()
{
    d->db->execSql(QLatin1String("DELETE FROM Identities;"));
}

Identity FaceDb::identity(int id) const
{
    QList<QVariant> values;
    Identity p;
    p.setId(id);
    d->db->execSql(QLatin1String("SELECT attribute, `value` FROM IdentityAttributes WHERE id=?;"),
                   p.id(), &values);

    for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ;)
    {
        QString attribute = it->toString();
        ++it;
        QString value     = it->toString();
        ++it;

        p.setAttribute(attribute, value);
    }

    return p;
}

QList<Identity> FaceDb::identities() const
{
    QList<QVariant> ids;
    QList<Identity> results;
    d->db->execSql(QLatin1String("SELECT id FROM Identities;"),
                   &ids);

    for (const QVariant& v : std::as_const(ids))
    {
        QList<QVariant> values;
        Identity p;
        p.setId(v.toInt());
        d->db->execSql(QLatin1String("SELECT attribute, `value` FROM IdentityAttributes WHERE id=?;"),
                       p.id(), &values);

        for (QList<QVariant>::const_iterator it = values.constBegin() ; it != values.constEnd() ;)
        {
            QString attribute = it->toString();
            ++it;
            QString value     = it->toString();
            ++it;

            p.setAttribute(attribute, value);
        }

        results << p;
    }

    return results;
}

QList<int> FaceDb::identityIds() const
{
    QList<QVariant> ids;
    d->db->execSql(QLatin1String("SELECT id FROM Identities;"),
                   &ids);

    QList<int> results;

    for (const QVariant& var : std::as_const(ids))
    {
        results << var.toInt();
    }

    return results;
}

int FaceDb::getNumberOfIdentities() const
{
    QList<QVariant> values;

    d->db->execSql(QString::fromUtf8("SELECT COUNT(id) FROM Identities;"),
                   &values);

    if (values.isEmpty())
    {
        return 0;
    }

    return values.first().toInt();
}

} // namespace Digikam
