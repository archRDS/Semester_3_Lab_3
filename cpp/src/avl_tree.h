#pragma once

#include <string>
#include <vector>

class AvlTree {
public:
    AvlTree() = default;
    ~AvlTree();
    AvlTree(const AvlTree&) = delete;
    AvlTree& operator=(const AvlTree&) = delete;

    void insert(int key);
    bool contains(int key) const;
    bool erase(int key);
    std::vector<int> values() const;
    std::vector<int> levelOrder() const;
    std::string readPreOrder() const;

private:
    struct Node {
        explicit Node(int value) : key(value) {}
        int key;
        Node* left = nullptr;
        Node* right = nullptr;
        unsigned char height = 1;
    };

    Node* root = nullptr;

    static unsigned char getHeight(Node* node);
    static int getBalanceFactor(Node* node);
    static void fixHeight(Node* node);
    static Node* rotateRight(Node* node);
    static Node* rotateLeft(Node* node);
    static Node* balance(Node* node);
    static Node* insert(Node* node, int key);
    static Node* findMin(Node* node);
    static Node* removeKey(Node* node, int key);
    static void readInOrder(Node* node, std::vector<int>& result);
    static void readPreOrder(Node* node, int depth, const std::string& side, std::string& result);
    static void deleteNodes(Node* node);
};
