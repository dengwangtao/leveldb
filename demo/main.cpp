#include <cassert>
#include <iostream>
#include <string>

#include "db.h"

int main()
{
    leveldb::DB* db = nullptr;

    leveldb::Options options;
    options.create_if_missing = true;

    // 数据库目录，运行后会生成这个文件夹
    std::string db_path = "testdb";

    leveldb::Status status = leveldb::DB::Open(options, db_path, &db);
    if (!status.ok())
    {
        std::cerr << "Open database failed: " << status.ToString() << std::endl;
        return 1;
    }

    // 写入 key-value
    status = db->Put(leveldb::WriteOptions(), "name", "leveldb");
    if (!status.ok())
    {
        std::cerr << "Put failed: " << status.ToString() << std::endl;
        delete db;
        return 1;
    }

    // 读取 key
    std::string value;
    status = db->Get(leveldb::ReadOptions(), "name", &value);
    if (!status.ok())
    {
        std::cerr << "Get failed: " << status.ToString() << std::endl;
        delete db;
        return 1;
    }

    std::cout << "name = " << value << std::endl;

    // 更新 key
    status = db->Put(leveldb::WriteOptions(), "name", "leveldb-demo");
    if (!status.ok())
    {
        std::cerr << "Update failed: " << status.ToString() << std::endl;
        delete db;
        return 1;
    }

    value.clear();
    status = db->Get(leveldb::ReadOptions(), "name", &value);
    if (status.ok())
    {
        std::cout << "name after update = " << value << std::endl;
    }

    // 删除 key
    status = db->Delete(leveldb::WriteOptions(), "name");
    if (!status.ok())
    {
        std::cerr << "Delete failed: " << status.ToString() << std::endl;
        delete db;
        return 1;
    }

    value.clear();
    status = db->Get(leveldb::ReadOptions(), "name", &value);
    if (status.IsNotFound())
    {
        std::cout << "name has been deleted" << std::endl;
    }
    else if (!status.ok())
    {
        std::cerr << "Get after delete failed: " << status.ToString() << std::endl;
    }

    delete db;
    return 0;
}