#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <iostream>
#include <vector>

class BPlusNode{
public:
    int *keys;
    int *rids;
    BPlusNode **children;
    BPlusNode *next;
    int n;
    bool leaf;
    int t;

    BPlusNode(int t, bool leaf);
    ~BPlusNode();
};

class BPlusTree{
    BPlusNode *root;
    int t;
    int splitCount;

    BPlusNode* findParent(BPlusNode *cursor, BPlusNode *child);

public:
    BPlusTree(int order);
    ~BPlusTree();

    void insert(int k, int rid);
    int search(int k);
    bool remove(int k);
    std::vector<int> rangeQuery(int lo, int hi);
    void traverse();

    int getSplitCount() const {return splitCount;}
    void resetStats() {splitCount = 0;}
    double nodeUtilization();
};

#endif
