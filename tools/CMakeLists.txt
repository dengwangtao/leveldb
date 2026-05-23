add_executable(manifest_dump manifest_dump.cc)
target_link_libraries(manifest_dump PRIVATE leveldb)

add_executable(wal_dump wal_dump.cc)
target_link_libraries(wal_dump PRIVATE leveldb)