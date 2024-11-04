# declare project
project(tweaks)

# declare include directories which hold for all subdirectories
include_directories("${tweaks_SOURCE_DIR}/include" "${dcmnet_SOURCE_DIR}/include" "${dcmtls_SOURCE_DIR}/include" ${OPENSSL_INCDIR} "${ofstd_SOURCE_DIR}/include" "${oflog_SOURCE_DIR}/include" "${dcmdata_SOURCE_DIR}/include" ${ZLIB_INCDIR})

# recurse into subdirectories
foreach(SUBDIR apps libsrc include)
  add_subdirectory(${SUBDIR})
endforeach()
