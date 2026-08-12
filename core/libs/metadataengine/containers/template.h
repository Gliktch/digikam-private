/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-20
 * Description : Template information container.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QDebug>

// Local includes

#include "metadatainfo.h"
#include "digikam_export.h"
#include "metaengine.h"

namespace Digikam
{

class TemplatePrivate;

class DIGIKAM_EXPORT Template
{
public:

    Template()  = default;
    ~Template() = default;

    /**
     * Return true if Template title is null
     */
    bool isNull()                               const;

    /**
     * Return true if Template contents is empty
     */
    bool isEmpty()                              const;

    /**
     * Merge the metadata from another Template
     */
    void merge(const Template& t);

    /**
     * Compare for metadata equality, not including "templateTitle" value.
     */
    bool operator==(const Template& t)          const;

    void setTemplateTitle(const QString& title);
    QString templateTitle()                     const;

    void setTemplateMerge(bool merge);
    bool templateMerge()                        const;

    void setAuthors(const QStringList& authors);
    void setAuthorsPosition(const QString& authorPosition);
    void setCredit(const QString& credit);
    void setCopyright(const MetaEngine::AltLangMap& copyright);
    void setRightUsageTerms(const MetaEngine::AltLangMap& rightUsageTerms);
    void setSource(const QString& source);
    void setInstructions(const QString& instructions);
    void setLocationInfo(const IptcCoreLocationInfo& inf);
    void setContactInfo(const IptcCoreContactInfo& inf);
    void setIptcSubjects(const QStringList& subjects);

    QStringList            authors()            const;
    QString                authorsPosition()    const;
    QString                credit()             const;
    MetaEngine::AltLangMap copyright()          const;
    MetaEngine::AltLangMap rightUsageTerms()    const;
    QString                source()             const;
    QString                instructions()       const;
    IptcCoreLocationInfo   locationInfo()       const;
    IptcCoreContactInfo    contactInfo()        const;
    QStringList            IptcSubjects()       const;

    static QString removeTemplateTitle()
    {
        return QLatin1String("_REMOVE_TEMPLATE_");
    };

protected:

    /**
     * Template title used internally. This value always exist and cannot be empty.
     */
    QString                  m_templateTitle;

    /**
     * Merge flag when applying the template.
     */
    bool                     m_templateMerge  = false;

    /**
     * List of author names.
     */
    QStringList              m_authors;

    /**
     * Description of authors position.
     */
    QString                  m_authorsPosition;

    /**
     * Credit description.
     */
    QString                  m_credit;

    /**
     * Language alternative copyright notices.
     */
    MetaEngine::AltLangMap   m_copyright;

    /**
     * Language alternative right term usages.
     */
    MetaEngine::AltLangMap   m_rightUsageTerms;

    /**
     * Descriptions of contents source.
     */
    QString                  m_source;

    /**
     * Special instructions to process with contents.
     */
    QString                  m_instructions;

    /**
     * IPTC Location Information.
     */
    IptcCoreLocationInfo     m_locationInfo;

    /**
     * IPTC Contact Information.
     */
    IptcCoreContactInfo      m_contactInfo;

    /**
     * IPTC Subjects Information.
     */
    QStringList              m_subjects;
};

//! qDebug() stream operator. Writes Template to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const Template& t);

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Template)
