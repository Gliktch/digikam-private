# #
# # SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# #
# # SPDX-License-Identifier: BSD-3-Clause
# #

# APPLY_COMMON_POLICIES()

# add_custom_target(datalink_help ALL
#                   COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../data ${CMAKE_CURRENT_BINARY_DIR}/data)

# add_custom_target(scriptlink_help ALL
#                   COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../scripts ${CMAKE_CURRENT_BINARY_DIR}/scripts)

# # -----------------------------------------------------------------------------

# set(convert_format_cli_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/convert_format_cli.cpp)
# add_executable(convert_format_cli ${convert_format_cli_SRCS})

# target_link_libraries(convert_format_cli

#                       digikamcore
#                       digikamdatabase
#                       digikamgui

#                       ${COMMON_TEST_LINK}
# )

