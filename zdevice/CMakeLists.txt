project(zdevice)

set(HEADER_DIR "include/zdevice")
set(PRIVATE_HEADER_DIR "private/include")
set(SRC_DIR "src")

include_directories(private/include)

set(HEADER_FILES
    ${HEADER_DIR}/ZDevice.h
    ${HEADER_DIR}/ZDeviceInfo.h
    ${HEADER_DIR}/ZDeviceManager.h
    ${HEADER_DIR}/ZVarInfo.h
    ${HEADER_DIR}/ZVarLastestValue.h
    ${HEADER_DIR}/ZVarValue.h
)

set(PRIVATE_HEADER_FILES "")

set(SOURCE_FILES
    ${SRC_DIR}/ZDevice.cpp
    ${SRC_DIR}/ZDeviceInfo.cpp
    ${SRC_DIR}/ZDeviceManager.cpp
    ${SRC_DIR}/ZVarInfo.cpp
    ${SRC_DIR}/ZVarLastestValue.cpp
    ${SRC_DIR}/ZVarValue.cpp
)

add_library(zdevice STATIC ${SOURCE_FILES} ${HEADER_FILES} ${PRIVATE_HEADER_FILES})
