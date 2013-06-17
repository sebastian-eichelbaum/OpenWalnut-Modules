# Adapted flags for this directory to inhibit all warnings.
# There is a good reason for this: We do not want to debug
# third party software!

IF( CMAKE_GENERATOR MATCHES "Visual Studio" )
    SET( CMAKE_C_FLAGS "/TC /DWIN32")
ELSE()
    SET( CMAKE_C_FLAGS "-w ${OW_C_FLAGS_INJECT}" )
ENDIF()

FILE( GLOB LIBPDSAMPLE_SRC "*.cpp" "*.h" )

ADD_LIBRARY( ${OWExtPDSampleName} SHARED ${LIBPDSAMPLE_SRC} )
IF( CMAKE_GENERATOR MATCHES "Visual Studio" )
    TARGET_LINK_LIBRARIES( ${OWExtPDSampleName} )
ELSE()
    TARGET_LINK_LIBRARIES( ${OWExtPDSampleName} m )
ENDIF()

# Set the version of the library.
SET_TARGET_PROPERTIES( ${OWExtPDSampleName} PROPERTIES
        VERSION ${OW_LIB_VERSION} 
        SOVERSION ${OW_SOVERSION}
)

# install target
INSTALL( TARGETS ${OWExtPDSampleName}
            ARCHIVE # NOTE: this is needed on windows
                DESTINATION ${OW_LIBRARY_DIR_RELATIVE}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
            RUNTIME # NOTE: this is needed on windows
                DESTINATION ${OW_LIBRARY_DIR_RELATIVE}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
            LIBRARY # NOTE: this is needed for all the others
                DESTINATION ${OW_LIBRARY_DIR_RELATIVE}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
                NAMELINK_SKIP
         COMPONENT "EXT"
)

# provide an namelink-only target for dev install
INSTALL( TARGETS ${OWExtPDSampleName}
            ARCHIVE # NOTE: this is needed on windows
                DESTINATION ${OW_LIBRARY_DIR_RELATIVE}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
            LIBRARY # NOTE: this is needed for all the others
                DESTINATION ${OW_LIBRARY_DIR_RELATIVE}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
                NAMELINK_ONLY
            COMPONENT "EXT_DEV"
)

