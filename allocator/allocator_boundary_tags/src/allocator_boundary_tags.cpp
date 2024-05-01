#include <not_implemented.h>

#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    debug_with_guard(get_typename() + "::destructor | start");
    logger * _logger = get_logger();
    allocator::destruct(&get_mutex());
    if (_logger != nullptr) {
        _logger->debug(get_typename() + "::destructor | end");
    }
}


allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{

    if (logger != nullptr) {
        logger->debug(get_typename()  + "::constructor | finished");
    }

    size_t meta_size = get_ancillary_space_size();
    size_t block_meta_size = sizeof(allocator *) + 2 * sizeof(void*) + sizeof(size_t);

    if (space_size < block_meta_size + sizeof(void*))
    {
        if (logger != nullptr)
        {
            error_with_guard(get_typename() + "::constructor_allocate| Not enough space");
        }
        throw std::logic_error("can't initialize allocator instance");
    }

    auto common_size = space_size + meta_size;
    try
    {
        _trusted_memory = parent_allocator == nullptr
                          ? ::operator new(common_size)
                          : parent_allocator->allocate(common_size, 1);
    }
    catch (std::bad_alloc const &ex)
    {
        error_with_guard(get_typename() + "::constructor_allocate| Not enough space");
    }

    uint8_t* tmp_ptr = reinterpret_cast<uint8_t*>(_trusted_memory);
    *reinterpret_cast<allocator**>(tmp_ptr) = parent_allocator;

    *reinterpret_cast<class logger**>(tmp_ptr + sizeof(allocator*)) = logger;

    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(tmp_ptr + sizeof(allocator*) + sizeof(class logger*)) = allocate_fit_mode;

    *reinterpret_cast<size_t*>(tmp_ptr + sizeof(allocator*) + sizeof(class logger*) + sizeof(allocate_fit_mode)) = space_size;

    tmp_ptr += sizeof(allocator*) + sizeof(class logger*) + sizeof(allocate_fit_mode) + sizeof(size_t);

    allocator::construct(reinterpret_cast<std::mutex *>(tmp_ptr));
    tmp_ptr += sizeof(std::mutex);

    *reinterpret_cast<void**>(tmp_ptr) = nullptr;
    tmp_ptr += sizeof(void*);

    // *first_block_address_space_address = reinterpret_cast<void *>(first_block_address_space_address + 1);

    //*reinterpret_cast<size_t *>(reinterpret_cast<void **>(*first_block_address_space_address) + 1) = space_size - sizeof(block_size_t);

    if (logger != nullptr) {
        debug_with_guard(get_typename() + "::constructor | finished");
    }
}

[[nodiscard]] void *allocator_boundary_tags::allocate(
    size_t value_size,
    size_t values_count)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());

    debug_with_guard(get_typename() + "::allocate | start");

    auto asked_size = value_size * values_count;

    block_size_t min_size_block = sizeof(size_t) + 2 * sizeof(void*) + sizeof(void*);

    auto requested_size = asked_size + min_size_block;

    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

    void *target_block = nullptr;
    void *previous_to_target_block = nullptr;
    void *next_to_target_block = nullptr;
    void* end_ptr = get_end_ptr();

    {
        void *prev_busy_block = nullptr;
        void *current_busy_block = get_first_busy_block();
        size_t prev_size = 0;
        size_t available_block_size = get_available_memory();
        void * current_avail_block = nullptr;

        if (available_block_size < requested_size)
        {
            error_with_guard(get_typename() + "::allocate| Not enough space");
            throw std::logic_error("not enough space");
        }

        if (current_busy_block == nullptr && available_block_size >= requested_size)
        {
            target_block = get_first_aviable_block();
        }

        while (current_busy_block != nullptr && current_busy_block != end_ptr)
        {
            if (prev_busy_block == nullptr)
                current_avail_block = get_first_aviable_block();
            else
                current_avail_block = reinterpret_cast<uint8_t *>(prev_busy_block) + get_size_block((prev_busy_block));

            if (current_avail_block != current_busy_block)
            {
                available_block_size = reinterpret_cast<unsigned char*>(current_busy_block) - reinterpret_cast<unsigned char *>(current_avail_block);
                if (available_block_size >= requested_size)
                {
                    if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit)
                    {
                        debug_with_guard(get_typename() + "::allocate | first_fit");
                        target_block = current_avail_block;
                        prev_size = available_block_size;
                        previous_to_target_block = prev_busy_block;
                        next_to_target_block = current_busy_block;
                        break;
                    } else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit)
                    {
                        debug_with_guard(get_typename() + "::allocate | best_fit");
                        if (available_block_size < prev_size || prev_size == 0)
                        {
                            target_block = current_avail_block;
                            prev_size = available_block_size;
                            previous_to_target_block = prev_busy_block;
                            next_to_target_block = current_busy_block;
                        }
                    } else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit)
                    {
                        debug_with_guard(get_typename() + "::allocate | worst_fit");
                        if (available_block_size > prev_size)
                        {
                            target_block = current_avail_block;
                            prev_size = available_block_size;
                            previous_to_target_block = prev_busy_block;
                            next_to_target_block = current_busy_block;
                        }
                    }
                }
            }

            prev_busy_block = current_busy_block;
            current_busy_block = get_next_block((current_busy_block));
        }

        if ((current_busy_block == nullptr || current_busy_block == end_ptr)  && prev_busy_block != nullptr ) {
            current_avail_block = reinterpret_cast<unsigned char *>(prev_busy_block)  + get_size_block(prev_busy_block);
            available_block_size = reinterpret_cast<unsigned char*>(get_end_ptr()) - reinterpret_cast<unsigned char *>(current_avail_block);
            if (available_block_size >= requested_size) {
                if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit && target_block == nullptr)
                {
                    target_block = current_avail_block;
                    prev_size = available_block_size;
                    previous_to_target_block = prev_busy_block;
                    next_to_target_block = current_busy_block;
                } else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit) {
                    if (available_block_size < prev_size || prev_size == 0) {
                        target_block = current_avail_block;
                        prev_size = available_block_size;
                        previous_to_target_block = prev_busy_block;
                        next_to_target_block = current_busy_block;
                    }
                } else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit) {
                    if (available_block_size > prev_size) {
                        target_block = current_avail_block;
                        prev_size = available_block_size;
                        previous_to_target_block = prev_busy_block;
                        next_to_target_block = current_busy_block;
                    }
                }
            }
        }

        if (target_block == nullptr)  {
            error_with_guard(get_typename() + " no space to allocate\n");
            throw std::bad_alloc();
        }

        if (current_busy_block == nullptr && prev_busy_block == nullptr) {
            prev_size = get_size_memory();
        }

        size_t blocks_sizes_difference = prev_size - (requested_size);
        if (blocks_sizes_difference > 0 && blocks_sizes_difference < min_size_block)
        {
            requested_size += blocks_sizes_difference;
            warning_with_guard(get_typename() + " size of needed block has changed\n");
        }

        merge_blocks(previous_to_target_block, target_block);
        merge_blocks(target_block, next_to_target_block);

        *reinterpret_cast<allocator**>((target_block)) = this;
        *reinterpret_cast<size_t*>(reinterpret_cast<uint8_t *>(target_block) + sizeof(allocator*)) = requested_size;

        void * result_ptr = reinterpret_cast<unsigned char *>(target_block) + min_size_block;

        information_with_guard(get_typename() + "   -> Available memory: " + std::to_string(get_available_memory()));

        debug_with_guard(get_blocks_info(get_blocks_info()));

        debug_with_guard(get_typename() + "::allocate | end");
        return result_ptr;
    }
}


void allocator_boundary_tags::deallocate(void *at)
{
    debug_with_guard(get_typename() + "::deallocate | start");
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    if (at == nullptr)
    {
        error_with_guard(get_typename() + "::deallocate | Pointer Nullptr!");
        return;
    }

    std::string block_info_array = get_block_info(at);
    debug_with_guard(get_typename() + "" + block_info_array);

    auto meta_data = 2 * sizeof(void*) + sizeof(size_t) + sizeof(allocator*);
    unsigned char * block = reinterpret_cast<unsigned char *>(at) - meta_data;

    if (this != get_allocator_from_block(block)) {
        error_with_guard(get_typename() + "::deallocate | Handle deallocation of memory from another allocator");
        throw std::logic_error("can't deallocate memmory of another allocator");
    }

    void *previous_block_ptr = get_prev_block(block);
    void *next_block_ptr = get_next_block(block);

    merge_blocks(previous_block_ptr, next_block_ptr);

    //clear_block(block);

    information_with_guard(get_typename() +  "Available memory: " + std::to_string(get_available_memory()))->

    debug_with_guard(get_blocks_info(get_blocks_info()))->

    debug_with_guard(get_typename() + "::deallocator | end");
}

void allocator_boundary_tags::clear_block(void * block) const noexcept {
    unsigned char* block_ptr = reinterpret_cast<unsigned char *>(block);
    allocator** allocator_ptr = reinterpret_cast<allocator**>(block);
    *allocator_ptr = nullptr;

    block_ptr += sizeof(allocator*);
    size_t* block_size = reinterpret_cast<size_t *>(block_ptr);
    block_size = nullptr;

    block_ptr += sizeof(size_t);
    void** prev = reinterpret_cast<void**>(block_ptr);
    *prev = nullptr;

    block_ptr += sizeof(void*);
    void** next = reinterpret_cast<void**>(block_ptr);
    *next = nullptr;
}


inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> mutex_guard(get_mutex());
    *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*) + sizeof(class logger*)) = mode;
}

inline allocator *allocator_boundary_tags::get_allocator() const
{
    return reinterpret_cast<allocator*>(_trusted_memory);
}

std::string allocator_boundary_tags::get_blocks_info(std::vector<allocator_test_utils::block_info> info) const noexcept
{
    std::string blocks = "Info from blocks:\n";
    int k = 1;
    for (auto &block : info) {
        std::string small_info = "\t" + std::to_string(k++) + ". <" + block_status(block.is_block_occupied) + "> <" + std::to_string(block.block_size) + ">\n";
        blocks += small_info;

    }
    return blocks;
}

std::string allocator_boundary_tags::block_status(bool state) const noexcept {
    if (state) {
        return "occupied";
    }
    return "available";
}

inline logger *allocator_boundary_tags::get_logger() const
{
    //return *reinterpret_cast<logger **>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(allocator *));
    return *reinterpret_cast<logger **>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*));
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}


// Support functions
size_t allocator_boundary_tags::get_ancillary_space_size() const noexcept
{
    return sizeof(size_t) + sizeof(allocator *) + sizeof(logger *) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(void*) + sizeof(std::mutex);
}

allocator_with_fit_mode::fit_mode allocator_boundary_tags::get_fit_mode() const noexcept
{
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode *>(reinterpret_cast<uint8_t *>(_trusted_memory) + sizeof(allocator *) + sizeof(logger *));
}


void * allocator_boundary_tags::get_first_busy_block() const noexcept {
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) +
                                     sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex));
}

void *allocator_boundary_tags::get_first_aviable_block() const noexcept
{
    return reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) +
           sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*);
}

allocator::block_size_t allocator_boundary_tags::get_size_block(
    void *block_address) const noexcept
{
    return *reinterpret_cast<size_t *>(reinterpret_cast<uint8_t*>(block_address) + sizeof(allocator*));
}

void * allocator_boundary_tags::get_end_ptr() const noexcept {
    return reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) +
           sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(void*) + get_size_memory();
}

allocator * allocator_boundary_tags::get_allocator_from_block(void* block) const noexcept {
    return *reinterpret_cast<allocator**>(block);
}

void * allocator_boundary_tags::get_prev_block(void* block) const noexcept {
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(allocator*) + sizeof(size_t));
}

void * allocator_boundary_tags::get_next_block(void* block) const noexcept {
    return *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(block) + sizeof(allocator*) + sizeof(size_t) + sizeof(void*));
}

void allocator_boundary_tags::merge_blocks(void* prev, void* next) noexcept {
    if (prev != nullptr) {
        void ** new_next = reinterpret_cast<void**>(reinterpret_cast<uint8_t *>(prev) + sizeof(size_t) + sizeof(allocator*) + sizeof(void*));
        *new_next = next;
    } else {
        set_first_filled_block(next);
        *reinterpret_cast<void**>(reinterpret_cast<uint8_t *>(next) + sizeof(size_t) + sizeof(allocator*)) = nullptr;
        *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(next) + sizeof(size_t) + sizeof(allocator*) + sizeof(void*)) = get_end_ptr();
        return;
    }
    if (next != nullptr ) {
        *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(next) + sizeof(size_t) + sizeof(allocator*)) = prev;
    } else
    {
        *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(prev) + sizeof(size_t) + sizeof(allocator*) + sizeof(void*)) = nullptr;
    }
}

void allocator_boundary_tags::set_first_filled_block(void* block) const noexcept {
    *reinterpret_cast<void**>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) + sizeof(logger*) +
                              sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex)) = block;
}

std::mutex & allocator_boundary_tags::get_mutex() const noexcept {
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<unsigned char *>(_trusted_memory) + sizeof(size_t) + sizeof(allocator*) +
                                          sizeof(logger*) + sizeof(allocator_with_fit_mode::fit_mode));
}

size_t allocator_boundary_tags::get_available_memory() const noexcept {
    void * cur = get_first_busy_block();
    if (cur == nullptr) {
        return get_size_memory();
    }
    void* next = nullptr;

    size_t memory_occupied = 0;


    while (cur != nullptr && cur != get_end_ptr()) {
        memory_occupied += get_size_block(cur);
        cur = get_next_block(cur);
    }

    size_t res = get_size_memory() - memory_occupied;
    return res;
}

size_t allocator_boundary_tags::get_size_memory() const noexcept {
    return *reinterpret_cast<size_t*>(reinterpret_cast<uint8_t*>(_trusted_memory) + sizeof(allocator*)
                                      + sizeof (logger*) + sizeof (allocator_with_fit_mode::fit_mode));
}

std::string allocator_boundary_tags::get_block_info(void * block) const noexcept {
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

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const noexcept {
    std::vector<allocator_test_utils::block_info> info;
    void* cur = get_first_busy_block();


    if (cur == nullptr) {
        allocator_test_utils::block_info block;
        block.block_size = get_size_memory();
        block.is_block_occupied = false;
        info.push_back(block);
        return info;
    }

    void * prev = nullptr;

    void *end_ptr = get_end_ptr();


    while (cur != nullptr && cur != end_ptr) {
        if ((prev == nullptr && cur != get_first_aviable_block()) || (prev != nullptr && (reinterpret_cast<unsigned char *>(prev) + get_size_block(prev) != cur))) {
            size_t size;
            if (prev == nullptr) {
                size = reinterpret_cast<unsigned char *>(cur) - (reinterpret_cast<unsigned char*>(get_first_aviable_block()));
            } else {
                size = reinterpret_cast<unsigned char *>(cur) - (reinterpret_cast<unsigned char*>(prev)  + get_size_block(prev));
            }
            allocator_test_utils::block_info avail_block;
            avail_block.block_size = size;
            avail_block.is_block_occupied = false;
            info.push_back(avail_block);
        }
        allocator_test_utils::block_info occup_block;
        occup_block.block_size = get_size_block(cur);
        occup_block.is_block_occupied = true;
        info.push_back(occup_block);
        prev = cur;
        cur = get_next_block(cur);
    }
    if (prev != get_end_ptr() && prev != nullptr) {
        size_t size = reinterpret_cast<unsigned char *>(get_end_ptr()) - (reinterpret_cast<unsigned char*>(prev)  + get_size_block(prev));
        allocator_test_utils::block_info avail_block;
        avail_block.block_size = size;
        avail_block.is_block_occupied = false;
        if (size != 0) {
            info.push_back(avail_block);
        }
    }
    return info;
}