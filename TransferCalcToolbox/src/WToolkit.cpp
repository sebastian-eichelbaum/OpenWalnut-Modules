#include <boost/shared_ptr.hpp>

#include <core/kernel/WModule.h>

#include "transferCalc/WMTransferCalc.h"

#include "WToolkit.h"

// This file's purpose is to provide a list of modules as entry point for OpenWalnut's module loader.
// Add your modules here. If you miss this step, OpenWalnut will not be able to load your modules.
extern "C" void WLoadModule( WModuleList& m ) // NOLINT
{
    m.push_back( boost::shared_ptr< WModule >( new WMTransferCalc ) );
}

