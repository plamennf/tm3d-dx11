#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>
#include <new>

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
    
    int allocated = 0;
    int count = 0;
    Bucket *buckets = nullptr;
    bool *occupancy_mask = nullptr;

    inline ~Hash_Table() {
        if (buckets) {
            ::operator delete(buckets, allocated * sizeof(Bucket));
            buckets = nullptr;
        }

        if (occupancy_mask) {
            ::operator delete(occupancy_mask, allocated * sizeof(bool));
            occupancy_mask = nullptr;
        }
    }
    
    inline void resize(int newSize) {
        Bucket *new_block = (Bucket *)::operator new(newSize * sizeof(Bucket));
        bool *new_occupancies = (bool *)::operator new(newSize * sizeof(bool));
        memset(new_occupancies, 0, newSize * sizeof(bool));
        
        if (newSize < count) {
            count = newSize;
        }

        for (int i = 0; i < count; i++) {
            new_block[i] = buckets[i];
            new_occupancies[i] = occupancy_mask[i];
        }

        if (buckets) {
            ::operator delete(buckets, allocated * sizeof(Bucket));
            ::operator delete(occupancy_mask, allocated * sizeof(bool));
        }

        buckets = new_block;
        occupancy_mask = new_occupancies;

        allocated = newSize;
    }

    inline void add(Key key, Value value) {
        if (count >= allocated) {
            if (!allocated) {
                allocated = 16;
            }
            resize(allocated * 2);
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

    Value &get(Key key) {
        auto hk = hash(key) & (allocated - 1);
        for (int i = 0; i < allocated && occupancy_mask[hk] && buckets[hk].key != key; i++) {
            hk = (hk + 1) & (allocated - 1);            
        }

        return buckets[hk].value;
    }

    bool contains(Key key) {
        if (!allocated) return 0;
        
        auto hk = hash(key) & (allocated - 1);
        for (int i = 0; i < allocated && occupancy_mask[hk] && buckets[hk].key != key; i++) {
            hk = (hk + 1) & (allocated - 1);            
        }

        return occupancy_mask[hk];
    }
};

template <typename Value>
struct String_Hash_Table {
    struct Bucket {
        char *key;
        Value value;
    };
    
    int allocated = 0;
    int count = 0;
    Bucket *buckets = nullptr;
    bool *occupancy_mask = nullptr;

    inline ~String_Hash_Table() {
        if (buckets) {
            ::operator delete(buckets, allocated * sizeof(Bucket));
            buckets = nullptr;
        }

        if (occupancy_mask) {
            ::operator delete(occupancy_mask, allocated * sizeof(bool));
            occupancy_mask = nullptr;
        }
    }
    
    inline void resize(int newSize) {
        Bucket *new_block = (Bucket *)::operator new(newSize * sizeof(Bucket));
        bool *new_occupancies = (bool *)::operator new(newSize * sizeof(bool));
        memset(new_occupancies, 0, newSize * sizeof(bool));
        
        if (newSize < count) {
            count = newSize;
        }

        for (int i = 0; i < count; i++) {
            new_block[i] = buckets[i];
            new_occupancies[i] = occupancy_mask[i];
        }

        if (buckets) {
            ::operator delete(buckets, allocated * sizeof(Bucket));
            ::operator delete(occupancy_mask, allocated * sizeof(bool));
        }

        buckets = new_block;
        occupancy_mask = new_occupancies;

        allocated = newSize;
    }

    inline void add(char *key, Value value) {
        if (count >= allocated) {
            if (!allocated) {
                allocated = 16;
            }
            resize(allocated * 2);
        }

        extern bool strings_match(char *a, char *b);
        
        auto hk = hash(key) & (allocated - 1);
        while (occupancy_mask[hk] && !strings_match(buckets[hk].key, key)) {
            hk = (hk + 1) & (allocated - 1);
        }

        occupancy_mask[hk] = true;

        buckets[hk].key = copy_string(key);
        buckets[hk].value = value;

        count++;
    }

    Value &get(char *key) {
        extern bool strings_match(char *a, char *b);
        
        auto hk = hash(key) & (allocated - 1);
        for (int i = 0; i < allocated && occupancy_mask[hk] && !strings_match(buckets[hk].key, key); i++) {
            hk = (hk + 1) & (allocated - 1);            
        }

        return buckets[hk].value;
    }

    bool contains(char *key) {
        if (!allocated) return 0;

        extern bool strings_match(char *a, char *b);
        
        auto hk = hash(key) & (allocated - 1);
        for (int i = 0; i < allocated && occupancy_mask[hk] && !strings_match(buckets[hk].key, key); i++) {
            hk = (hk + 1) & (allocated - 1);            
        }

        return occupancy_mask[hk];
    }
};

#endif
