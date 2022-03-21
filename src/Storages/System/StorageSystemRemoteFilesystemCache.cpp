#include "StorageSystemRemoteFilesystemCache.h"
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <DataTypes/DataTypeTuple.h>
#include <Common/FileCache.h>
#include <Common/FileCacheFactory.h>
#include <Interpreters/Context.h>
#include <Disks/IDisk.h>
#include <Storages/System/StorageSystemRemoteFilesystemCache.h>


namespace DB
{

NamesAndTypesList StorageSystemRemoteFilesystemCache::getNamesAndTypes()
{
    return {
        {"cache_base_path", std::make_shared<DataTypeString>()},
        {"cache_path", std::make_shared<DataTypeString>()},
        {"file_segment_range", std::make_shared<DataTypeTuple>(DataTypes{std::make_shared<DataTypeUInt64>(), std::make_shared<DataTypeUInt64>()})},
        {"size", std::make_shared<DataTypeUInt64>()},
    };
}

StorageSystemRemoteFilesystemCache::StorageSystemRemoteFilesystemCache(const StorageID & table_id_)
    : IStorageSystemOneBlock(table_id_)
{
}

void StorageSystemRemoteFilesystemCache::fillData(MutableColumns & res_columns, ContextPtr, const SelectQueryInfo &) const
{
    auto caches = FileCacheFactory::instance().getAll();

    for (const auto & [cache_base_path, cache_data] : caches)
    {
        auto & cache = cache_data.cache;
        auto holder = cache->getAll();

        for (const auto & file_segment : holder.file_segments)
        {
            res_columns[0]->insert(cache_base_path);
            res_columns[1]->insert(cache->getPathInLocalCache(file_segment->key(), file_segment->offset()));

            const auto & range = file_segment->range();
            res_columns[2]->insert(Tuple({range.left, range.right}));
            res_columns[3]->insert(range.size());
        }
    }
}

}
