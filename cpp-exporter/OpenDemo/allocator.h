#pragma once

#include "types.h"

#include <cassert>
#include <cstdlib>

template<class Type>
class Allocator
{
public:

    Allocator ();
    ~Allocator ();

    void Clear ();
    void Delete (Type* ptr);

    bool Initialize (size_t elementCount);

    bool IsFull () const;
    bool IsValid () const;

    Type* New ();
    void Reset ();

    bool Validate (Type* ptr) const;

protected:

    uint8* m_block;
    uint8* m_free;
    uint8* m_end;
    size_t m_sizeInBytes;
    size_t m_sizeInElements;
};

template<class Type>
inline Allocator<Type>::Allocator ()
{
    m_block = NULL;
    m_free = NULL;
    m_end = NULL;
    m_sizeInBytes = 0;
    m_sizeInElements = 0;
}

template<class Type>
inline Allocator<Type>::~Allocator ()
{
    delete[] m_block;
}

template<class Type>
void Allocator<Type>::Clear ()
{
    if (!IsValid())
    {
        return;
    }

    delete[] m_block;

    m_block = NULL;
    m_free = NULL;
    m_end = NULL;
    m_sizeInBytes = 0;
    m_sizeInElements = 0;
}

template<class Type>
inline void Allocator<Type>::Delete (Type* ptr)
{
    assert(Validate(ptr));
    *(void**)ptr = m_free;
    m_free = ptr;
}

template<class Type>
inline bool Allocator<Type>::Initialize (size_t elementCount)
{
    assert(sizeof(Type) >= sizeof(void*));
    if (IsValid())
    {
        delete[] m_block;
    }

    size_t sizeInBytes = (sizeof(Type) * elementCount);
    m_block = new uint8[sizeInBytes]; // Avoid constructors
    if (m_block == NULL)
    {
        return false;
    }

    m_sizeInElements = elementCount;
    m_end = (m_block + sizeInBytes);
    m_sizeInBytes = sizeInBytes;

    Reset();
    return true;
}

template<class Type>
inline bool Allocator<Type>::IsFull () const
{
    return (m_free == NULL);
}

template<class Type>
inline bool Allocator<Type>::IsValid () const
{
    return (m_block != NULL);
}

template<class Type>
inline Type* Allocator<Type>::New ()
{
    assert(IsValid());
    if (IsFull())
    {
        return NULL;
    }

    Type* ptr = (Type*)m_free;
    m_free = *(uint8**)m_free;
    return ptr;
}

template<class Type>
inline void Allocator<Type>::Reset ()
{
    size_t iEnd = (m_sizeInElements - 1);
    Type* ptr = (Type*)m_block;
    for (size_t i = 0; i < iEnd; i++, ptr++)
    {
        *(void**)ptr = (ptr + 1);
    }
    *(void**)ptr = NULL;
    m_free = m_block;
}

template<class Type>
inline bool Allocator<Type>::Validate (Type* ptr) const
{
    return (ptr >= m_block && ptr < m_end);
}
