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

#ifndef WOPENCLCODE_H
#define WOPENCLCODE_H

#include <boost/filesystem.hpp>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

#include <core/common/WLogger.h>

/**
 * Wrapps initialization and code binding for OpenCL kernels into an handy OW class.
 */
class WOpenCLCode
{
//public:
//    explicit WOpenCLCode( boost::filesystem::path path );
//
//protected:
//    wlog::WStreamedLogger debugLog() const;
//
//
//private:
//    void initDevice();
//
////    static void clErrorLog( const char * errorinfo, const void * private_info_size, ::size_t cb, void * user_data );
//
//    std::vector< cl::Device > m_clDevices;
//    cl::Context m_clContext;
//
//    cl_int m_err;
//    boost::filesystem::path m_kernelSource;
};

#endif  // WOPENCLCODE_H
