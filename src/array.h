#ifndef ARRAY_H
#define ARRAY_H

#include <assert.h>
#include <new>

template<typename T>
struct Array {
    T *data = NULL;
    int count = 0;
    int allocated = 0;

    inline void reserve(int newSize) {
        if (newSize > allocated) {
            resize(newSize);
        }
    }
    
    inline ~Array() {
        if (data) {
            ::operator delete(data, allocated * sizeof(T));
        }
    }
    
    inline void add(const T& value) {
        if (count >= allocated) {
            if (!allocated) {
                allocated = 16;
            }
            resize(allocated * 2);
        }
        
        data[count] = value;
        count++;
    }

    inline T *add() {
        if (count >= allocated) {
            if (!allocated) {
                allocated = 16;
            }
            resize(allocated * 2);
        }

        T tmp = {};
        data[count] = tmp;
        T *result = &data[count++];
        return result;
    }
    
    inline void resize(int newSize) {
        T *new_block = (T *)::operator new(newSize * sizeof(T));

        if (newSize < count) {
            count = newSize;
        }
        
        for (int i = 0; i < count; i++) {
            new_block[i] = data[i];
        }
        
        if (data) {
            ::operator delete(data, allocated * sizeof(T));
        }
        data = new_block;
        allocated = newSize;
    }

    inline T remove_nth(int index) {
        T result = data[index];
        data[index] = data[count - 1];
        count--;

        return result;
    }
    
    inline const T &get(int index) const {
        assert(index < count);
        
        return data[index];
    }

    inline T &get(int index) {
        assert(index < count);
        
        return data[index];
    }
    
    inline const T &operator[](int index) const {
        assert(index < count);
        
        return data[index];
    }
    
    inline T &operator[](int index) {
        assert(index < count);
        
        return data[index];
    }
};

#endif
