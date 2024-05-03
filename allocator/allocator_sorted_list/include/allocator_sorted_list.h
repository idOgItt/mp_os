#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_SORTED_LIST_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_SORTED_LIST_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <mutex>

class allocator_sorted_list final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    
    void *_trusted_memory;

public:
    
    ~allocator_sorted_list() override;
    
    allocator_sorted_list(
        allocator_sorted_list const &other)  = delete;
    
    allocator_sorted_list &operator=(
        allocator_sorted_list const &other) = delete;
    
    allocator_sorted_list(
        allocator_sorted_list &&other) noexcept;
    
    allocator_sorted_list &operator=(
        allocator_sorted_list &&other) noexcept;

public:
    
    explicit allocator_sorted_list(
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

    std::mutex& get_mutex() noexcept;

    allocator_with_fit_mode::fit_mode& get_fit_mode() const noexcept;

    size_t get_size_memory() const noexcept;

    size_t get_size_block(void* current_block) const noexcept;

    inline void** get_ptr_to_previous(void* ptr) const noexcept;

    void* get_ptr_from_block(void* current) const noexcept;

    void* get_first_block() const noexcept;

    std::string get_blocks_info(std::vector<allocator_test_utils::block_info> info) const noexcept;

    std::string block_status(bool state) const noexcept;

    std::string get_block_info(void * block) const noexcept;

    size_t get_ancillary_space_size() const noexcept;

    size_t get_size_meta_block() const noexcept;

    size_t get_available_memory()  noexcept;

    void* get_best_suitable(size_t requested_size)  noexcept;

    void* get_first_suitable(size_t requested_size)  noexcept;

    void* get_worst_suitable(size_t requested_size)  noexcept;

    void* get_previous_for_loaded(void* loaded_ptr) const noexcept;

    void* get_previous_for_free(void* free_ptr) const noexcept;

    void set_first_block(void* block) const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_SORTED_LIST_H