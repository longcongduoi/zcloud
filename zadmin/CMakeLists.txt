project(zadmin)

set(HEADER_DIR "include/zadmin")
set(SRC_DIR "src")

include_directories(private/include)

set(HEADER_FILES
    ${HEADER_DIR}/ZAdminInfo.h
    ${HEADER_DIR}/ZAdminManager.h
)

set(SOURCE_FILES
    ${SRC_DIR}/ZAdminInfo.cpp
    ${SRC_DIR}/ZAdminManager.cpp
)

add_library(zadmin STATIC ${SOURCE_FILES} ${HEADER_FILES})
