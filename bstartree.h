#ifndef BSTARTREE_H
#define BSTARTREE_H

#include <iostream>

class BStarNode{
private:
    int *keys;
    int *rids;
    BStarNode **children;
    int n;
    bool leaf;
    int t;

    friend class BStarTree;

public:
    BStarNode(int t, bool leaf);
    ~BStarNode();

    void insertNonFull(int k, int rid);
    void splitChild(int i, BStarNode *y);
    void splitTwoIntoThree(int i);
    bool borrowFromSibling(int i, bool left);
    BStarNode* search(int k, int &rid);
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

class BStarTree{
private:
    BStarNode *root;
    int t;
    int splitCount;
    int redistributeCount;

public:
    BStarTree(int order);
    ~BStarTree();

    void insert(int k, int rid);
    int search(int k);
    bool remove(int k);
    void traverse();

    int getSplitCount() const {return splitCount;}
    int getRedistributeCount() const {return redistributeCount;}
    void resetStats() {splitCount = 0; redistributeCount = 0;}
    double nodeUtilization();
};

#endif
