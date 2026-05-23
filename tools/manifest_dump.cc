#include <iostream>
#include <memory>
#include <string>

#include "db/log_reader.h"
#include "db/version_edit.h"
#include "leveldb/env.h"

class LogReporter : public leveldb::log::Reader::Reporter
{
public:
    void Corruption(size_t bytes, const leveldb::Status& status) override
    {
        std::cerr << "corruption: dropped " << bytes << " bytes, reason: " << status.ToString() << "\n";
    }
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: manifest_dump <manifest_file>\n";
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
        leveldb::VersionEdit edit;
        s = edit.DecodeFrom(record);

        std::cout << "\n========== Manifest Record " << index++ << " ==========\n";

        if (!s.ok())
        {
            std::cout << "Decode VersionEdit failed: " << s.ToString() << "\n";
            std::cout << "Raw size: " << record.size() << " bytes\n";
            continue;
        }

        std::cout << edit.DebugString() << "\n";
    }

    return 0;
}