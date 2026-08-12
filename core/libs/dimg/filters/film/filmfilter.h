/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : film color negative inverter filter
 *
 * SPDX-FileCopyrightText: 2012 by Matthias Welwarsky <matthias at welwarsky dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QList>
#include <QListWidgetItem>
#include <QSharedPointer>

// Local includes

#include "dimgthreadedfilter.h"
#include "levelsfilter.h"
#include "cbfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT FilmContainer
{
public:

    enum CNFilmProfile
    {
        CNNeutral = 0,
        CNKodakGold100,
        CNKodakGold200,
        CNKodakEktar100,
        CNKodakProfessionalPortra160NC,
        CNKodakProfessionalPortra160VC,
        CNKodakProfessionalPortra400NC,
        CNKodakProfessionalPortra400VC,
        CNKodakProfessionalPortra800Box,
        CNKodakProfessionalPortra800P1,
        CNKodakProfessionalPortra800P2,
        CNKodakProfessionalNewPortra160,
        CNKodakProfessionalNewPortra400,
        CNKodakFarbwelt100,
        CNKodakFarbwelt200,
        CNKodakFarbwelt400,
        CNKodakRoyalGold400,
        CNAgfaphotoVistaPlus200,
        CNAgfaphotoVistaPlus400,
        CNFujicolorPro160S,
        CNFujicolorPro160C,
        CNFujicolorNPL160,
        CNFujicolorPro400H,
        CNFujicolorPro800Z,
        CNFujicolorSuperiaReala,
        CNFujicolorSuperia100,
        CNFujicolorSuperia200,
        CNFujicolorSuperiaXtra400,
        CNFujicolorSuperiaXtra800,
        CNFujicolorTrueDefinition400,
        CNFujicolorSuperia1600
    };

public:

    class ListItem : public QListWidgetItem
    {
    public:

        explicit ListItem(const QString& text, QListWidget* const parent, CNFilmProfile type)
            : QListWidgetItem(text, parent, (int)QListWidgetItem::UserType + (int)type)
        {
        }

    private:

        Q_DISABLE_COPY(ListItem)
    };

public:

    explicit FilmContainer();
    explicit FilmContainer(CNFilmProfile profile, double gamma, bool sixteenBit);

    void   setWhitePoint(const DColor& wp);
    DColor whitePoint()                         const;

    void   setExposure(double strength);
    double exposure()                           const;

    void   setSixteenBit(bool val);
    void   setGamma(double val);
    double gamma()                              const;

    void          setCNType(CNFilmProfile profile);
    CNFilmProfile cnType()                      const;

    void setApplyBalance(bool val);
    bool applyBalance()                         const;

    LevelsContainer toLevels()                  const;
    CBContainer     toCB()                      const;

public:

    static const QMap<int, QString> profileMap;
    static QList<ListItem*> profileItemList(QListWidget* const view);

private:

    int    whitePointForChannel(int channel)    const;
    double blackPointForChannel(int ch)         const;
    double gammaForChannel(int ch)              const;

    static QMap<int, QString> profileMapInitializer();

private:

    class Private;
    QSharedPointer<Private> d;
};

// ---------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT FilmFilter: public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit FilmFilter(QObject* const parent = nullptr);
    explicit FilmFilter(DImg* const orgImage, QObject* const parent = nullptr,
                        const FilmContainer& settings = FilmContainer());
    ~FilmFilter()                                                 override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:FilmFilter");
    }

    static QString DisplayableName();

    static QList<int> SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int CurrentVersion()
    {
        return 1;
    }

    QString filterIdentifier()                              const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                                   override;
    void readParameters(const FilterAction& action)               override;

private:

    void filterImage()                                            override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
