# DWT Concept


options

status

slice

comparator

```
Comparator
  │
  ├─ MemTable
  │    └─ SkipList 按 InternalKeyComparator 排序
  │
  ├─ SSTable
  │    ├─ BlockBuilder 要求 key 按递增顺序 Add
  │    ├─ Block 内查找依赖 comparator
  │    └─ index block separator 依赖 FindShortestSeparator
  │
  ├─ VersionSet
  │    ├─ 判断文件 key range 是否重叠
  │    ├─ level>=1 文件按 key range 排序
  │    └─ compaction input 选择依赖 key 范围
  │
  ├─ Iterator
  │    └─ 多路归并时比较各路 key
  │
  └─ InternalKeyComparator
       ├─ 先用 user comparator 比较 user key
       └─ user key 相同，sequence number 越大越靠前
```


write_batch

