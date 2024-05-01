#include <cstddef>
#include <iostream>
#include <stdexcept>

#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap(
    logger* logger) : _logger(logger) 
    {
        debug_with_guard(get_typename() + "::constructor");
    }

allocator_global_heap::~allocator_global_heap() {
        debug_with_guard(get_typename() + "::destructor");
    }


[[nodiscard]] void* allocator_global_heap::allocate(
    size_t value_size, size_t values_count) 
{
    debug_with_guard(get_typename() + "::allocate");
    auto common_size = value_size * values_count;
    void* _trusted_memory;
    try {
        _trusted_memory = ::operator new(common_size + sizeof(size_t) + sizeof(allocator_global_heap*));
    } catch (std::bad_alloc const &exception) //
    {
        debug_with_guard(get_typename() + "::allocate | not_enough_memmory");
        throw ; //
    }
    uint8_t* tmp_ptr = reinterpret_cast<uint8_t*>(_trusted_memory);
    *reinterpret_cast<allocator**>(tmp_ptr) = this;
    
    *reinterpret_cast<size_t*>(tmp_ptr + sizeof(allocator*)) = common_size;
    block_pointer_t final_ptr = reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*);
 
    trace_with_guard(get_typename() + "::allocate - Memory allocated");
    return final_ptr;
}

void allocator_global_heap::deallocate(
    void *at)
{
    debug_with_guard(get_typename() + "::deallocate.start");
    if (at == nullptr)
    {
        debug_with_guard(get_typename() + "deallocate.end");
        throw std::logic_error(get_typename() + "Deallocation error | nullptr");
    }
    block_pointer_t block_start_ptr = reinterpret_cast<uint8_t*>(at) - sizeof(allocator*) - sizeof(size_t);
    if (*(reinterpret_cast<allocator_global_heap**>(block_start_ptr)) != this) {
        error_with_guard(get_typename() + "::deallocate - Logic error");
        debug_with_guard(get_typename() + "::deallocate.finish");
        throw std::logic_error(get_typename() + "Deallocation error | another allocao");
    }
    
    block_size_t block_size = (*(reinterpret_cast<uint8_t *>(at) - sizeof(size_t)));
    std::string bytes; // base function
    int* pointer_bytes = reinterpret_cast<int*>(at);

    if (*pointer_bytes == 0)
        bytes += '0';

    for (block_size_t i = 0; i < block_size; i++)
    {
        bytes += std::to_string(*pointer_bytes++);
        if (i != block_size - 1)
            bytes += ' ';
    }
    debug_with_guard("Bytes to free: " + bytes);
    ::operator delete(block_start_ptr);
    debug_with_guard(get_typename() + "::deallocate.end");
}

inline logger *allocator_global_heap::get_logger() const
{
    return _logger;
}

inline std::string allocator_global_heap::get_typename() const noexcept
{
    trace_with_guard("allocator_global_heap::get_typename");
    return "allocator_global_heap";
}