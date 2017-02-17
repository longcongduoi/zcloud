project(zapihttp.web)

set(HEADER_DIR "include/zapihttp.web")
set(HEADER_PRIVATE_DIR "private/include")
set(SRC_DIR "src")

include_directories(private/include)

set(HEADER_FILES
    ${HEADER_DIR}/ZApiHTTPWebRequestHandler.h
    ${HEADER_DIR}/ZHTTPWebManager.h
)

set(HEADER_PRIVATE_FILES
    ${HEADER_PRIVATE_DIR}/ZHTTPWidget.h
)

set(SOURCE_FILES
    ${SRC_DIR}/ZHTTPWebManager.cpp
    ${SRC_DIR}/ZHTTPWidget.cpp
    ${SRC_DIR}/ZApiHTTPWebRequestHandler.cpp
)

add_library(zapihttp.web STATIC ${SOURCE_FILES} ${HEADER_FILES} ${HEADER_PRIVATE_FILES})
