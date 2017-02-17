project(zdb)

set(HEADER_DIR "include/zdb")
set(HEADER_PRIVATE_DIR "private/include")
set(SRC_DIR "src")

add_definitions(-DNO_BOOST)

include_directories(private/include)
include_directories(private/include/hiredis)
include_directories(private/include/redis3m)

set(HEADER_FILES
    ${HEADER_DIR}/ZDBProxy.h
    ${HEADER_DIR}/ZIdGenerator.h
)

set(HEADER_PRIVATE_FILES
    ${HEADER_PRIVATE_DIR}/hiredis/fmacros.h
    ${HEADER_PRIVATE_DIR}/hiredis/hiredis.h
    ${HEADER_PRIVATE_DIR}/hiredis/net.h
    ${HEADER_PRIVATE_DIR}/hiredis/read.h
    ${HEADER_PRIVATE_DIR}/hiredis/sds.h

    ${HEADER_PRIVATE_DIR}/redis3m/cluster_pool.h
    ${HEADER_PRIVATE_DIR}/redis3m/command.h
    ${HEADER_PRIVATE_DIR}/redis3m/connection.h
    ${HEADER_PRIVATE_DIR}/redis3m/connection_pool.h
    ${HEADER_PRIVATE_DIR}/redis3m/redis3m.hpp
    ${HEADER_PRIVATE_DIR}/redis3m/reply.h
    ${HEADER_PRIVATE_DIR}/redis3m/simple_pool.h

    ${HEADER_PRIVATE_DIR}/redis3m/patterns/median_filter.h
    ${HEADER_PRIVATE_DIR}/redis3m/patterns/model.h
    ${HEADER_PRIVATE_DIR}/redis3m/patterns/orm.h
    ${HEADER_PRIVATE_DIR}/redis3m/patterns/patterns.hpp
    ${HEADER_PRIVATE_DIR}/redis3m/patterns/scheduler.h
    ${HEADER_PRIVATE_DIR}/redis3m/patterns/script_exec.h
    ${HEADER_PRIVATE_DIR}/redis3m/patterns/simple_obj_store.h

    ${HEADER_PRIVATE_DIR}/redis3m/utils/crc16.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/datetime.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/exception.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/file.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/logging.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/noncopyable.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/pool.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/resolv.h
    ${HEADER_PRIVATE_DIR}/redis3m/utils/sha1.h
)

set(SOURCE_FILES
    ${SRC_DIR}/ZDBProxy.cpp
    ${SRC_DIR}/ZIdGenerator.cpp

    ${SRC_DIR}/hiredis/hiredis.c
    ${SRC_DIR}/hiredis/net.c
    ${SRC_DIR}/hiredis/read.c
    ${SRC_DIR}/hiredis/sds.c

    ${SRC_DIR}/redis3m/cluster_pool.cpp
    ${SRC_DIR}/redis3m/connection.cpp
    ${SRC_DIR}/redis3m/connection_pool.cpp
    ${SRC_DIR}/redis3m/reply.cpp
    ${SRC_DIR}/redis3m/simple_pool.cpp

    ${SRC_DIR}/redis3m/patterns/median_filter.cpp
    ${SRC_DIR}/redis3m/patterns/scheduler.cpp
    ${SRC_DIR}/redis3m/patterns/script_exec.cpp

    ${SRC_DIR}/redis3m/utils/crc16.cpp
    ${SRC_DIR}/redis3m/utils/datetime.cpp
    ${SRC_DIR}/redis3m/utils/file.cpp
    ${SRC_DIR}/redis3m/utils/logging.cpp
    ${SRC_DIR}/redis3m/utils/resolv.cpp
    ${SRC_DIR}/redis3m/utils/sha1.cpp
)

add_library(zdb STATIC ${SOURCE_FILES} ${HEADER_FILES} ${HEADER_PRIVATE_FILES})
