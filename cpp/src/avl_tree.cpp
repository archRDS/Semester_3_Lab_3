#include "avl_tree.h"

using namespace std;

unsigned char AvlTree::getHeight(Node* node) {
    if (node == nullptr) return 0;
    return node->height;
}

int AvlTree::getBalanceFactor(Node* node) {
    return getHeight(node->left) - getHeight(node->right);
}

void AvlTree::fixHeight(Node* node) {
    unsigned char left = getHeight(node->left);
    unsigned char right = getHeight(node->right);
    node->height = (left > right ? left : right) + 1;
}

// Повороты меняют связи узлов, когда дерево теряет баланс.
AvlTree::Node* AvlTree::rotateRight(Node* node) {
    Node* next = node->left;
    node->left = next->right;
    next->right = node;
    fixHeight(node);
    fixHeight(next);
    return next;
}

AvlTree::Node* AvlTree::rotateLeft(Node* node) {
    Node* next = node->right;
    node->right = next->left;
    next->left = node;
    fixHeight(node);
    fixHeight(next);
    return next;
}

// Восстанавливает баланс после вставки или удаления ключа.
AvlTree::Node* AvlTree::balance(Node* node) {
    fixHeight(node);
    if (getBalanceFactor(node) == 2) {
        if (getBalanceFactor(node->left) < 0) {
            node->left = rotateLeft(node->left);
        }
        return rotateRight(node);
    }
    if (getBalanceFactor(node) == -2) {
        if (getBalanceFactor(node->right) > 0) {
            node->right = rotateRight(node->right);
        }
        return rotateLeft(node);
    }
    return node;
}

// Рекурсивно ищет место для нового ключа; одинаковые ключи пропускает.
AvlTree::Node* AvlTree::insert(Node* node, int key) {
    if (node == nullptr) return new Node(key);
    if (key < node->key) node->left = insert(node->left, key);
    else if (key > node->key) node->right = insert(node->right, key);
    else return node;
    return balance(node);
}

AvlTree::Node* AvlTree::findMin(Node* node) {
    if (node->left == nullptr) return node;
    return findMin(node->left);
}

// Удаляет ключ; если есть два потомка, берет минимальный ключ справа.
AvlTree::Node* AvlTree::removeKey(Node* node, int key) {
    if (node == nullptr) return nullptr;
    if (key < node->key) node->left = removeKey(node->left, key);
    else if (key > node->key) node->right = removeKey(node->right, key);
    else {
        if (node->left == nullptr) {
            Node* right = node->right;
            delete node;
            return right;
        }
        if (node->right == nullptr) {
            Node* left = node->left;
            delete node;
            return left;
        }
        Node* min = findMin(node->right);
        node->key = min->key;
        node->right = removeKey(node->right, min->key);
    }
    return balance(node);
}

// Обход слева направо возвращает ключи в порядке возрастания.
void AvlTree::readInOrder(Node* node, vector<int>& result) {
    if (node == nullptr) return;
    readInOrder(node->left, result);
    result.push_back(node->key);
    readInOrder(node->right, result);
}

void AvlTree::readPreOrder(Node* node, int depth, const string& side, string& result) {
    if (node == nullptr) return;
    result += string(depth * 2, ' ') + side + ": " + to_string(node->key) + '\n';
    readPreOrder(node->left, depth + 1, "L", result);
    readPreOrder(node->right, depth + 1, "R", result);
}

void AvlTree::deleteNodes(Node* node) {
    if (node == nullptr) return;
    deleteNodes(node->left);
    deleteNodes(node->right);
    delete node;
}

AvlTree::~AvlTree() {
    deleteNodes(root);
}

void AvlTree::insert(int key) {
    root = insert(root, key);
}

// Ищет ключ, выбирая левое или правое поддерево.
bool AvlTree::contains(int key) const {
    Node* node = root;
    while (node != nullptr) {
        if (key == node->key) return true;
        node = key < node->key ? node->left : node->right;
    }
    return false;
}

bool AvlTree::erase(int key) {
    if (!contains(key)) return false;
    root = removeKey(root, key);
    return true;
}

vector<int> AvlTree::values() const {
    vector<int> result;
    readInOrder(root, result);
    return result;
}

vector<int> AvlTree::levelOrder() const {
    if (root == nullptr) return {};
    vector<const Node*> nodes{root};
    vector<int> result;
    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto* node = nodes[i];
        result.push_back(node->key);
        if (node->left) nodes.push_back(node->left);
        if (node->right) nodes.push_back(node->right);
    }
    return result;
}

string AvlTree::readPreOrder() const {
    if (root == nullptr) return "(empty)";
    string result;
    readPreOrder(root, 0, "ROOT", result);
    result.pop_back();
    return result;
}
