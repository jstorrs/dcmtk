# declare additional include directories needed for compression support
include_directories("${dcmjpls_SOURCE_DIR}/include" "${dcmjpeg_SOURCE_DIR}/include" "${dcmimage_SOURCE_DIR}/include" "${dcmimgle_SOURCE_DIR}/include")

# declare executables
foreach(PROGRAM dcmdump+ dcmftest+ storescp+)
  DCMTK_ADD_EXECUTABLE(${PROGRAM} ${PROGRAM}.cc)
endforeach()

# declare installation files
if(WITH_OPENSSL)
  if(WIN32)
    install(DIRECTORY "${OPENSSL_BINDIR}/"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT bin
        CONFIGURATIONS Release MinSizeRel
        FILES_MATCHING REGEX "dcmtk(eay|crypto|ssl).*\\.dll"
        REGEX "dcmtk(eay|crypto|ssl)_d.*\\.dll" EXCLUDE
    )
    install(DIRECTORY "${OPENSSL_BINDIR}/"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT bin
        CONFIGURATIONS Debug RelWithDebInfo
        FILES_MATCHING REGEX "dcmtk(eay|crypto|ssl)_d.*\\.dll"
    )
  endif()
endif()

# make sure executables are linked to the corresponding libraries
DCMTK_TARGET_LINK_MODULES(dcmdump+ dcmdata oflog ofstd tweaks)
DCMTK_TARGET_LINK_MODULES(storescp+ dcmnet dcmdata oflog ofstd dcmtls tweaks)
DCMTK_TARGET_LINK_MODULES(dcmftest+ ofstd)
