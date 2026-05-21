


set(LEVELDB_ROOT "${CMAKE_SOURCE_DIR}/${LEVELDB_PUBLIC_INCLUDE_DIR}")
message("LEVELDB_ROOT: ${LEVELDB_ROOT}")

add_executable(leveldb_demo main.cpp)

target_include_directories(leveldb_demo PRIVATE
    "${LEVELDB_ROOT}"
)

target_link_libraries(leveldb_demo leveldb)