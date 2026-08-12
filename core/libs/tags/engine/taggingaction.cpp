/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Special line edit for adding or creatingtags
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "taggingaction.h"

namespace Digikam
{

TaggingAction::TaggingAction(int tagId)
    : m_type (AssignTag),
      m_tagId(tagId)
{
}

TaggingAction::TaggingAction(const QString& name, int parentTagId)
    : m_type   (CreateNewTag),
      m_tagId  (parentTagId),
      m_tagName(name)
{
}

bool TaggingAction::operator==(const TaggingAction& other) const
{
    return (
            (m_type    == other.m_type)    &&
            (m_tagId   == other.m_tagId)   &&
            (m_tagName == other.m_tagName)
           );
}

TaggingAction::Type TaggingAction::type() const
{
    return m_type;
}

bool TaggingAction::isValid() const
{
    return (m_type != NoAction);
}

bool TaggingAction::shallAssignTag() const
{
    return (m_type == AssignTag);
}

bool TaggingAction::shallCreateNewTag() const
{
    return (m_type == CreateNewTag);
}

int TaggingAction::tagId() const
{
    return m_tagId;
}

QString TaggingAction::newTagName() const
{
    return m_tagName;
}

int TaggingAction::parentTagId() const
{
    return m_tagId;
}

} // namespace Digikam
