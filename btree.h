#ifndef BTREE_H
#define BTREE_H

#include <iostream>

class BTreeNode{
private:
    int *keys;
    int *rids;
    BTreeNode **children;
    int n;
    bool leaf;
    int t;

    friend class BTree;

public:
    BTreeNode(int t, bool leaf);
    ~BTreeNode();

    void insertNonFull(int k, int rid);
    void splitChild(int i, BTreeNode *y);
    BTreeNode* search(int k, int &rid);
    bool remove(int k);
    void traverse();

private:
    int findKey(int k);
    void removeFromLeaf(int idx);
    void removeFromNonLeaf(int idx);
    int getPredecessor(int idx);
    int getPredecessorRid(int idx);
    int getSuccessor(int idx);
    int getSuccessorRid(int idx);
    void fill(int idx);
    void borrowFromPrev(int idx);
    void borrowFromNext(int idx);
    void merge(int idx);
};

class BTree{
private:
    BTreeNode *root;
    int t;
    int splitCount;

public:
    BTree(int order);
    ~BTree();

    void insert(int k, int rid);
    int search(int k);
    bool remove(int k);
    void traverse();

    int getSplitCount() const {return splitCount;}
    void resetStats() {splitCount = 0;}
    double nodeUtilization();
};

#endif
