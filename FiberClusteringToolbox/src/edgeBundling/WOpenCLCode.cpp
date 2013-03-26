//---------------------------------------------------------------------------
//
// Project: OpenWalnut ( http://www.openwalnut.org )
//
// Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
// For more information see http://www.openwalnut.org/copying
//
// This file is part of OpenWalnut.
//
// OpenWalnut is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenWalnut is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with OpenWalnut. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------

#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <core/common/WLogger.h>

#include "WOpenCLCode.h"

//WOpenCLCode::WOpenCLCode( boost::filesystem::path path )
//{
//    initDevice();
//}
//
//void WOpenCLCode::initDevice()
//{
//    //cl::Context context( CL_DEVICE_TYPE_GPU, &WOpenCLCode::clErrorLog );
//    //m_devices = context.getInfo< CL_CONTEXT_DEVICES >();
//    //cl_int                      ret;
//    //std::string value;
//    //std::vector< cl::Platform > platforms;
//    //cl::Platform::get( &platforms );
//    //for( std::vector< cl::Platform >::const_iterator cit = platforms.begin(); cit != platforms.end(); ++cit )
//    //{
//    //    cit->getInfo( CL_PLATFORM_PROFILE, &value );
//    //    debugLog() << "CL_PLATFORM_PROFILE: " << value;
//    //    cit->getInfo( CL_PLATFORM_VERSION, &value );
//    //    debugLog() << "CL_PLATFORM_VERSION: " << value;
//    //    cit->getInfo( CL_PLATFORM_NAME , &value );
//    //    debugLog() << "CL_PLATFORM_NAME: " << value;
//    //    cit->getInfo( CL_PLATFORM_VENDOR , &value );
//    //    debugLog() << "CL_PLATFORM_VENDOR: " << value;
//    //    cit->getInfo( CL_PLATFORM_EXTENSIONS, &value );
//    //    debugLog() << "CL_PLATFORM_EXTENSIONS: " << value;
//    //}
//
//}
//
//wlog::WStreamedLogger WOpenCLCode::debugLog() const
//{
//    return wlog::debug( "WOpenCLCode" );
//}

//void WOpenCLCode::clErrorLog( const char * errorinfo, const void * private_info_size, ::size_t cb, void * user_data )
//{
//    return wlog::error( "OpenCL" ) << std::string( errorinfo );
//}
