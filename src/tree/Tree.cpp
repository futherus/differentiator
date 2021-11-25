#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "Tree.h"
#include "../dumpsystem/dumpsystem.h"

#define ASSERT(CONDITION, ERROR)                \
    do                                          \
    {                                           \
        if(!(CONDITION))                        \
        {                                       \
            tree_dump(tree, #ERROR, (ERROR));   \
            return (ERROR);                     \
        }                                       \
    } while(0)                                  \

#define PASS(CONDITION, ERROR)                  \
    do                                          \
    {                                           \
        if(!(CONDITION))                        \
            return (ERROR);                     \
    } while(0)                                  \

static tree_err ptr_arr_resize_(Tree* tree)
{
    assert(tree);
    
    ptrdiff_t new_cap = tree->ptr_arr_cap;
    if(new_cap == 0)
        new_cap = TREE_PTR_ARR_MIN_CAP;

    Node** temp = (Node**) realloc(tree->ptr_arr, new_cap * sizeof(Node*));
    ASSERT(temp, TREE_BAD_ALLOC);
    
    memset(temp + tree->ptr_arr_cap, 0, new_cap - tree->ptr_arr_cap);

    tree->ptr_arr     = temp;
    tree->ptr_arr_cap = new_cap;
    
    return TREE_NOERR;
}

static tree_err add_chunk_(Tree* tree)
{
    assert(tree);

    if(tree->cap / TREE_CHUNK_SIZE == tree->ptr_arr_cap)
        PASS(!ptr_arr_resize_(tree), TREE_BAD_ALLOC);
    
    Node* temp = (Node*) calloc(TREE_CHUNK_SIZE, sizeof(Node));
    ASSERT(temp, TREE_BAD_ALLOC);

    tree->ptr_arr[tree->cap / TREE_CHUNK_SIZE] = temp;
    tree->cap += TREE_CHUNK_SIZE;
    
    return TREE_NOERR;
}

tree_err tree_dstr(Tree* tree)
{
    ASSERT(tree, TREE_NULLPTR);
    ASSERT(tree->ptr_arr, TREE_NOTINIT);

    for(ptrdiff_t iter = 0; iter < tree->cap / TREE_CHUNK_SIZE; iter++)
        free(tree->ptr_arr[iter]);
    
    return TREE_NOERR;
}

tree_err tree_add(Tree* tree, Node** base_ptr, const Lexem* data)
{
    ASSERT(tree && base_ptr && data, TREE_NULLPTR);

    if(tree->cap == tree->size)
        PASS(!add_chunk_(tree), TREE_BAD_ALLOC);

    Node* new_node = &tree->ptr_arr[tree->size / TREE_CHUNK_SIZE][tree->size % TREE_CHUNK_SIZE];
    *new_node  = {*data, nullptr, nullptr};
    *base_ptr  = new_node;

    tree->size++;

    return TREE_NOERR;
}

static void (*VISITOR_FUNCTION_)(Node*, size_t depth) = nullptr;
static size_t VISITOR_DEPTH_ = -1;

static void tree_visitor_(Node* node)
{
    assert(node);

    VISITOR_DEPTH_++;    

    VISITOR_FUNCTION_(node, VISITOR_DEPTH_);

    if(node->right)
        tree_visitor_(node->right);

    if(node->left)
        tree_visitor_(node->left);

    VISITOR_DEPTH_--;
}

tree_err tree_visitor(Tree* tree, void (*function)(Node* node, size_t depth))
{
    ASSERT(tree && function, TREE_NULLPTR);
    ASSERT(tree->ptr_arr, TREE_NOTINIT);

    VISITOR_FUNCTION_ = function;
    VISITOR_DEPTH_    = -1;

    tree_visitor_(tree->root);

    return TREE_NOERR;
}
