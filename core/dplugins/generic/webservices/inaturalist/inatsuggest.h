/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-03-20
 * Description : a tool to export images to iNaturalist web service
 *
 * SPDX-FileCopyrightText: 2021      by Joerg Lohse <joergmlpts at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QLineEdit>
#include <QEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QList>
#include <QVector>
#include <QHash>

// Local includes

#include "inattaxon.h"
#include "inattaxonedit.h"
#include "inatscore.h"

namespace DigikamGenericINatPlugin
{

class INatTalker;

typedef QPair<QString, QList<Taxon> > AutoCompletions;

class SuggestTaxonCompletion : public QObject
{
    Q_OBJECT

public:

    explicit SuggestTaxonCompletion(TaxonEdit* const parent);
    ~SuggestTaxonCompletion();

    void setTalker(INatTalker* const talker);

Q_SIGNALS:

    void signalTaxonSelected(const Taxon&, bool fromVision);
    void signalTaxonDeselected();
    void signalComputerVision();

private Q_SLOTS:

    void slotDoneCompletion();
    void slotPreventSuggest();
    void slotAutoSuggest();
    void slotInFocus();
    void slotTaxonAutoCompletions(const AutoCompletions&);
    void slotComputerVisionResults(const ImageScores&);
    void slotImageLoaded(const QUrl&, const QByteArray&);
    void slotTextEdited(const QString&);

private:

    bool    eventFilter(QObject* obj, QEvent* ev) override;
    void    showCompletion(const struct Completions&);
    void    taxon2Item(const Taxon&, QTreeWidgetItem*, const QString&);
    QString getText() const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericINatPlugin

