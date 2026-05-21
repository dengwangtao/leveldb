#include <cassert>
#include <iostream>
#include <string>

#include "db.h"
#include "write_batch.h"

void CheckStatus(const leveldb::Status& status, const std::string& msg)
{
    if (!status.ok())
    {
        std::cerr << msg << ": " << status.ToString() << std::endl;
        std::exit(1);
    }
}

int main()
{
    leveldb::DB* db = nullptr;

    leveldb::Options options;
    options.create_if_missing = true;

    const std::string db_path = "testdb";

    leveldb::Status status = leveldb::DB::Open(options, db_path, &db);
    CheckStatus(status, "open db failed");

    std::cout << "Open database successfully.\n";

    // 1. Put：写入单个 key-value
    status = db->Put(leveldb::WriteOptions(), "user:1", "Alice");
    CheckStatus(status, "put user:1 failed");

    status = db->Put(leveldb::WriteOptions(), "user:2", "Bob");
    CheckStatus(status, "put user:2 failed");

    status = db->Put(leveldb::WriteOptions(), "user:3", "Charlie");
    CheckStatus(status, "put user:3 failed");

    // 2. Get：读取 key
    std::string value;
    status = db->Get(leveldb::ReadOptions(), "user:1", &value);
    if (status.ok())
    {
        std::cout << "user:1 = " << value << "\n";
    }
    else
    {
        std::cout << "get user:1 failed: " << status.ToString() << "\n";
    }

    // 3. Delete：删除 key
    status = db->Delete(leveldb::WriteOptions(), "user:2");
    CheckStatus(status, "delete user:2 failed");

    value.clear();
    status = db->Get(leveldb::ReadOptions(), "user:2", &value);
    if (status.IsNotFound())
    {
        std::cout << "user:2 has been deleted.\n";
    }

    // 4. WriteBatch：批量写入
    leveldb::WriteBatch batch;
    batch.Put("user:4", "David");
    batch.Put("user:5", "Eva");
    batch.Delete("user:3");

    status = db->Write(leveldb::WriteOptions(), &batch);
    CheckStatus(status, "write batch failed");

    // 5. Iterator：遍历所有 key-value
    std::cout << "\nIterate all records:\n";

    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());

    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        std::cout << it->key().ToString() << " => " << it->value().ToString() << "\n";
    }

    if (!it->status().ok())
    {
        std::cerr << "iterator error: " << it->status().ToString() << "\n";
    }

    delete it;

    // 6. Range Scan：范围扫描
    std::cout << "\nRange scan [user:1, user:5):\n";

    it = db->NewIterator(leveldb::ReadOptions());

    for (it->Seek("user:1"); it->Valid() && it->key().ToString() < "user:5"; it->Next())
    {
        std::cout << it->key().ToString() << " => " << it->value().ToString() << "\n";
    }

    delete it;

    // 7. Snapshot：快照读取
    std::cout << "\nSnapshot demo:\n";

    status = db->Put(leveldb::WriteOptions(), "config", "version1");
    CheckStatus(status, "put config version1 failed");

    const leveldb::Snapshot* snapshot = db->GetSnapshot();

    status = db->Put(leveldb::WriteOptions(), "config", "version2");
    CheckStatus(status, "put config version2 failed");

    leveldb::ReadOptions snapshot_read_options;
    snapshot_read_options.snapshot = snapshot;

    value.clear();
    status = db->Get(snapshot_read_options, "config", &value);
    CheckStatus(status, "snapshot get failed");

    std::cout << "config in snapshot = " << value << "\n";

    value.clear();
    status = db->Get(leveldb::ReadOptions(), "config", &value);
    CheckStatus(status, "normal get failed");

    std::cout << "config now = " << value << "\n";

    db->ReleaseSnapshot(snapshot);

    // 8. Sync Write：同步写入
    leveldb::WriteOptions sync_write_options;
    sync_write_options.sync = true;

    status = db->Put(sync_write_options, "sync_key", "sync_value");
    CheckStatus(status, "sync put failed");

    std::cout << "\nSync write finished.\n";

    // 9. CompactRange：手动触发 Compaction
    db->CompactRange(nullptr, nullptr);

    std::cout << "Manual compaction finished.\n";

    delete db;

    std::cout << "\nDone.\n";
    return 0;
}