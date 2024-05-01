#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H

#include <allocator_guardant.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <logger_guardant.h>
#include <typename_holder.h>
#include <mutex>


class allocator_buddies_system final:
    private allocator_guardant,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:
    
    void *_trusted_memory;

    struct byte_field
    {
        bool is_occup : 1;
        uint8_t  size : 7;
    };

public:
    
    ~allocator_buddies_system() override;
    
    allocator_buddies_system(
        allocator_buddies_system const &other) = delete;
    
    allocator_buddies_system &operator=(
        allocator_buddies_system const &other) = delete;
    
    allocator_buddies_system(
        allocator_buddies_system &&other) noexcept = delete;
    
    allocator_buddies_system &operator=(
        allocator_buddies_system &&other) noexcept = delete;

public:
    
    explicit allocator_buddies_system(
        size_t space_size_power_of_two,
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

    size_t get_meta_block() const noexcept;

    size_t get_ancillary_space_size() const noexcept;

    uint8_t two_degree(size_t size) const noexcept;

    std::string block_status(bool state) const noexcept;

    std::string get_block_info(void * block) const noexcept;

    std::string get_blocks_info(std::vector<allocator_test_utils::block_info> info) const noexcept;

    allocator_with_fit_mode::fit_mode& get_fit_mode() const noexcept;

    size_t get_block_size(void* current_block) const noexcept;

    size_t get_size_memory() const noexcept;

    inline allocator *get_allocator_block(void* current_block) const noexcept;

    void* get_block_twin(void* current_block) noexcept;

    bool is_busy(void* current_block) const noexcept;

    size_t size_in_power(size_t power_of_2) const noexcept;

    size_t get_available_memory()  const noexcept;

    void* get_first_suitable(size_t requested_size)  const noexcept;

    void* get_worst_suitable(size_t requested_size) const noexcept;

    void* get_best_suitable(size_t requested_size)  const noexcept;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BUDDIES_SYSTEM_H
