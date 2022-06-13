#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>
#include <stdlib.h>

static int hash(int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

static int hash(char *str) {
    int hash = 5381;

    extern int get_string_length(char *str);
    int len = get_string_length(str);
    for (int i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + str[i];
    }
    return hash;
}

template <typename Key, typename Value>
struct Hash_Table {
    struct Bucket {
        Key key;
        Value value;
    };

    Bucket *buckets = nullptr;
    bool *occupancy_mask = nullptr;
    int allocated = 0;
    int count = 0;

    inline void grow() {
        const int HASH_TABLE_INITIAL_CAPACITY = 256;

        if (!buckets) {
            assert(allocated == 0);
            assert(count == 0);
            
            buckets = (Bucket *)calloc(HASH_TABLE_INITIAL_CAPACITY, sizeof(Bucket));
            occupancy_mask = (bool *)calloc(HASH_TABLE_INITIAL_CAPACITY, sizeof(bool));
            allocated = HASH_TABLE_INITIAL_CAPACITY;
            count = 0;
        } else {
            Hash_Table <Key, Value> new_hash_table = {
                (Bucket *)calloc(allocated * 2, sizeof(Bucket)),
                (bool *)calloc(allocated * 2, sizeof(bool)),
                allocated * 2,
                0,
            };

            for (int i = 0; i < count; i++) {
                if (occupancy_mask[i]) {
                    new_hash_table.add(buckets[i].key, buckets[i].value);
                }
            }

            free(buckets);
            free(occupancy_mask);

            *this = new_hash_table;
        }
    }

    inline void add(Key key, Value value) {
        if (count >= allocated) {
            grow();
        }

        auto hk = hash(key) & (allocated - 1);
        while (occupancy_mask[hk] && buckets[hk].key != key) {
            hk = (hk + 1) & (allocated - 1);
        }

        occupancy_mask[hk] = true;
        buckets[hk].key = key;
        buckets[hk].value = value;
        count++;
    }

    inline Value *get(Key key) {
        auto hk = hash(key) & (allocated - 1);
        for (int i = 0; i < allocated && occupancy_mask[hk] && buckets[hk].key != key; i++) {
            hk = (hk + 1) & (allocated - 1);
        }

        if (buckets && occupancy_mask[hk] && buckets[hk].key == key) {
            return &buckets[hk].value;
        } else {
            return nullptr;
        }
    }

    inline bool contains(Key key) {
        return get(key);
    }

    Value *operator[](Key key) {
        {
            Value *maybe_value = get(key);
            if (!maybe_value) {
                add(key, {});
            } else {
                return maybe_value;
            }
        }
        Value *maybe_value = get(key);
        assert(maybe_value);
        return maybe_value;
    }
};

template <typename Value>
struct String_Hash_Table {
    struct Bucket {
        char *key;
        Value value;
    };

    Bucket *buckets = nullptr;
    bool *occupancy_mask = nullptr;
    int allocated = 0;
    int count = 0;

    inline void grow() {
        const int HASH_TABLE_INITIAL_CAPACITY = 256;

        if (!buckets) {
            assert(allocated == 0);
            assert(count == 0);
            
            buckets = (Bucket *)calloc(HASH_TABLE_INITIAL_CAPACITY, sizeof(Bucket));
            occupancy_mask = (bool *)calloc(HASH_TABLE_INITIAL_CAPACITY, sizeof(bool));
            allocated = HASH_TABLE_INITIAL_CAPACITY;
            count = 0;
        } else {
            String_Hash_Table <Value> new_hash_table = {
                (Bucket *)calloc(allocated * 2, sizeof(Bucket)),
                (bool *)calloc(allocated * 2, sizeof(bool)),
                allocated * 2,
                0,
            };

            for (int i = 0; i < count; i++) {
                if (occupancy_mask[i]) {
                    new_hash_table.add(buckets[i].key, buckets[i].value);
                }
            }

            free(buckets);
            free(occupancy_mask);

            *this = new_hash_table;
        }
    }

    inline void add(char *key, Value value) {
        if (count >= allocated) {
            grow();
        }

        auto hk = hash(key) & (allocated - 1);
        while (occupancy_mask[hk] && !strings_match(buckets[hk].key, key)) {
            hk = (hk + 1) & (allocated - 1);
        }

        occupancy_mask[hk] = true;
        buckets[hk].key = copy_string(key);
        buckets[hk].value = value;
        count++;
    }

    inline Value *get(char *key) {
        auto hk = hash(key) & (allocated - 1);
        for (int i = 0; i < allocated && occupancy_mask[hk] && !strings_match(buckets[hk].key, key); i++) {
            hk = (hk + 1) & (allocated - 1);
        }

        if (buckets && occupancy_mask[hk] && strings_match(buckets[hk].key, key)) {
            return &buckets[hk].value;
        } else {
            return nullptr;
        }
    }

    inline bool contains(char *key) {
        return get(key);
    }

    Value *operator[](char *key) {
        {
            Value *maybe_value = get(key);
            if (!maybe_value) {
                add(key, {});
            } else {
                return maybe_value;
            }
        }
        Value *maybe_value = get(key);
        assert(maybe_value);
        return maybe_value;
    }
};

#endif
