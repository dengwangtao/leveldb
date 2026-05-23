#include <iostream>
#include <memory>
#include <string>

#include "db/log_reader.h"
#include "db/write_batch_internal.h"
#include "leveldb/env.h"
#include "leveldb/write_batch.h"

// static std::string EscapeString(const leveldb::Slice& s)
// {
//     std::string r;
//     for (size_t i = 0; i < s.size(); ++i)
//     {
//         const unsigned char c = static_cast<unsigned char>(s.data()[i]);
//         if (c >= 32 && c <= 126)
//         {
//             r.push_back(static_cast<char>(c));
//         }
//         else
//         {
//             char buf[5];
//             std::snprintf(buf, sizeof(buf), "\\x%02x", c);
//             r.append(buf);
//         }
//     }
//     return r;
// }

class LogReporter : public leveldb::log::Reader::Reporter
{
public:
    void Corruption(size_t bytes, const leveldb::Status& status) override
    {
        std::cerr << "corruption: dropped " << bytes << " bytes, reason: " << status.ToString() << "\n";
    }
};

class PrintBatchHandler : public leveldb::WriteBatch::Handler
{
public:
    void Put(const leveldb::Slice& key, const leveldb::Slice& value) override
    {
        std::cout << "  PUT    " << EscapeString(key) << " => " << EscapeString(value) << "\n";
    }

    void Delete(const leveldb::Slice& key) override
    {
        std::cout << "  DELETE " << EscapeString(key) << "\n";
    }
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: wal_dump <log_file>\n";
        return 1;
    }

    const std::string file_name = argv[1];

    leveldb::SequentialFile* raw_file = nullptr;
    leveldb::Status s = leveldb::Env::Default()->NewSequentialFile(file_name, &raw_file);

    if (!s.ok())
    {
        std::cerr << "open file failed: " << s.ToString() << "\n";
        return 1;
    }

    std::unique_ptr<leveldb::SequentialFile> file(raw_file);

    LogReporter reporter;
    leveldb::log::Reader reader(file.get(), &reporter, true, 0);

    leveldb::Slice record;
    std::string scratch;

    int index = 0;
    while (reader.ReadRecord(&record, &scratch))
    {
        leveldb::WriteBatch batch;
        leveldb::WriteBatchInternal::SetContents(&batch, record);

        std::cout << "\n========== WAL Record " << index++ << " ==========\n";

        std::cout << "sequence: " << leveldb::WriteBatchInternal::Sequence(&batch) << "\n";

        std::cout << "count: " << leveldb::WriteBatchInternal::Count(&batch) << "\n";

        PrintBatchHandler handler;
        s = batch.Iterate(&handler);

        if (!s.ok())
        {
            std::cout << "WriteBatch decode failed: " << s.ToString() << "\n";
        }
    }

    return 0;
}