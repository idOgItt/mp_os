#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <mutex>

class allocator_boundary_tags final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    
    void *_trusted_memory;

public:
    
    ~allocator_boundary_tags() override;
    
    allocator_boundary_tags(
        allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags const &other) = delete;
    
    allocator_boundary_tags(
        allocator_boundary_tags &&other) noexcept = delete;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags &&other) noexcept = delete;

public:
    
    explicit allocator_boundary_tags(
        size_t space_size,
        allocator *parent_allocator = nullptr,
        logger *logger = nullptr,
        allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *allocate(
        size_t value_size,
        size_t values_count) override;
    
    void deallocate(
        void *at) override;

public:
    
    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

private:
    
    inline allocator *get_allocator() const override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const noexcept override;

private:
    
    inline logger *get_logger() const override;

private:
    
    inline std::string get_typename() const noexcept override;

private:

    size_t get_ancillary_space_size() const noexcept;

    allocator_with_fit_mode::fit_mode get_fit_mode() const noexcept;

    void *get_first_aviable_block() const noexcept;

private:

    size_t get_available_memory() const noexcept;

    void clear_block(void * block) const noexcept;

    std::string get_blocks_info(std::vector<allocator_test_utils::block_info> info) const noexcept;

    std::string block_status(bool state) const noexcept;

    void * get_first_busy_block() const noexcept;

    allocator::block_size_t get_size_block(
            void *block_address) const noexcept;

    void * get_end_ptr() const noexcept;

    allocator * get_allocator_from_block(void* block) const noexcept;

    void * get_prev_block(void* block) const noexcept;

    void * get_next_block(void* block) const noexcept;

    void merge_blocks(void* prev, void* next) noexcept;

    void set_first_filled_block(void* block) const noexcept;

    size_t get_size_memory() const noexcept;

    std::mutex & get_mutex() const noexcept;

    std::string get_block_info(void * block) const noexcept;
    
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H