#include <not_implemented.h>
#include <mutex>

#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    deallocate_with_guard(_trusted_memory);
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    if (_trusted_memory != nullptr) deallocate_with_guard(_trusted_memory);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}

allocator_sorted_list::allocator_sorted_list(
    size_t space_size,
    allocator *parent_allocator,
    logger *logger,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{

    if (space_size < get_available_block_meta_size()) { // 
        throw std::logic_error("Unable to create allocator on such small size");
    }

    try {
        if (parent_allocator != nullptr) _trusted_memory = parent_allocator->allocate(get_main_meta_size() + get_available_block_meta_size(), 1);
        _trusted_memory = ::operator new(get_main_meta_size() + get_available_block_meta_size());
    }
    catch (std::bad_alloc &e) {
        throw e;
    }

    unsigned char* memory = reinterpret_cast<unsigned char*>(_trusted_memory);

    *reinterpret_cast<allocator**>(memory) = parent_allocator;
    memory += sizeof(allocator*);

    *reinterpret_cast<class logger**>(memory) = logger;
    memory += sizeof(class logger*);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(memory) = allocate_fit_mode;
    memory += sizeof(allocator_with_fit_mode::fit_mode);

    allocator::construct(reinterpret_cast<std::mutex *>(memory));
    memory += sizeof(std::mutex);

    *reinterpret_cast<block_size_t*>(memory) = space_size;
    memory += sizeof(block_size_t);

    *reinterpret_cast<block_pointer_t*>(memory) = reinterpret_cast<block_pointer_t>(memory + sizeof(block_pointer_t));
    memory += sizeof(block_pointer_t);    

    *reinterpret_cast<block_size_t*>(memory) = space_size;
    memory += sizeof(block_size_t);

    *reinterpret_cast<block_pointer_t*>(memory) = nullptr; 
}

[[nodiscard]] void *allocator_sorted_list::allocate(
    size_t value_size,
    size_t values_count)
{
    size_t result_size = value_size * values_count;

    allocator_with_fit_mode::fit_mode mode = get_fit_mode();

    block_pointer_t prev_res = nullptr;
    block_pointer_t res = nullptr;

    switch (mode)
    {
    case allocator_with_fit_mode::fit_mode::first_fit:
        res = find_first_fit(result_size, &prev_res);
        break;
    case allocator_with_fit_mode::fit_mode::the_best_fit:
        res = find_best_fit(result_size, &prev_res);
        break;
    case allocator_with_fit_mode::fit_mode::the_worst_fit:
        res = find_worst_fit(result_size, &prev_res);
        break;
    default:
        throw std::logic_error("this is impossible");
        break;
    }

    if (res == nullptr) {
        throw std::bad_alloc();
    }

    if (res == prev_res) {

        if (get_available_block_size(res) + get_available_block_meta_size() - result_size - get_occupied_block_meta_size() < get_available_block_meta_size()) {
            result_size = get_available_block_size(res);
        }  

        if (get_available_block_size(res) + get_available_block_meta_size() == result_size + get_occupied_block_meta_size()) {
            get_first_available_block() = nullptr;
            init_occupied_block(res, result_size, this);
        }
        else {
            block_pointer_t new_av = reinterpret_cast<unsigned char*> (res) + get_available_block_meta_size() + result_size;
            init_available_block(new_av, get_available_block_size(res) - result_size - get_available_block_meta_size(), get_available_block_next_ptr(res));

            get_first_available_block() = new_av;   
        }
    }

    if (get_available_block_size(res) + get_available_block_meta_size() - result_size - get_occupied_block_meta_size() < get_available_block_meta_size()) {
        result_size = get_available_block_size(res);
    }  

    if (get_available_block_size(res) + get_available_block_meta_size() == result_size + get_occupied_block_meta_size()) {
        get_available_block_next_ptr(prev_res) = get_available_block_next_ptr(res);
        init_occupied_block(res, result_size, this);
    }
    else {
        block_pointer_t new_av = reinterpret_cast<block_pointer_t>(reinterpret_cast<unsigned char*>(res) + get_available_block_meta_size() + result_size);
        init_available_block(new_av, get_available_block_size(res) - get_available_block_meta_size(), get_available_block_next_ptr(res));

        get_available_block_next_ptr(prev_res) = new_av;
        init_occupied_block(res, result_size, this);
    }

    return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(res) + get_occupied_block_meta_size());

}

allocator::block_pointer_t allocator_sorted_list::find_first_fit(block_size_t requested_size, block_pointer_t* previous_block) {
    block_pointer_t res = get_first_available_block();
    block_pointer_t prev_res = res;

    while (res != nullptr && (get_available_block_size(res) + get_available_block_meta_size()) < requested_size + get_occupied_block_meta_size()) {
        prev_res = res;
        res = get_available_block_next_ptr(res);
    }

    *previous_block = prev_res;
    return res;
}

allocator::block_pointer_t allocator_sorted_list::find_best_fit(block_size_t requested_size, block_pointer_t* previous_block) {
    block_pointer_t res = get_first_available_block();
    block_pointer_t prev_res = res;

    if (res == nullptr) {
        return res;
    }

    block_size_t min_diff = get_available_block_size(res) + get_available_block_meta_size() - requested_size - get_occupied_block_meta_size();
    block_size_t tmp_diff = get_available_block_size(res) + get_available_block_meta_size() - requested_size - get_occupied_block_meta_size();


    block_pointer_t tmp_el = res;
    block_pointer_t prev_tmp_el = tmp_el;

    while(tmp_el != nullptr) {
        tmp_diff = get_available_block_size(tmp_el) + get_occupied_block_meta_size() - requested_size - get_occupied_block_meta_size(); 
        if (tmp_diff >= 0 && tmp_diff < min_diff) {
            min_diff = tmp_diff;
            res = tmp_el;
            prev_res = prev_tmp_el;
        }
        prev_tmp_el = tmp_el;
        tmp_el = get_available_block_next_ptr(tmp_el);
    }

    *previous_block = prev_res;
    return res;
}

allocator::block_pointer_t allocator_sorted_list::find_worst_fit(block_size_t requested_size, block_pointer_t* previous_block) {
    block_pointer_t res = get_first_available_block();
    block_pointer_t prev_res = res;

    if (res == nullptr) {
        return res;
    }

    block_size_t min_diff = get_available_block_size(res) + get_available_block_meta_size() - requested_size - get_occupied_block_meta_size();
    block_size_t tmp_diff = get_available_block_size(res) + get_available_block_meta_size() - requested_size - get_occupied_block_meta_size();


    block_pointer_t tmp_el = res;
    block_pointer_t prev_tmp_el = tmp_el;

    while(tmp_el != nullptr) {
        tmp_diff = get_available_block_size(tmp_el) + get_occupied_block_meta_size() - requested_size - get_occupied_block_meta_size(); 
        if (tmp_diff >= 0 && tmp_diff > min_diff) {
            min_diff = tmp_diff;
            res = tmp_el;
            prev_res = prev_tmp_el;
        }
        prev_tmp_el = tmp_el;
        tmp_el = get_available_block_next_ptr(tmp_el);
    }

    *previous_block = prev_res;
    return res;
}

void allocator_sorted_list::deallocate(
    void *at)
{

}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*) + sizeof(logger*)) = mode; 
}

inline allocator_with_fit_mode::fit_mode allocator_sorted_list::get_fit_mode() {
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator *) + sizeof(class logger*));
}

inline allocator *allocator_sorted_list::get_allocator() const
{
    return reinterpret_cast<allocator*>(_trusted_memory);
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    throw not_implemented("std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept", "your code should be here...");
}

inline logger *allocator_sorted_list::get_logger() const
{
    return reinterpret_cast<logger*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator*));
}

inline std::string allocator_sorted_list::get_typename() const noexcept
{
    return "allocator sorted list";
}

size_t allocator_sorted_list::get_main_meta_size() {
    return sizeof(allocator *) + sizeof(class logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(size_t) + sizeof(block_pointer_t);
    // указатель на аллокатор + указатель на логгер + фит мод + мьютекс + размер блока + указатель на первый свободный блок памяти
}

size_t allocator_sorted_list::get_available_block_meta_size() {
    return sizeof(block_size_t) + sizeof(block_pointer_t);
}

size_t allocator_sorted_list::get_occupied_block_meta_size() {
    return sizeof(allocator*) + sizeof(allocator::block_size_t);
}

allocator::block_pointer_t &allocator_sorted_list::get_first_available_block() {
    return *reinterpret_cast<block_pointer_t*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator *) + sizeof(class logger*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(std::mutex) + sizeof(size_t));
}

std::mutex &allocator_sorted_list::get_mutex() {
    return *reinterpret_cast<std::mutex*>(reinterpret_cast<unsigned char*>(_trusted_memory) + sizeof(allocator *) + sizeof(class logger*) + sizeof(allocator_with_fit_mode::fit_mode));
}

allocator::block_size_t &allocator_sorted_list::get_available_block_size(block_pointer_t b) {
    return *reinterpret_cast<block_size_t*>(b);
}

allocator::block_pointer_t &allocator_sorted_list::get_available_block_next_ptr(block_pointer_t b) {
    return *reinterpret_cast<block_pointer_t*>(reinterpret_cast<unsigned char*>(b) + sizeof(block_size_t));
}

allocator::block_size_t &allocator_sorted_list::get_occupied_block_size(block_pointer_t b) {
    return *reinterpret_cast<block_size_t*>(reinterpret_cast<unsigned char*>(b) + sizeof(allocator*));
}

allocator* &allocator_sorted_list::get_occupied_block_allocator(block_pointer_t b) {
    return *reinterpret_cast<allocator**>(b);
}

allocator::block_pointer_t allocator_sorted_list::init_available_block(block_pointer_t block, block_size_t size, block_pointer_t next) {
    get_available_block_size(block) = size;
    get_available_block_next_ptr(block) = next;

    return block;
}

allocator::block_pointer_t allocator_sorted_list::init_occupied_block(block_pointer_t block, block_size_t size, allocator* alctr) {
    get_occupied_block_size(block) = size;
    get_occupied_block_allocator(block) = alctr;

    return block;
}