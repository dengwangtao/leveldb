// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A filter block is stored near the end of a Table file.  It contains
// filters (e.g., bloom filters) for all data blocks in the table combined
// into a single filter block.

#ifndef STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "leveldb/slice.h"
#include "util/hash.h"

namespace leveldb
{

class FilterPolicy;

// A FilterBlockBuilder is used to construct all of the filters for a
// particular Table.  It generates a single string which is stored as
// a special block in the Table.
//
// The sequence of calls to FilterBlockBuilder must match the regexp:
//      (StartBlock AddKey*)* Finish
class FilterBlockBuilder
{
    public:
        explicit FilterBlockBuilder(const FilterPolicy*);

        FilterBlockBuilder(const FilterBlockBuilder&) = delete;
        FilterBlockBuilder& operator=(const FilterBlockBuilder&) = delete;

        void StartBlock(uint64_t block_offset);
        void AddKey(const Slice& key);
        Slice Finish();

    private:
        void GenerateFilter();

        const FilterPolicy* policy_;
        std::string keys_;            // add进来的所有key,摆放在 keys_ 中， start_中记录每个 key 的起始位置
        std::vector<size_t> start_;   // Starting index in keys_ of each key
        std::string result_;          // 最终的filter block结果
        std::vector<Slice> tmp_keys_; // 用于在GenerateFilter时，构建一个临时的Slice数组，成员指向 keys_ 中的每个key的开始
        std::vector<uint32_t> filter_offsets_; // 每个元素记录一个过滤器的起始位置
};

class FilterBlockReader
{
    public:
        // REQUIRES: "contents" and *policy must stay live while *this is live.
        FilterBlockReader(const FilterPolicy* policy, const Slice& contents);
        bool KeyMayMatch(uint64_t block_offset, const Slice& key);

    private:
        const FilterPolicy* policy_;
        const char* data_;   // Pointer to filter data (at block-start)
        const char* offset_; // Pointer to beginning of offset array (at block-end)
        size_t num_;         // Number of entries in offset array
        size_t base_lg_;     // Encoding parameter (see kFilterBaseLg in .cc file)
};

} // namespace leveldb

#endif // STORAGE_LEVELDB_TABLE_FILTER_BLOCK_H_
