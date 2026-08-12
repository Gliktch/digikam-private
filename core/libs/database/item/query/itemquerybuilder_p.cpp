/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemquerybuilder_p.h"

namespace Digikam
{

QString SubQueryBuilder::build(enum SKey key,
                               enum SOperator op,
                               const QString& passedVal,
                               QList<QVariant>* boundValues) const
{
    QString query;
    QString val = passedVal;

    if ((op == LIKE) || (op == NLIKE))
    {
        val = QLatin1Char('%') + val + QLatin1Char('%');
    }

    switch (key)
    {
        case (ALBUM):
        {
            query = QString::fromUtf8(" (Images.dirid $$##$$ ?) ");
            *boundValues << val;
            break;
        }

        case (ALBUMNAME):
        {
            query = QString::fromUtf8(" (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE url $$##$$ ?)) ");
            *boundValues << val;
            break;
        }

        case (ALBUMCAPTION):
        {
            query = QString::fromUtf8(" (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE caption $$##$$ ?)) ");
            *boundValues << val;
            break;
        }

        case (ALBUMCOLLECTION):
        {
            query = QString::fromUtf8(" (Images.dirid IN "
                    "  (SELECT id FROM Albums WHERE collection $$##$$ ?)) ");
            *boundValues << val;
            break;
        }

        case (TAG):
        {
            if      (op == EQ)
            {
                query = QString::fromUtf8(" (Images.id IN "
                        "   (SELECT imageid FROM ImageTags "
                        "    WHERE tagid = ?)) ");
                *boundValues << val.toInt();
            }
            else if (op == NE)
            {
                query = QString::fromUtf8(" (Images.id NOT IN "
                        "   (SELECT imageid FROM ImageTags "
                        "    WHERE tagid = ?)) ");
                *boundValues << val.toInt();
            }
            else if (op == LIKE)
            {
                query = QString::fromUtf8(" (Images.id IN "
                        "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                        "    WHERE TagsTree.pid = ? or ImageTags.tagid = ? )) ");
                *boundValues << val.toInt() << val.toInt();
            }
            else // op == NLIKE
            {
                query = QString::fromUtf8(" (Images.id NOT IN "
                        "   (SELECT ImageTags.imageid FROM ImageTags INNER JOIN TagsTree ON ImageTags.tagid = TagsTree.id "
                        "    WHERE TagsTree.pid = ? or ImageTags.tagid = ? )) ");
                *boundValues << val.toInt() << val.toInt();
            }

            //         query = QString::fromUtf8(" (Images.id IN "
            //                 "   (SELECT imageid FROM ImageTags "
            //                 "    WHERE tagid $$##$$ ?)) ");

            break;
        }

        case (TAGNAME):
        {
            query = QString::fromUtf8(" (Images.id IN "
                    "  (SELECT imageid FROM ImageTags "
                    "   WHERE tagid IN "
                    "   (SELECT id FROM Tags WHERE name $$##$$ ?))) ");
            *boundValues << val;
            break;
        }

        case (IMAGENAME):
        {
            query = QString::fromUtf8(" (Images.name $$##$$ ?) ");
            *boundValues << val;
            break;
        }

        case (IMAGECAPTION):
        {
            query = QString::fromUtf8(" (Images.caption $$##$$ ?) ");
            *boundValues << val;
            break;
        }

        case (IMAGEDATE):
        {
            query = QString::fromUtf8(" (Images.datetime $$##$$ ?) ");
            *boundValues << val;
            break;
        }

        case (KEYWORD):
        {
            qCWarning(DIGIKAM_DATABASE_LOG) << "KEYWORD Detected which is not possible";
            break;
        }

        case (RATING):
        {
            query = QString::fromUtf8(" (ImageProperties.value $$##$$ ? and ImageProperties.property='Rating') ");
            *boundValues << val;
            break;
        }
    }

    if (key != TAG)
    {
        switch (op)
        {
            case (EQ):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("="));
                break;
            }

            case (NE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("<>"));
                break;
            }

            case (LT):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("<"));
                break;
            }

            case (GT):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8(">"));
                break;
            }

            case (LTE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("<="));
                break;
            }

            case (GTE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8(">="));
                break;
            }

            case (LIKE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("LIKE"));
                break;
            }

            case (NLIKE):
            {
                query.replace(QString::fromUtf8("$$##$$"), QString::fromUtf8("NOT LIKE"));
                break;
            }
        }
    }

    // special case for imagedate. If the key is imagedate and the operator is EQ,
    // we need to split it into two rules

    if ((key == IMAGEDATE) && (op == EQ))
    {
        QDate date = QDate::fromString(val, Qt::ISODate);

        if (!date.isValid())
        {
            return query;
        }

        query = QString::fromUtf8(" (Images.datetime > ? AND Images.datetime < ?) ");
        *boundValues << date.addDays(-1).toString(Qt::ISODate)
                     << date.addDays( 1).toString(Qt::ISODate);
    }

    return query;
}

} // namespace Digikam
