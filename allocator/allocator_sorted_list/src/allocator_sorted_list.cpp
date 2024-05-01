#include <not_implemented.h>

#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    debug_with_guard(get_typename() + "::destructor| end");
    get_mutex().~mutex();
    deallocate_with_guard(_trusted_memory);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    debug_with_guard(get_typename() + "::move_copy | start");
    _trusted_memory = std::exchange(other._trusted_memory, nullptr);
    debug_with_guard(get_typename() + "::move_copy | finish");
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    debug_with_guard(get_typename() + "::assign_move | start");
    std::swap(_trusted_memory, other._trusted_memory);
    debug_with_guard(get_typename() + "::assign_move | finish");
    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    size_t meta_allocator = get_ancillary_space_size();
    size_t meta_block = get_size_meta_block();

    size_t requested_size = space_size + meta_allocator;

    if(parent_allocator == nullptr)
    {
        try
        {
            _trusted_memory = ::operator new(requested_size);
        }
        catch(std::bad_alloc& ex)
        {
            error_with_guard("bad alloc for allocator !");
            throw;
        }
    }
    else
    {
        _trusted_memory = parent_allocator->allocate(requested_size, 1);
    }

    uint8_t* tmp_ptr = reinterpret_cast<uint8_t*>(_trusted_memory);
    *reinterpret_cast<allocator**>(tmp_ptr) = parent_allocator;

    *reinterpret_cast<class logger**>(tmp_ptr + sizeof(allocator*)) = logger;

    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(tmp_ptr + sizeof(allocator*) + sizeof(class logger*)) = allocate_fit_mode;

    *reinterpret_cast<size_t*>(tmp_ptr + sizeof(allocator*) + sizeof(class logger*) + sizeof(allocate_fit_mode)) = space_size;

    tmp_ptr += sizeof(allocator*) + sizeof(class logger*) + sizeof(allocate_fit_mode) + sizeof(size_t);

    allocator::construct(reinterpret_cast<std::mutex *>(tmp_ptr));
    tmp_ptr += sizeof(std::mutex);

    *reinterpret_cast<void**>(tmp_ptr) = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(tmp_ptr) + sizeof(void*));
    tmp_ptr += sizeof(void*);

    *reinterpret_cast<size_t*>(tmp_ptr) = space_size - meta_block;

    tmp_ptr += sizeof(size_t);

    *reinterpret_cast<void**>(tmp_ptr) = nullptr;

    if (logger != nullptr) {
        debug_with_guard(get_typename() + "::constructor | finished");
    }
}

[[nodiscard]] void *allocator_sorted_list::allocate(
    size_t value_size,
    size_t values_count)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard(get_typename() + "::allocate | start");

    size_t requested_size = value_size * values_count;

    size_t meta_allocator = get_ancillary_space_size();

    void* previous_block = nullptr;
    void* current_block = nullptr;
    void* end_ptr = reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator + get_size_memory();

    switch (get_fit_mode())
    {
        case allocator_sorted_list::fit_mode::first_fit:
            current_block = get_first_suitable(requested_size);
            break;
        case allocator_sorted_list::fit_mode::the_best_fit:
            current_block = get_best_suitable(requested_size);
            break;
        case allocator_sorted_list::fit_mode::the_worst_fit:
            current_block = get_worst_suitable(requested_size);
            break;
    }

    if (current_block == nullptr)
    {
        error_with_guard(get_typename() + "allocate| Bad allocation for" + std::to_string(requested_size) + " bytes");
        throw std::bad_alloc();
    }

    previous_block = get_previous_for_free(current_block);

    void* next_block = get_ptr_from_block(current_block);

    size_t size_free_block = get_size_block(current_block);

    size_t meta_block = get_size_meta_block();

    if (size_free_block  < meta_block + requested_size)
    {
        debug_with_guard(get_typename() + " changed allocating block size to " + std::to_string(size_free_block));
        requested_size = size_free_block;
    }

    if (next_block == end_ptr)
    {
        next_block = nullptr;
    }

    *reinterpret_cast<size_t*>(current_block) = requested_size;
    *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(current_block) + sizeof(size_t)) = _trusted_memory;

    if (size_free_block >= meta_block + requested_size)
    {
        auto tmp_ptr = reinterpret_cast<uint8_t *>(current_block) + meta_block + get_size_block(current_block);
        *reinterpret_cast<size_t*>(tmp_ptr) = size_free_block - requested_size - meta_block;
        tmp_ptr += sizeof(size_t);
        *reinterpret_cast<void**>(tmp_ptr) = next_block;
        next_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(current_block) + meta_block + get_size_block(current_block));

    }

    if (previous_block == nullptr)
    {
        set_first_block(next_block);
    }
    else
    {
        *get_ptr_to_previous(previous_block) = next_block;
    }

    debug_with_guard(get_typename() + "   -> Available memory: " + std::to_string(get_available_memory()));

    debug_with_guard(get_blocks_info(get_blocks_info()));

    debug_with_guard(get_typename() + "::allocate | end");

    return reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(current_block) + meta_block);

}

void allocator_sorted_list::deallocate(
    void *at)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard(get_typename() + "::deallocate | start");

    size_t meta_block = get_size_meta_block();

    void* block_start = reinterpret_cast<uint8_t *>(at) - meta_block;

    if (get_ptr_from_block(block_start) != _trusted_memory)
    {
        error_with_guard("The memory was allocated from another allocator");
        throw std::logic_error("Block does not belong to allocator");
    }

    void* previous_free = get_previous_for_loaded(block_start);

    void* next_free;

    if (previous_free == nullptr)
    {
        next_free = get_first_block();
        set_first_block(block_start);
    }
    else
    {
        next_free = get_ptr_from_block(previous_free);
    }

    if(next_free != nullptr && reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(block_start) + meta_block + get_size_block(block_start)) == next_free)
    {
        auto tmp_ptr = reinterpret_cast<uint8_t *>(block_start);
        *reinterpret_cast<size_t*>(tmp_ptr) = get_size_block(block_start) + meta_block + get_size_block(next_free);
        tmp_ptr += sizeof(size_t);
        *reinterpret_cast<void**>(tmp_ptr) = get_ptr_from_block(next_free);
    }
    else
    {
        *get_ptr_to_previous(block_start) = next_free;
    }

    if (previous_free != nullptr)
    {
        *get_ptr_to_previous(previous_free) = block_start;
    }


    if(previous_free != nullptr && reinterpret_cast<uint8_t *>(previous_free) + meta_block + get_size_block(previous_free) == block_start )
    {
        *reinterpret_cast<size_t*>(previous_free) = get_size_block(previous_free) + meta_block + get_size_block(block_start);
        auto tmp_ptr = reinterpret_cast<uint8_t *>(previous_free) + sizeof(size_t);
        *reinterpret_cast<void**>(tmp_ptr) = get_ptr_from_block(block_start);
    }

    debug_with_guard(get_typename() +  "Available memory: " + std::to_string(get_available_memory()));

    debug_with_guard(get_blocks_info(get_blocks_info()));

    debug_with_guard(get_typename() + "::deallocator | end");
}

inline allocator *allocator_sorted_list::get_allocator() const
{
    return *reinterpret_cast<allocator**>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> info;
    size_t meta_allocator = get_ancillary_space_size();
    void* cur = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(_trusted_memory) + meta_allocator);

    void * prev = nullptr;
    void* cur_avail_block = get_first_block();
    size_t meta_block = get_size_meta_block();

    void* end_ptr = reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator + get_size_memory();

    if (cur_avail_block == nullptr) {
        allocator_test_utils::block_info block;
        block.block_size = get_size_memory();
        block.is_block_occupied = true;
        info.push_back(block);
        return info;
    }

    while (cur != nullptr && cur != end_ptr)
    {
        if (get_ptr_from_block(cur) != _trusted_memory)
        {
            allocator_test_utils::block_info block_avail;
            block_avail.block_size = get_size_block(cur);
            block_avail.is_block_occupied = false;
            info.push_back(block_avail);
        }
        else
        {
            allocator_test_utils::block_info block_busy;
            block_busy.block_size = get_size_block(cur);
            block_busy.is_block_occupied = true;
            info.push_back(block_busy);
        }

        cur = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(cur) + meta_block + get_size_block(cur));

    }

    return info;
}

inline logger *allocator_sorted_list::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*));
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "allocator_sorted_list";
}

/*
 allocator::block_size_t allocator_sorted_list::get_available_block_size(
    void *block_address) noexcept
{
    return *reinterpret_cast<allocator::block_size_t *>(reinterpret_cast<void **>(block_address) + 1);
}
*/

size_t allocator_sorted_list::get_ancillary_space_size() const noexcept
{
    return sizeof(allocator*) + sizeof(class logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t) + sizeof(std::mutex) + sizeof(void*);
}

size_t allocator_sorted_list::get_size_meta_block() const noexcept
{
    return sizeof(size_t) + sizeof(void*);
}

std::mutex& allocator_sorted_list::get_mutex() noexcept
{
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) +
                                          sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode));
}

allocator_with_fit_mode::fit_mode& allocator_sorted_list::get_fit_mode() const noexcept
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*));
}

std::string allocator_sorted_list::get_blocks_info(std::vector<allocator_test_utils::block_info> info) const noexcept
{
    std::string blocks = "Info from blocks:\n";
    int k = 1;
    for (auto &block : info) {
        std::string small_info = "\t" + std::to_string(k++) + ". <" + block_status(block.is_block_occupied) + "> <" + std::to_string(block.block_size) + ">\n";
        blocks += small_info;

    }
    return blocks;
}

inline void allocator_sorted_list::set_fit_mode(
        allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*) + sizeof(class logger*)) = mode;
}

std::string allocator_sorted_list::get_block_info(void * block) const noexcept {
    unsigned char * bytes = reinterpret_cast<unsigned char *>(block);
    size_t size = get_size_block(bytes - sizeof(size_t) - sizeof(allocator*) - 2 * sizeof(void*));
    std::string array = "";

    for (block_size_t i = 0; i < size; ++i) {
        array += std::to_string(*bytes);
        bytes += sizeof(unsigned char);
        array += ' ';

    }
    return array;
}

size_t allocator_sorted_list::get_size_memory() const noexcept
{
    return *reinterpret_cast<size_t*>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*)
                                      + sizeof (logger*) + sizeof (allocator_with_fit_mode::fit_mode));
}

size_t allocator_sorted_list::get_size_block(void* current_block) const noexcept
{
    return *reinterpret_cast<size_t*>(current_block);
}

std::string allocator_sorted_list::block_status(bool state) const noexcept {
    if (state) {
        return "occupied";
    }
    return "available";
}

void* allocator_sorted_list::get_ptr_from_block(void* current) const noexcept
{
    return  *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(current) + sizeof(size_t));
}

void* allocator_sorted_list::get_first_block() const noexcept
{
    auto temp_ptr = reinterpret_cast<uint8_t*>(_trusted_memory);
    temp_ptr += sizeof(logger*) + sizeof(allocator*) + sizeof(allocator_with_fit_mode::fit_mode) +sizeof(size_t) + sizeof(std::mutex);
    return *reinterpret_cast<void**>(temp_ptr);
}

inline void** allocator_sorted_list::get_ptr_to_previous(void* ptr) const noexcept
{
    return reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(ptr) + sizeof(size_t));
}

size_t allocator_sorted_list::get_available_memory() noexcept
{
    void* current_block = get_first_block();
    size_t available_memory = 0;

    while (current_block != nullptr)
    {
        available_memory += get_size_block(current_block);

        current_block = get_ptr_from_block(current_block);
    }

    return available_memory;
}

void* allocator_sorted_list::get_first_suitable(size_t requested_size)  noexcept
{

    void* current_block = get_first_block();

    if (current_block == nullptr)
    {
        return current_block;
    }

    while (current_block != nullptr)
    {
        size_t size_block = get_size_block(current_block);

        if (size_block >= requested_size)
        {
            return current_block;
        }

        current_block = get_ptr_from_block(current_block);
    }

    return nullptr;
}

void* allocator_sorted_list::get_worst_suitable(size_t requested_size)  noexcept
{
    void* result = nullptr;
    size_t max = 0;
    void* current_block = get_first_block();

    if (current_block == nullptr)
    {
        return current_block;
    }

    while(current_block != nullptr)
    {
        size_t current_size = get_size_block(current_block);
        if(current_size >= requested_size && current_size > max)
        {
            max = current_size;
            result = current_block;
        }
        current_block = get_ptr_from_block(current_block);
    }

    return result;
}

void* allocator_sorted_list::get_best_suitable(size_t requested_size)  noexcept
{
    void* result = nullptr;
    size_t max = get_size_memory();
    void* current_block = get_first_block();

    if (current_block == nullptr)
    {
        return current_block;
    }

    while(current_block != nullptr)
    {
        size_t current_size = get_size_block(current_block);
        if(current_size >= requested_size && current_size < max)
        {
            max = current_size;
            result = current_block;
        }
        current_block = get_ptr_from_block(current_block);
    }

    return result;
}

void* allocator_sorted_list::get_previous_for_loaded(void* loaded_ptr) const noexcept
{
    void* cur = get_first_block();
    void* prev = nullptr;

    while (cur != nullptr)
    {
        ptrdiff_t diff = reinterpret_cast<uint8_t*>(cur) - reinterpret_cast<uint8_t *>(loaded_ptr);

        if(diff > 0)
        {
            return prev;
        }

        prev = cur;
        cur = get_ptr_from_block(cur);
    }
    return prev;
}

void* allocator_sorted_list::get_previous_for_free(void* free_ptr) const noexcept
{
    void* cur = get_first_block();
    void* prev = cur;

    if (cur == free_ptr)
    {
        return nullptr;
    }

    while (cur != free_ptr)
    {
        prev = cur;
        cur = get_ptr_from_block(cur);
    }
    return prev;
}

void allocator_sorted_list::set_first_block(void* block) const noexcept {
    *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) +
                              sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex)) = block;
}