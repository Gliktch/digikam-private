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

#include "template.h"

namespace Digikam
{

bool Template::isNull() const
{
    return m_templateTitle.isNull();
}

bool Template::operator==(const Template& t) const
{
    bool b1  = (m_authors         == t.m_authors);
    bool b2  = (m_authorsPosition == t.m_authorsPosition);
    bool b3  = (m_credit          == t.m_credit);
    bool b4  = (m_copyright       == t.m_copyright);
    bool b5  = (m_rightUsageTerms == t.m_rightUsageTerms);
    bool b6  = (m_source          == t.m_source);
    bool b7  = (m_instructions    == t.m_instructions);
    bool b8  = (m_locationInfo    == t.m_locationInfo);
    bool b9  = (m_contactInfo     == t.m_contactInfo);
    bool b10 = (m_subjects        == t.m_subjects);
/*
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.authors()         << m_authors         << b1;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.authorsPosition() << m_authorsPosition << b2;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.credit()          << m_credit          << b3;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.copyright()       << m_copyright       << b4;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.rightUsageTerms() << m_rightUsageTerms << b5;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.source()          << m_source          << b6;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.instructions()    << m_instructions    << b7;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.locationInfo()    << m_locationInfo    << b8;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.contactInfo()     << m_contactInfo     << b9;
    qCDebug(DIGIKAM_METAENGINE_LOG) << t.IptcSubjects()    << m_subjects        << b10;
*/
    return (b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10);
}

bool Template::isEmpty() const
{
    return (
            m_authors.isEmpty()         &&
            m_authorsPosition.isEmpty() &&
            m_credit.isEmpty()          &&
            m_copyright.isEmpty()       &&
            m_rightUsageTerms.isEmpty() &&
            m_source.isEmpty()          &&
            m_instructions.isEmpty()    &&
            m_locationInfo.isEmpty()    &&
            m_contactInfo.isEmpty()     &&
            m_subjects.isEmpty()
           );
}

void Template::merge(const Template& t)
{
    m_templateTitle        = t.templateTitle();
    m_templateMerge        = t.templateMerge();
    const QString xdefault = QLatin1String("x-default");

    if (!m_templateMerge || (!t.authors().isEmpty() && m_authors.isEmpty()))
    {
        m_authors = t.authors();
    }

    if (!m_templateMerge || (!t.authorsPosition().isEmpty() && m_authorsPosition.isEmpty()))
    {
        m_authorsPosition = t.authorsPosition();
    }

    if (!m_templateMerge || (!t.credit().isEmpty() && m_credit.isEmpty()))
    {
        m_credit = t.credit();
    }

    bool tCopyright = t.copyright().value(xdefault).isEmpty();
    bool mCopyright = m_copyright.value(xdefault).isEmpty();

    if (!m_templateMerge || (!tCopyright && mCopyright))
    {
        m_copyright = t.copyright();
    }

    bool tRightUsageTerms = t.rightUsageTerms().value(xdefault).isEmpty();
    bool mRightUsageTerms = m_rightUsageTerms.value(xdefault).isEmpty();

    if (!m_templateMerge || (!tRightUsageTerms && mRightUsageTerms))
    {
        m_rightUsageTerms = t.rightUsageTerms();
    }

    if (!m_templateMerge || (!t.source().isEmpty() && m_source.isEmpty()))
    {
        m_source = t.source();
    }

    if (!m_templateMerge || (!t.instructions().isEmpty() && m_instructions.isEmpty()))
    {
        m_instructions = t.instructions();
    }

    m_locationInfo.merge(t.locationInfo(), m_templateMerge);
    m_contactInfo.merge(t.contactInfo(), m_templateMerge);

    if (!m_templateMerge || (!t.IptcSubjects().isEmpty() && m_subjects.isEmpty()))
    {
        m_subjects = t.IptcSubjects();
    }
}

void Template::setTemplateTitle(const QString& title)
{
    m_templateTitle = title;
}

QString Template::templateTitle() const
{
    return m_templateTitle;
}

void Template::setTemplateMerge(bool merge)
{
    m_templateMerge = merge;
}

bool Template::templateMerge() const
{
    return m_templateMerge;
}

void Template::setAuthors(const QStringList& authors)
{
    m_authors = authors;
    m_authors.sort();
}

void Template::setAuthorsPosition(const QString& authorsPosition)
{
    m_authorsPosition = authorsPosition;
}

void Template::setCredit(const QString& credit)
{
    m_credit = credit;
}

void Template::setCopyright(const MetaEngine::AltLangMap& copyright)
{
    m_copyright = copyright;
}

void Template::setRightUsageTerms(const MetaEngine::AltLangMap& rightUsageTerms)
{
    m_rightUsageTerms = rightUsageTerms;
}

void Template::setSource(const QString& source)
{
    m_source = source;
}

void Template::setInstructions(const QString& instructions)
{
    m_instructions = instructions;
}

void Template::setLocationInfo(const IptcCoreLocationInfo& inf)
{
    m_locationInfo = inf;
}

void Template::setContactInfo(const IptcCoreContactInfo& inf)
{
    m_contactInfo = inf;
}

void Template::setIptcSubjects(const QStringList& subjects)
{
    m_subjects = subjects;
    m_subjects.sort();
}

QStringList Template::authors() const
{
    return m_authors;
}

QString Template::authorsPosition() const
{
    return m_authorsPosition;
}

QString Template::credit() const
{
    return m_credit;
}

MetaEngine::AltLangMap Template::copyright() const
{
    return m_copyright;
}

MetaEngine::AltLangMap Template::rightUsageTerms() const
{
    return m_rightUsageTerms;
}

QString Template::source() const
{
    return m_source;
}

QString Template::instructions() const
{
    return m_instructions;
}

IptcCoreLocationInfo Template::locationInfo() const
{
    return m_locationInfo;
}

IptcCoreContactInfo Template::contactInfo() const
{
    return m_contactInfo;
}

QStringList Template::IptcSubjects() const
{
    return m_subjects;
}

QDebug operator<<(QDebug dbg, const Template& t)
{
    dbg.nospace() << "Template::title: "
                  << t.templateTitle() << ", ";
    dbg.nospace() << "Template::merge: "
                  << t.templateMerge() << ", ";
    dbg.nospace() << "Template::authors: "
                  << t.authors() << ", ";
    dbg.nospace() << "Template::authorsPosition: "
                  << t.authorsPosition() << ", ";
    dbg.nospace() << "Template::credit: "
                  << t.credit() << ", ";
    dbg.nospace() << "Template::copyright: "
                  << t.copyright() << ", ";
    dbg.nospace() << "Template::rightUsageTerms: "
                  << t.rightUsageTerms() << ", ";
    dbg.nospace() << "Template::source: "
                  << t.source() << ", ";
    dbg.nospace() << "Template::instructions: "
                  << t.instructions() << ", ";
    dbg.nospace() << "Template::locationinfo: "
                  << t.locationInfo() << ", ";
    dbg.nospace() << "Template::contactinfo: "
                  << t.contactInfo();
    dbg.nospace() << "Template::IptcSubjects: "
                  << t.IptcSubjects();

    return dbg.space();
}

} // namespace Digikam
