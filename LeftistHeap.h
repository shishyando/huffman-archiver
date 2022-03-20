#pragma once

#include <functional>
#include <stdexcept>

template <class T, class Compare = std::less<T>>
class LeftistHeap {
public:
    LeftistHeap() = default;
    explicit LeftistHeap(const T& value);
    explicit LeftistHeap(const std::vector<T>& values);
    void Insert(const T& value);
    T Extract();
    T Top();
    [[nodiscard]] bool Empty() const;
    [[nodiscard]] size_t Size() const;
    ~LeftistHeap();

private:
    struct Node {
        explicit Node(const T& value);
        static size_t Dist(Node* v);
        T val;
        Node* left = nullptr;
        Node* right = nullptr;
        size_t dist = 0;
    };

    static Node* Merge(Node* x, Node* y);
    static void Delete(Node* node);

    Node* root_ = nullptr;
    size_t size_ = 0;
};

template <class T, class Compare>
LeftistHeap<T, Compare>::Node::Node(const T& value) : val(value) {
    dist = 1;
}

template <class T, class Compare>
size_t LeftistHeap<T, Compare>::Node::Dist(LeftistHeap::Node* v) {
    return v == nullptr ? 0 : v->dist;
}

template <class T, class Compare>
LeftistHeap<T, Compare>::LeftistHeap(const T& value) {
    root_ = new Node(value);
    size_ = 1;
}

template <class T, class Compare>
LeftistHeap<T, Compare>::LeftistHeap(const std::vector<T>& values) {
    *this = LeftistHeap();
    for (const auto& x : values) {
        this->Insert(x);
    }
}

template <class T, class Compare>
typename LeftistHeap<T, Compare>::Node* LeftistHeap<T, Compare>::Merge(LeftistHeap::Node* x, LeftistHeap::Node* y) {
    if (x == nullptr) {
        return y;
    }
    if (y == nullptr) {
        return x;
    }
    if (Compare()(y->val, x->val)) {
        std::swap(x, y);
    }
    x->right = Merge(x->right, y);
    if (Node::Dist(x->right) > Node::Dist(x->left)) {
        std::swap(x->left, x->right);
    }
    x->dist = Node::Dist(x->right) + 1;
    return x;
}

template <class T, class Compare>
void LeftistHeap<T, Compare>::Insert(const T& value) {
    auto* new_heap = new LeftistHeap<T, Compare>(value);
    root_ = Merge(new_heap->root_, root_);
    size_ += new_heap->size_;
}

template <class T, class Compare>
T LeftistHeap<T, Compare>::Top() {
    if (Empty()) {
        throw std::runtime_error("Error: No top element in an empty heap");
    }
    if (root_ == nullptr) {
        throw std::runtime_error("Error: Heap root is invalid");
    }
    return root_->val;
}

template <class T, class Compare>
T LeftistHeap<T, Compare>::Extract() {
    if (Empty()) {
        throw std::runtime_error("Error: Extracting from an empty heap");
    }
    if (root_ == nullptr) {
        throw std::runtime_error("Error: Heap root is invalid");
    }
    T top = root_->val;
    auto left = root_->left;
    auto right = root_->right;
    delete root_;
    root_ = Merge(left, right);
    --size_;
    return top;
}

template <class T, class Compare>
bool LeftistHeap<T, Compare>::Empty() const {
    return size_ == 0;
}

template <class T, class Compare>
size_t LeftistHeap<T, Compare>::Size() const {
    return size_;
}

template <class T, class Compare>
void LeftistHeap<T, Compare>::Delete(LeftistHeap::Node* node) {
    if (node == nullptr) {
        return;
    }
    Delete(node->left);
    Delete(node->right);
    delete node;
}

template <class T, class Compare>
LeftistHeap<T, Compare>::~LeftistHeap() {
    Delete(root_);
}
