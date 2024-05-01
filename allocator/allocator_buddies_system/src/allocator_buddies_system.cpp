#include <not_implemented.h>

#include "../include/allocator_buddies_system.h"

allocator_buddies_system::~allocator_buddies_system()
{
    debug_with_guard(get_typename() + "::destructor");
    get_mutex().~mutex();
    deallocate_with_guard(_trusted_memory);
}

allocator_buddies_system::allocator_buddies_system(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{

    size_t min_space_block = get_meta_block();

    if (space_size < min_space_block)
    {
        throw std::logic_error("not enough space to constructor the allocator");
    }

    size_t meta_allocator = get_ancillary_space_size();

    space_size = size_in_power(two_degree(space_size));

    size_t requested_size = space_size + meta_allocator;

    if(parent_allocator == nullptr)
    {
        try
        {
            _trusted_memory = ::operator new(requested_size);
        }
        catch(std::bad_alloc& ex)
        {
            error_with_guard("Bad allacation from global heap!");
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

    *reinterpret_cast<uint8_t *>(tmp_ptr + sizeof(allocator*) + sizeof(class logger*) + sizeof(allocate_fit_mode)) = two_degree(space_size);

    tmp_ptr += sizeof(allocator*) + sizeof(class logger*) + sizeof(allocate_fit_mode) + sizeof(uint8_t);

    allocator::construct(reinterpret_cast<std::mutex *>(tmp_ptr));
    tmp_ptr += sizeof(std::mutex);

    auto first_block = reinterpret_cast<byte_field*>(tmp_ptr);

    first_block->is_occup = false;
    first_block->size = two_degree(space_size);

    if (logger != nullptr) {
        debug_with_guard(get_typename() + " new space " + std::to_string(space_size) + " is  now");
        debug_with_guard(get_typename() + "::constructor | finished");
    }
}

[[nodiscard]] void *allocator_buddies_system::allocate(
    size_t value_size,
    size_t values_count)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard(get_typename() + "::allocate | start");

    size_t min_space_block = get_meta_block();

    size_t requested_size = value_size * values_count + min_space_block;

    size_t meta_allocator = get_ancillary_space_size();

    void* current_block = nullptr;

    switch(get_fit_mode())
    {
        case allocator_with_fit_mode::fit_mode::first_fit:
            current_block = get_first_suitable(requested_size);
            break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            current_block = get_best_suitable(requested_size);
            break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            current_block = get_worst_suitable(requested_size);
            break;
    }

    if (current_block == nullptr)
    {
        error_with_guard(get_typename() + "Bad allacation for " + std::to_string(requested_size) + " bytes");
        throw std::bad_alloc();
    }

    while(get_block_size(current_block) - min_space_block >= requested_size * 2)
    {
        auto first_twin = reinterpret_cast<byte_field*>(current_block);
        --first_twin->size;
        auto* second_twin = reinterpret_cast<byte_field*>(get_block_twin(current_block));
        second_twin->is_occup = false;
        second_twin->size = first_twin->size;
    }

    if(get_block_size(current_block) - min_space_block != requested_size)
    {
        debug_with_guard("Allocator buddies_system changed allocating block size to " + std::to_string(
                get_block_size(current_block) - min_space_block));
    }

    auto first_twin = reinterpret_cast<byte_field*>(current_block);
    first_twin->is_occup = true;
    *reinterpret_cast<void**>(reinterpret_cast<uint8_t *>(current_block) + sizeof (byte_field)) = _trusted_memory;

    debug_with_guard(get_typename() + "   -> Available memory: " + std::to_string(get_available_memory()));

    debug_with_guard(get_blocks_info(get_blocks_info()));

    debug_with_guard(get_typename() + "::allocate | end");

    return reinterpret_cast<uint8_t *>(current_block) + min_space_block;
}

void allocator_buddies_system::deallocate(
    void *at)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard(get_typename() + "::deallocate | start");

    size_t meta_block = get_meta_block();

    void* block_start = reinterpret_cast<uint8_t *>(at) - meta_block;

    if (get_allocator_block(block_start) != _trusted_memory)
    {
        error_with_guard("The memory was allocated from another allocator");
        throw std::logic_error("Block does not belong to allocator");
    }

    size_t block_start_size = get_block_size(block_start) - meta_block;

    reinterpret_cast<byte_field*>(block_start)->is_occup = false;

    void* twin = get_block_twin(block_start);

    while(get_block_size(block_start) < get_size_memory() && get_block_size(block_start) == get_block_size(twin) && !is_busy(twin))
    {
        void* tmp_ptr = block_start < twin ? block_start : twin;

        auto current_meta = reinterpret_cast<byte_field*>(tmp_ptr);
        ++current_meta->size;

        block_start = tmp_ptr;
        twin = get_block_twin(block_start);
    }

    debug_with_guard(get_typename() +  "Available memory: " + std::to_string (get_available_memory()));

    debug_with_guard(get_blocks_info(get_blocks_info()));

    debug_with_guard(get_typename() + "::deallocator | end");

}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*)) = mode;
}

inline allocator *allocator_buddies_system::get_allocator() const
{
    return *reinterpret_cast<allocator**>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> info;
    size_t meta_allocator = get_ancillary_space_size();
    size_t meta_block = get_meta_block();
    void* current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator);
    void* end_ptr = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator + get_size_memory());
    size_t available_memory = 0;

    while (current_block < end_ptr)
    {
        if (!is_busy(current_block))
        {
            allocator_test_utils::block_info block;
            block.block_size = get_block_size(current_block);
            block.is_block_occupied = false;
            info.push_back(std::move(block));
        }
        else
        {
            allocator_test_utils::block_info block;
            block.block_size = get_block_size(current_block);
            block.is_block_occupied = true;
            info.push_back(std::move(block));
        }
        current_block = reinterpret_cast<uint8_t*>(current_block) + get_block_size(current_block);
    }

    return info;
}

inline logger *allocator_buddies_system::get_logger() const
{
    return *reinterpret_cast<logger**>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*));
}

inline std::string allocator_buddies_system::get_typename() const noexcept
{
    return "allocator_buddies_system";
}

size_t allocator_buddies_system::get_meta_block() const noexcept
{
    return sizeof(byte_field) + sizeof(void*);
}

size_t allocator_buddies_system::get_ancillary_space_size() const noexcept
{
    return sizeof(allocator*) + sizeof(class logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(uint8_t) + sizeof(std::mutex);
}

uint8_t allocator_buddies_system::two_degree(size_t size) const noexcept
{
    size_t number = 1;
    uint8_t degree = 0;

    while (number < size)
    {
        number <<= 1;
        ++degree;
    }

    return  degree;
}

allocator_with_fit_mode::fit_mode& allocator_buddies_system::get_fit_mode() const noexcept
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*));
}

std::mutex& allocator_buddies_system::get_mutex() noexcept
{
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<uint8_t *>(_trusted_memory) + sizeof(uint8_t) + sizeof(allocator*) +
                                          sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode));
}

std::string allocator_buddies_system::get_blocks_info(std::vector<allocator_test_utils::block_info> info) const noexcept
{
    std::string blocks = "Info from blocks:\n";
    int k = 1;
    for (auto &block : info) {
        std::string small_info = "\t" + std::to_string(k++) + ". <" + block_status(block.is_block_occupied) + "> <" + std::to_string(block.block_size) + ">\n";
        blocks += small_info;

    }
    return blocks;
}

std::string allocator_buddies_system::get_block_info(void * block) const noexcept {
    unsigned char * bytes = reinterpret_cast<unsigned char *>(block);
    size_t meta_block = get_meta_block();
    size_t size = get_block_size(bytes - meta_block);
    std::string array = "";

    for (block_size_t i = 0; i < size; ++i) {
        array += std::to_string(*bytes);
        bytes += sizeof(unsigned char);
        array += ' ';

    }
    return array;
}

std::string allocator_buddies_system::block_status(bool state) const noexcept {
    if (state) {
        return "occupied";
    }
    return "available";
}

size_t allocator_buddies_system::get_size_memory() const noexcept
{
    return size_in_power(*(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*)
                                      + sizeof (logger*) + sizeof (allocator_with_fit_mode::fit_mode)));
}

size_t allocator_buddies_system::get_block_size(void* current_block) const noexcept
{
        return size_in_power(reinterpret_cast<byte_field*>(current_block)->size);
}

inline allocator *allocator_buddies_system::get_allocator_block(void* current_block) const noexcept
{
    return *reinterpret_cast<allocator**>(reinterpret_cast<uint8_t *>(current_block) + sizeof(byte_field));
}

bool allocator_buddies_system::is_busy(void* current_block) const noexcept
{
    return reinterpret_cast<byte_field*>(current_block)->is_occup;
}

size_t allocator_buddies_system::size_in_power(size_t power_of_2) const noexcept
{
    constexpr const size_t number = 1;
    return number << power_of_2;
}

void* allocator_buddies_system::get_block_twin(void* current_block) noexcept
{
    size_t relative_distance = reinterpret_cast<uint8_t *>(current_block) - (reinterpret_cast<uint8_t *>(_trusted_memory) + get_ancillary_space_size());


    size_t distance_to_twin = relative_distance ^ (static_cast<size_t>(1u) << two_degree(get_block_size(current_block)));

    return reinterpret_cast<void*>(reinterpret_cast<std::uint8_t *>(_trusted_memory) + get_ancillary_space_size() + distance_to_twin);
}

size_t allocator_buddies_system::get_available_memory()  const noexcept
{
    size_t meta_allocator = get_ancillary_space_size();
    size_t meta_block = get_meta_block();
    void* current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator);
    void* end_ptr = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator + get_size_memory());
    size_t available_memory = 0;

    while (current_block < end_ptr)
    {
        if (!is_busy(current_block))
        {
            available_memory += get_block_size(current_block);
        }
        current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current_block) + get_block_size(current_block));
    }

    return available_memory;
}

void* allocator_buddies_system::get_first_suitable(size_t requested_size)  const noexcept
{
    size_t meta_allocator = get_ancillary_space_size();
    size_t meta_block = get_meta_block();
    void* current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator);
    void* end_ptr = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator + get_size_memory());

    while(current_block < end_ptr)
    {
        if(!is_busy(current_block) && (get_block_size(current_block) - meta_block) >= requested_size)
        {
            return current_block;
        }
        current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current_block) + get_block_size(current_block));
    }
    return nullptr;
}

void* allocator_buddies_system::get_worst_suitable(size_t requested_size) const noexcept
{
    size_t meta_allocator = get_ancillary_space_size();
    size_t meta_block = get_meta_block();
    void* current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator);
    void* end_ptr = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator + get_size_memory());
    size_t block_memory_max = 0;
    void* result = nullptr;

    while(current_block < end_ptr)
    {
        if(!is_busy(current_block) && (get_block_size(current_block) - meta_block) >= requested_size && (get_block_size(current_block) - meta_block) >= block_memory_max)
        {
            block_memory_max = get_block_size(current_block);
            result = current_block;
        }
        current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current_block) + get_block_size(current_block));
    }

    return result;
}

void* allocator_buddies_system::get_best_suitable(size_t requested_size)  const noexcept
{
    size_t meta_allocator = get_ancillary_space_size();
    size_t meta_block = get_meta_block();
    void* current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator);
    void* end_ptr = reinterpret_cast<void*>(reinterpret_cast<uint8_t *>(_trusted_memory) + meta_allocator + get_size_memory());
    void* result = nullptr;
    size_t maximum_memory = get_size_memory();

    while(current_block < end_ptr)
    {
        if(!is_busy(current_block) && (get_block_size(current_block) - meta_block) >= requested_size && (get_block_size(current_block) - meta_block) <= maximum_memory)
        {
            maximum_memory = get_block_size(current_block);
            result = current_block;
        }
        current_block = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current_block) + get_block_size(current_block));
    }
    return result;
}

