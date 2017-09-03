#ifndef LIST_H
#define LIST_H

#include <iterator>
#include <vector>
#include <type_traits>
#include <cassert>
#include <algorithm>

namespace exam {

using std::cout;
using std::endl;

template<typename T>
struct list {


    template<typename TT>
    struct iterator_impl;

    using iterator = iterator_impl<T>;
    using const_iterator = iterator_impl<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    list() { // without precondition
        start = finish = new snode;
        assert(start != nullptr);
        assert(start->left == nullptr && start->right == nullptr);
    }

    list(list const& other) : // without precondition
        start(other.start),
        finish(other.finish),
        sz(other.sz)
    {
        snode* cur = start;
        while (cur != finish) {
            snode* nnode = static_cast<snode*>(new node(static_cast<node*>(cur->right)));
            cur->right = nnode;
            nnode->left = cur;
            cur = nnode;
        }
        if (cur) cur->right = finish;
    }

    list& operator = (list const& other) { // without precondition
        list tmp = other;
        swap(*this, tmp);
        snode* cur = start;
        while (cur != finish) {
            cur->its.clear();
            cur->cits.clear();
        }
        return *this;
    }

    bool empty() noexcept {
        return sz == 0;
    }

    size_t size() noexcept {
        return sz;
    }

    void clear() {
        while (start != finish) {
            start = start->right;
            delete start->left;
        }
        sz = 0;
    }

    void push_back(T const& value) {
        snode *nnode = new node;
        static_cast<node*>(nnode)->value = value;
        nnode->left = finish->left;
        nnode->right = finish;
        if (nnode->left) nnode->left->right = nnode;
        finish->left = nnode;
        if (sz == 0) start = nnode;
        sz++;
    }

    void push_front(T const& value) {
        snode *nnode = new node;
        static_cast<node*>(nnode)->value = value;
        nnode->right = start;
        start->left = nnode;
        start = nnode;
        sz++;
    }

    void pop_back() {
        assert(sz != 0);
        snode *&lst = finish->left;
        if (lst->left) lst->left->right = finish;
        finish->left = lst->left;
        for (auto& it : lst->its) it->is_valid = false, it->lst = nullptr;
        for (auto& it : lst->cits) it->is_valid = false, it->lst = nullptr;
        delete lst;
        lst = nullptr;
        sz--;
    }

    void pop_front() {
        assert(sz != 0);
        start = start->right;
        for (auto& it : start->left->its) it->is_valid = false, it->lst = nullptr;
        for (auto& it : start->left->cits) it->is_valid = false, it->lst = nullptr;
        delete start->left;
        start->left = nullptr;
        sz--;
    }

    T& front() {
        assert(sz != 0);
        return static_cast<node*>(start)->value;
    }

    T& back() {
        assert(sz != 0);
        return static_cast<node*>(finish->left)->value;
    }

    iterator begin() {
        return iterator(start, this);
    }

    const_iterator begin() const {
        return const_iterator(start, this);
    }

    iterator end() {
        return iterator(finish, this);
    }

    const_iterator end() const {
        return const_iterator(finish, this);
    }

    void splice(const_iterator pos, list& x, const_iterator first, const_iterator last) {
        assert(first.lst == last.lst);
        assert(first.lst == &x);

        snode*& b = first.current;
        snode*& e = last.current;

        if (b->left)
            b->left->right = e;
        else
            x.start = e;

        if (pos.current->left) {
            pos.current->left->right = b;
            b->left = pos.current->left;
        } else {
            start = b;
            b->left = nullptr;
        }

        pos.current->left = e->left;
        if (e->left) e->left->right = pos.current;
        e->left = b->left;

        snode* cur = b;

        while (cur != pos.current->left) {
            if (cur == nullptr) {
                assert(false);
            }
            for (auto& it : cur->its) it->lst = pos.lst;
            for (auto& it : cur->cits) it->lst = pos.lst;
            cur = cur->right;
        }

        snode* nnode = start;
        while (nnode != finish) {
            nnode = nnode->right;
        }
    }

    void insert(const_iterator it, T const& value) {
        assert(it.lst == this);
        snode* nnode = new node;
        static_cast<node*>(nnode)->value = value;
        snode *& nxt = it.current;
        nnode->left = nxt->left;
        nnode->right = nxt;
        nxt->left = nnode;
        if (nnode->left) nnode->left->right = nnode;
        else start = nnode;
        sz++;
    }

    iterator erase(const_iterator iter) {
        assert(iter.lst == this);
        assert(sz != 0);
        snode*& cur = iter.current;

        for (auto& it : cur->cits) {
            it->is_valid = false;
            it->lst = nullptr;
        }

        for (auto& it : cur->its) {
            it->is_valid = false;
            it->lst = nullptr;
        }

        snode* tmp = cur->right;

        if (cur->left)
            cur->left->right = cur->right;

        if (cur->right)
            cur->right->left = cur->left;

        delete cur;
        cur = nullptr;

        if (!tmp->left) start = tmp;

        return iterator(tmp, this);
    }


    friend void swap(list& lhs, list& rhs) {
        using std::swap;
        swap(lhs.start, rhs.start);
        swap(lhs.finish, rhs.finish);
        swap(lhs.sz, rhs.sz);
    }

    ~list() {
        snode*& cur = start;
        while (cur != finish) {
            for (auto& it : cur->cits) {
                it->is_valid = false;
                it->lst = nullptr;
            }
            for (auto& it : cur->its) {
                it->is_valid = false;
                it->lst = nullptr;
            }
            cur = cur->right;
            delete cur->left;
        }
        delete cur;
    }

private:
    struct snode {

        snode() = default;

        snode(snode* other) :
            left(other->left),
            right(other->right),
            its(other->its),
            cits(other->cits)
        {}

        snode* left {};
        snode* right {};

        std::vector<iterator*> its;
        std::vector<const_iterator*> cits;


        void add(const_iterator* it) {
            cits.push_back(it);
        }

        void add(iterator* it) {
            its.push_back(it);
        }

        void del(const_iterator* it) {
            if (find(cits.begin(), cits.end(), it) != cits.end())
                cits.erase(find(cits.begin(), cits.end(), it));
            else {
                cout << "del_const_iterator" << endl;
                abort();
            }
        }

        void del(iterator* it) {
            if (find(its.begin(), its.end(), it) != its.end())
                its.erase(find(its.begin(), its.end(), it));
            else {
                cout << "del_iterator" << endl;
                abort();
            }
        }

        virtual ~snode() {};
    };


    struct node : public snode {
        node() = default;

        node(node* other) :
            snode{other},
            value(other->value)
        {}

        T value {};
    };

    snode* start {};
    snode* finish {};

    size_t sz {};

};

template <typename R> template <typename T>
struct list<R>::iterator_impl {

    ~iterator_impl() {
        if (is_valid)
            current->del(this);
    }

    iterator_impl(typename std::conditional<std::is_const<T>::value, snode* const&, snode*&>::type other, const list<R>* p) :
        current(other),
        lst(p)
    {
        other->add(this);
    }

    iterator_impl& operator ++() {
        assert(is_valid);
        assert(lst->finish != current);
        current->del(this);
        current = current->right;
        current->add(this);
        return *this;
    }

    iterator_impl operator ++(int) {
        iterator_impl tmp = *this;
        ++(*this);
        return tmp;
    }

    iterator_impl& operator --() {
        assert(is_valid);
        assert(current != lst->start);
        current->del(this);
        current = current->left;
        current->add(this);
        return *this;
    }

    iterator_impl operator --(int) {
        iterator_impl tmp = *this;
        --(*this);
        return tmp;
    }

    T& operator *() const { // ***
        assert(is_valid);
        assert(dynamic_cast<node*>(current) != nullptr);
        return static_cast<node*>(current)->value;
    }

    friend bool operator ==(iterator_impl lhs, iterator_impl rhs) {
        assert(lhs.is_valid && rhs.is_valid);
        return (lhs.current == rhs.current);
    }

    friend bool operator !=(iterator_impl lhs, iterator_impl rhs) {
        return !(lhs == rhs);
    }

    template<typename TT>
    iterator_impl(iterator_impl<TT> const& other,
                  typename std::enable_if<std::is_same<T, const TT>::value>::type * = nullptr):
        is_valid(other.is_valid),
        current(other.current),
        lst(other.lst)
    {
        current->add(this);
    }

    iterator_impl(iterator_impl<T> const& other):
        is_valid(other.is_valid),
        current(other.current),
        lst(other.lst)
    {
        current->add(this);
    }

    template<typename TT, class = typename std::enable_if<std::is_same<T, const TT>::value>::type>
    iterator_impl operator =(iterator_impl<TT> const& other) {
        if (is_valid) {
            current->del(this);
        }
        is_valid = other.is_valid;
        current = other.current;
        lst = other.lst;
        current->add(this);
        return *this;
    }

    iterator_impl operator =(iterator_impl<T> const& other) {
        if (is_valid) {
            current->del(this);
        }
        is_valid = other.is_valid;
        current = other.current;
        lst = other.lst;
        current->add(this);
        return *this;
    }

    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::bidirectional_iterator_tag;

    friend list<R>;

private:

    bool is_valid {true};
    snode* current {};
    const list<R>* lst {};

};

}
#endif // LIST_H