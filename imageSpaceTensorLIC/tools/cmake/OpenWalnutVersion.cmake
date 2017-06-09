#---------------------------------------------------------------------------
#
# Project: OpenWalnut ( http://www.openwalnut.org )
#
# Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
# For more information see http://www.openwalnut.org/copying
#
# This file is part of OpenWalnut.
#
# OpenWalnut is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OpenWalnut is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with OpenWalnut. If not, see <http://www.gnu.org/licenses/>.
#
#---------------------------------------------------------------------------

# This is a script executed during build time to genertae the WVersion.h headers. This is needed since CMake does not allow us to define the
# commands directly inside an ADD_CUSTOM_COMMAND call. An alternative to this would be SET_SOURCE_FILE_PROPERTIES which should be able to set the
# version as define for some files. But this caused the whole source to rebuild although it was only set for one file (bug? feature? we do not
# know.)

INCLUDE( OpenWalnutUtils.cmake )
GET_VERSION_STRING( OW_VERSION OW_API_VERSION )
FILE( WRITE ${HEADER_FILENAME} "// generated by CMake. Do not modify.\n#define ${PREFIX}_VERSION \"${OW_VERSION}\"\n#define ${PREFIX}_LIB_VERSION \"${OW_API_VERSION}\"\n" )
