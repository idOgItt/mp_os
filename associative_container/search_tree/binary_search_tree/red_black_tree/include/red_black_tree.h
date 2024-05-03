#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_RED_BLACK_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_RED_BLACK_TREE_H

#include <binary_search_tree.h>

template<
    typename tkey,
    typename tvalue>
class red_black_tree final:
    public binary_search_tree<tkey, tvalue>
{

public:
    
    enum class node_color
    {
        RED,
        BLACK
    };

private:
    
    struct node final:
        binary_search_tree<tkey, tvalue>::node
    {
    public:

        node_color _color;


public:

    explicit  node(tkey const& key, tvalue const& value);

    explicit node(tkey const& key, tvalue&& value);

    ~node() noexcept = default;


public:

    void change_color(node_color new_color)
    {
        _color = new_color;
    }

    };

public:
    
    struct iterator_data final:
        public binary_search_tree<tkey, tvalue>::iterator_data
    {
    
    public:
        
        node_color color;
    
    public:
        
        explicit iterator_data(
            unsigned int depth = 0,
            tkey const &key = tkey(),
            tvalue const &value = tvalue(),
            node_color color = node_color::RED);

        iterator_data(
                iterator_data const &other);

        iterator_data(
                iterator_data &&other) noexcept;

        iterator_data &operator=(
                iterator_data const &other);

        iterator_data &operator=(
                iterator_data &&other) noexcept;

        ~iterator_data() noexcept final = default;

    public:

        void update_iterator_data(std::stack<typename binary_search_tree<tkey, tvalue>::node**>& path) override
        {
            if (!path.empty()){
                binary_search_tree<tkey, tvalue>::iterator_data::key = (*(path.top()))->key;
                binary_search_tree<tkey, tvalue>::iterator_data::value = (*(path.top()))->value;
                binary_search_tree<tkey, tvalue>::iterator_data::depth = path.size() - 1;
                color = static_cast<node *>(*(path.top()))->_color;
            }
        }

        iterator_data* clone_iterator_data() const override
        {
            return new iterator_data(*this);
        }
        
    };

public:
    
    explicit red_black_tree(
        std::function<int(tkey const &, tkey const &)> keys_comparer,
        allocator *allocator = nullptr,
        logger *logger = nullptr,
        typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy = binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy::throw_an_exception,
        typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy = binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy::throw_an_exception);

public:
    
    ~red_black_tree() noexcept final;
    
    red_black_tree(
        red_black_tree<tkey, tvalue> const &other);
    
    red_black_tree<tkey, tvalue> &operator=(
        red_black_tree<tkey, tvalue> const &other);
    
    red_black_tree(
        red_black_tree<tkey, tvalue> &&other) noexcept;
    
    red_black_tree<tkey, tvalue> &operator=(
        red_black_tree<tkey, tvalue> &&other) noexcept;

private:

    void insert_inside(const tkey& key, const tvalue& value, std::stack<typename binary_search_tree<tkey, tvalue>::node**>& stack) override;

    void insert_inside(const tkey& key, tvalue&& value, std::stack<typename binary_search_tree<tkey, tvalue>::node**>& stack) override;

    tvalue dispose_inside(std::stack<typename binary_search_tree<tkey, tvalue>::node**>& stack) override;

    void copy_subtree(typename binary_search_tree<tkey, tvalue>::node** target, typename binary_search_tree<tkey, tvalue>::node* src);

    template<typename rb_tvalue>
    void red_black_insert(const tkey& key, rb_tvalue&& value, std::stack<typename binary_search_tree<tkey, tvalue>::node**>& stack);

    void print_rb(binary_search_tree<tkey, tvalue>::node* root, int level) const noexcept;

    void inject_additional_data(
            typename binary_search_tree<tkey,tvalue>::node *destination,
            typename binary_search_tree<tkey,tvalue>::node const *source) const override;

    typename binary_search_tree<tkey,tvalue>::iterator_data *create_iterator_data() const override;

    inline void call_node_constructor(
            typename binary_search_tree<tkey,tvalue>::node *raw_space,
            tkey const &key,
            tvalue const &value) override;

//    inline void update_node_data(
//            typename binary_search_tree<tkey,tvalue>::node *node) const noexcept override;

    inline void call_node_constructor(
            typename binary_search_tree<tkey,tvalue>::node *raw_space,
            tkey const &key,
            tvalue &&value) override;


    inline size_t get_node_size() const noexcept override;

    static inline node_color get_color(
            typename red_black_tree<tkey,tvalue>::node* node) noexcept;

    static inline void set_color(
            typename binary_search_tree<tkey,tvalue>::node* node,
            node_color color) noexcept;
    
};

// node
template<typename tkey, typename tvalue>
red_black_tree<tkey, tvalue>::node::node(
        const tkey &key, const tvalue &value) :
        binary_search_tree<tkey, tvalue>::node(key, value) ,
        _color(node_color::RED)
{
}

template<typename tkey, typename tvalue>
red_black_tree<tkey, tvalue>::node::node(
        const tkey &key, tvalue&& value) :
        binary_search_tree<tkey, tvalue>::node(key, std::move(value)),
        _color(node_color::RED)
{
}
//node

// region iterator_data

template<
        typename tkey,
        typename tvalue>
red_black_tree<tkey, tvalue>::iterator_data::iterator_data(
        iterator_data const &other):
        binary_search_tree<tkey, tvalue>::iterator_data(other),
        color(other.color)
{ }

template<
        typename tkey,
        typename tvalue>
red_black_tree<tkey, tvalue>::iterator_data::iterator_data(
        iterator_data &&other) noexcept:
        binary_search_tree<tkey, tvalue>::iterator_data(std::move(other)),
        color(std::move(other.color))
{ }

template<
        typename tkey,
        typename tvalue>
typename red_black_tree<tkey, tvalue>::iterator_data &red_black_tree<tkey, tvalue>::iterator_data::operator=(
        iterator_data const &other)
{
    if (this != &other)
    {
        binary_search_tree<tkey, tvalue>::iterator_data::operator=(other);
        color = other.color;
    }

    return *this;
}

template<
        typename tkey,
        typename tvalue>
typename red_black_tree<tkey, tvalue>::iterator_data &red_black_tree<tkey, tvalue>::iterator_data::operator=(
        iterator_data &&other) noexcept
{
    if (this != &other)
    {
        binary_search_tree<tkey, tvalue>::iterator_data::operator=(std::move(other));
        color = std::move(other.color);
    }

    return *this;
}


template<
    typename tkey,
    typename tvalue>
red_black_tree<tkey, tvalue>::iterator_data::iterator_data(
    unsigned int depth,
    tkey const &key,
    tvalue const &value,
    typename red_black_tree<tkey, tvalue>::node_color color):
    binary_search_tree<tkey, tvalue>::iterator_data(depth, key, value) , color(color)
{}

// end region iterator_data

// red_black_tree
template<
        typename tkey,
        typename tvalue>
        red_black_tree<tkey, tvalue>::red_black_tree(
        std::function<int(tkey const &, tkey const &)> keys_comparer,
        allocator *allocator,
        logger *logger,
        typename binary_search_tree<tkey, tvalue>::insertion_of_existent_key_attempt_strategy insertion_strategy,
        typename binary_search_tree<tkey, tvalue>::disposal_of_nonexistent_key_attempt_strategy disposal_strategy):
                binary_search_tree<tkey, tvalue>(keys_comparer, allocator, logger, insertion_strategy, disposal_strategy)
{}

template<
    typename tkey,
    typename tvalue>
red_black_tree<tkey, tvalue>::~red_black_tree() noexcept = default;

template<
    typename tkey,
    typename tvalue>
red_black_tree<tkey, tvalue>::red_black_tree(
    red_black_tree<tkey, tvalue> const &other):
        binary_search_tree<tkey, tvalue>(other)
{}

template<
    typename tkey,
    typename tvalue>
red_black_tree<tkey, tvalue> &red_black_tree<tkey, tvalue>::operator=(
    red_black_tree<tkey, tvalue> const &other)
{
    if (this != other)
    {
        binary_search_tree<tkey, tvalue>::operator=(other);
    }
    return *this;
}

template<
    typename tkey,
    typename tvalue>
red_black_tree<tkey, tvalue>::red_black_tree(
    red_black_tree<tkey, tvalue> &&other) noexcept:
        binary_search_tree<tkey, tvalue>(std::move(other))
{
}

template<
    typename tkey,
    typename tvalue>
red_black_tree<tkey, tvalue> &red_black_tree<tkey, tvalue>::operator=(
    red_black_tree<tkey, tvalue> &&other) noexcept
{
    if (this != &other)
    {
        binary_search_tree<tkey, tvalue>::operator=(std::move(other));
    }

    return *this;
}

// region dop-functions

template<
        typename tkey,
        typename tvalue>
void red_black_tree<tkey, tvalue>::inject_additional_data(
        typename binary_search_tree<tkey,tvalue>::node *destination,
        typename binary_search_tree<tkey,tvalue>::node const *source) const
{
    auto *rb_destination = dynamic_cast<red_black_tree<tkey, tvalue>::node*>(destination);
    auto *rb_source = dynamic_cast<red_black_tree<tkey, tvalue>::node const*>(source);

    rb_destination->_color = rb_source->_color;
}

template<
        typename tkey,
        typename tvalue>
typename binary_search_tree<tkey, tvalue>::iterator_data *red_black_tree<tkey, tvalue>::create_iterator_data() const
{
    return new iterator_data();
}

template<
        typename tkey,
        typename tvalue>
inline size_t red_black_tree<tkey, tvalue>::get_node_size() const noexcept
{
    return sizeof(typename red_black_tree<tkey, tvalue>::node);
}


template<typename tkey, typename tvalue>
void red_black_tree<tkey, tvalue>::insert_inside(const tkey &key, const tvalue &value,std::stack<typename binary_search_tree<tkey, tvalue>::node **> &stack)
{
    red_black_insert(key, value, stack);
}

template<typename tkey, typename tvalue>
void red_black_tree<tkey, tvalue>::insert_inside(const tkey &key, tvalue&& value,std::stack<typename binary_search_tree<tkey, tvalue>::node **> &stack)
{
    red_black_insert(key, std::move(value), stack);
}

template<typename tkey, typename tvalue>
template<typename rb_tvalue>
void red_black_tree<tkey, tvalue>::red_black_insert(const tkey &key, rb_tvalue &&value, std::stack<typename binary_search_tree<tkey, tvalue>::node**> & stack)
{
    (*stack.top()) = static_cast<typename binary_search_tree<tkey, tvalue>::node *>(static_cast<node *>(allocator_guardant::allocate_with_guard(
            sizeof(node), 1)));

    try {
        allocator::construct(*stack.top(), key, std::forward<rb_tvalue>(value));
    }
    catch (...) {
        allocator_guardant::deallocate_with_guard(*stack.top());
        throw;
    }

    node *current_node = static_cast<node *>(*stack.top());

    stack.pop();

    if (stack.empty())
    {
        current_node->change_color(node_color::BLACK);
        return;
    }
    current_node->change_color(node_color::RED);

    while (!stack.empty()) {
        node *parent = static_cast<node *>(*stack.top());
        stack.pop();

        if (get_color(parent) == node_color::BLACK) {
            break;
        }

        if (stack.empty())
        {
            return;
        }

        node *grandparent = static_cast<node *>(*stack.top());
        node *uncle = (parent == grandparent->left_subtree) ? static_cast<node *>(grandparent->right_subtree)
                                                            : static_cast<node *>(grandparent->left_subtree);

        // Родитель и дядя узла оба красные
        if (uncle != nullptr && uncle->_color == node_color::RED) {
            // Case 2: Parent and uncle are red, recoloring
            parent->change_color(node_color::BLACK);
            uncle->change_color(node_color::BLACK);
            grandparent->change_color(node_color::RED);
            current_node = grandparent;
        } else {
            // Родитель красный, а дядя черный или отсутствует
            if (current_node == parent->right_subtree && parent == grandparent->left_subtree) {
                binary_search_tree<tkey, tvalue>::small_right_rotation((*stack.top())->left_subtree);
                std::swap(parent-> _color, grandparent->_color);
                std::swap(parent, current_node);
            } else if (current_node == parent->left_subtree && parent == grandparent->right_subtree) {
                binary_search_tree<tkey, tvalue>::small_left_rotation((*stack.top())->right_subtree);
                std::swap(parent-> _color, grandparent->_color);
                std::swap(parent, current_node);
            }
            // Родитель и узел находятся в одной линии
            parent->change_color(node_color::BLACK);
            grandparent->change_color(node_color::RED);
            if (current_node == parent->left_subtree)
                binary_search_tree<tkey, tvalue>::small_left_rotation(*stack.top());
            else
                binary_search_tree<tkey, tvalue>::small_right_rotation(*stack.top());
            break;
        }
    }
    print_rb(binary_search_tree<tkey, tvalue>::_root, 0);
    std::cout << "\n-------------------------------------------------\n" << std::endl;
}


template<typename tkey, typename tvalue>
void red_black_tree<tkey, tvalue>::copy_subtree(typename binary_search_tree<tkey, tvalue>::node** target, typename binary_search_tree<tkey, tvalue>::node* src)
{
    if(src == nullptr)
    {
        *target = nullptr;
        return;
    }

    *target = static_cast<typename binary_search_tree<tkey, tvalue>::node*>(reinterpret_cast<node*>(allocator_guardant::allocate_with_guard(sizeof(node))));
    try
    {
        allocator::construct(static_cast<node*>(*target), *static_cast<node*>(src));
    } catch(...)
    {
        allocator_guardant::deallocate_with_guard(static_cast<node*>(*target));
        *target = nullptr;
        throw;
    }

    (*target)->right_subtree = nullptr;
    (*target)->left_subtree = nullptr;

    copy_subtree(&((*target)->left_subtree), src->left_subtree);
    copy_subtree(&((*target)->right_subtree), src->right_subtree);
}

template<typename tkey, typename tvalue>
tvalue red_black_tree<tkey, tvalue>::dispose_inside(std::stack<typename binary_search_tree<tkey, tvalue>::node**>& stack)
{
    //TODO
}

template<
        typename tkey,
        typename tvalue>
inline void red_black_tree<tkey, tvalue>::call_node_constructor(
        typename binary_search_tree<tkey,tvalue>::node *raw_space,
        tkey const &key,
        tvalue const &value)
{
    allocator::construct(reinterpret_cast<red_black_tree<tkey, tvalue>::node*>(raw_space), key, value);
}

template<
        typename tkey,
        typename tvalue>
inline void red_black_tree<tkey, tvalue>::call_node_constructor(
        typename binary_search_tree<tkey,tvalue>::node *raw_space,
        tkey const &key,
        tvalue &&value)
{
    allocator::construct(reinterpret_cast<red_black_tree<tkey, tvalue>::node*>(raw_space), key, std::move(value));
}

template<
        typename tkey,
        typename tvalue>
void red_black_tree<tkey, tvalue>::set_color(
        typename binary_search_tree<tkey,tvalue>::node* node,
        node_color color) noexcept
{
    dynamic_cast<red_black_tree<tkey,tvalue>::node*>(node)->color = color;
}

template<
        typename tkey,
        typename tvalue>
typename red_black_tree<tkey, tvalue>::node_color red_black_tree<tkey, tvalue>::get_color(
        typename red_black_tree<tkey,tvalue>::node* node) noexcept
{
    if (node != nullptr)
        return node->_color;
    else
        return node_color::BLACK;
}

template<typename tkey, typename tvalue>
void red_black_tree<tkey, tvalue>::print_rb(typename binary_search_tree<tkey, tvalue>::node* root, int level) const noexcept
{
    if(root == nullptr)
    {
        return;
    }

    print_rb(root->right_subtree, level + 1);
    std::string color = static_cast<node*>(root)->_color == node_color::BLACK ? "BLACK" : "RED";
    for(int i = 0, end = level; i != end; ++i)
    {
        std::cout << "    ";
    }
    std::cout << root->key << " " << root->value << " " << color << std::endl;
    print_rb(root->left_subtree, level + 1);
}

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_RED_BLACK_TREE_H
