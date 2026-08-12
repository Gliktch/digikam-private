#
# SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# SPDX-License-Identifier: BSD-3-Clause
#

APPLY_COMMON_POLICIES()

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

# add_subdirectory(alignment)
# add_subdirectory(preprocess)
# add_subdirectory(detection)
# add_subdirectory(recognition)
# add_subdirectory(demo)
# add_subdirectory(helper)
add_subdirectory(unittests)
