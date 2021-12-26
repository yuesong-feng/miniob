#pragma once
#include <vector>

template<typename KeyT, typename ValT>
class Node{
public:
    bool leaf;
    Node *parent;   //for non-root only
    Node *next;     //for leaf only
    std::vector<KeyT> key;
    std::vector<Node*> ptr2node;    //for non-leaf only
    std::vector<ValT*> ptr2val;     //for leaf only
    Node(bool _leaf = false);
};

template<typename KeyT, typename ValT>
class BPTree{
private:
    Node<KeyT, ValT> *root;
    inline int keyIndex(Node<KeyT, ValT> *_node, KeyT _key);
    inline std::pair<Node<KeyT, ValT>*, int> keyIndexInLeaf(KeyT _key);
    Node<KeyT, ValT>* splitLeaf(Node<KeyT, ValT>* _leaf);
    void createIndex(Node<KeyT, ValT>* _new_node, KeyT _index);
    std::pair<Node<KeyT, ValT>*, KeyT> splitNode(Node<KeyT, ValT>* _node);

public:
    BPTree();
    void insert(KeyT _key, ValT _val);
    void erase(KeyT _key);

    ValT* find(KeyT _key);
    void display();
    void scan();
};

