#include "btree.h"
using namespace std;

BTreeNode::BTreeNode(int t, bool leaf) : t(t), leaf(leaf), n(0){
    keys = new int[4*t + 2]();
    rids = new int[4*t + 2]();
    children = new BTreeNode*[4*t + 4]();
}

BTreeNode::~BTreeNode(){
    for(int i = 0; i <= n; i++)
        if(children[i]) delete children[i];
    delete[] keys;
    delete[] rids;
    delete[] children;
}

BTreeNode* BTreeNode::search(int k, int &rid){
    int i = 0;
    while(i < n && k > keys[i]) i++;
    if(i < n && keys[i] == k){rid = rids[i]; return this;}
    if(leaf) return nullptr;
    return children[i]->search(k, rid);
}

void BTreeNode::insertNonFull(int k, int rid){
    int i = n - 1;
    if(leaf){
        while(i >= 0 && keys[i] > k){keys[i+1] = keys[i]; rids[i+1] = rids[i]; i--;}
        keys[i+1] = k; rids[i+1] = rid; n++;
    } else{
        while(i >= 0 && keys[i] > k) i--;
        if(children[i+1]->n == 2*t - 1){
            splitChild(i+1, children[i+1]);
            if(keys[i+1] < k) i++;
        }
        children[i+1]->insertNonFull(k, rid);
    }
}

void BTreeNode::splitChild(int i, BTreeNode *y){
    BTreeNode *z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;
    for(int j = 0; j < t-1; j++){z->keys[j] = y->keys[j+t]; z->rids[j] = y->rids[j+t];}
    if(!y->leaf)
        for(int j = 0; j < t; j++){z->children[j] = y->children[j+t]; y->children[j+t] = nullptr;}
    y->n = t - 1;
    for(int j = n; j >= i+1; j--) children[j+1] = children[j];
    children[i+1] = z;
    for(int j = n-1; j >= i; j--){keys[j+1] = keys[j]; rids[j+1] = rids[j];}
    keys[i] = y->keys[t-1];
    rids[i] = y->rids[t-1];
    n++;
}

int BTreeNode::findKey(int k){
    int idx = 0;
    while(idx < n && keys[idx] < k) idx++;
    return idx;
}

bool BTreeNode::remove(int k){
    int idx = findKey(k);
    if(idx < n && keys[idx] == k){
        if(leaf) removeFromLeaf(idx);
        else removeFromNonLeaf(idx);
        return true;
    }
    if(leaf) return false;
    bool last = (idx == n);
    if(children[idx]->n < t) fill(idx);
    if(last && idx > n) return children[idx-1]->remove(k);
    return children[idx]->remove(k);
}

void BTreeNode::removeFromLeaf(int idx){
    for(int i = idx+1; i < n; i++){keys[i-1] = keys[i]; rids[i-1] = rids[i];}
    n--;
}

void BTreeNode::removeFromNonLeaf(int idx){
    int k = keys[idx];
    if(children[idx]->n >= t){
        keys[idx] = getPredecessor(idx); rids[idx] = getPredecessorRid(idx);
        children[idx]->remove(keys[idx]);
    } else if(children[idx+1]->n >= t){
        keys[idx] = getSuccessor(idx); rids[idx] = getSuccessorRid(idx);
        children[idx+1]->remove(keys[idx]);
    } else{
        merge(idx);
        children[idx]->remove(k);
    }
}

int BTreeNode::getPredecessor(int idx){
    BTreeNode *cur = children[idx];
    while(!cur->leaf) cur = cur->children[cur->n];
    return cur->keys[cur->n - 1];
}

int BTreeNode::getPredecessorRid(int idx){
    BTreeNode *cur = children[idx];
    while(!cur->leaf) cur = cur->children[cur->n];
    return cur->rids[cur->n - 1];
}

int BTreeNode::getSuccessor(int idx){
    BTreeNode *cur = children[idx+1];
    while(!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
}

int BTreeNode::getSuccessorRid(int idx){
    BTreeNode *cur = children[idx+1];
    while(!cur->leaf) cur = cur->children[0];
    return cur->rids[0];
}

void BTreeNode::fill(int idx){
    if(idx != 0 && children[idx-1]->n >= t) borrowFromPrev(idx);
    else if(idx != n && children[idx+1]->n >= t) borrowFromNext(idx);
    else{if(idx != n) merge(idx); else merge(idx-1);}
}

void BTreeNode::borrowFromPrev(int idx){
    BTreeNode *child = children[idx], *sib = children[idx-1];
    for(int i = child->n-1; i >= 0; i--){child->keys[i+1] = child->keys[i]; child->rids[i+1] = child->rids[i];}
    if(!child->leaf) for(int i = child->n; i >= 0; i--) child->children[i+1] = child->children[i];
    child->keys[0] = keys[idx-1]; child->rids[0] = rids[idx-1];
    if(!child->leaf) child->children[0] = sib->children[sib->n];
    keys[idx-1] = sib->keys[sib->n-1]; rids[idx-1] = sib->rids[sib->n-1];
    child->n++; sib->n--;
}

void BTreeNode::borrowFromNext(int idx){
    BTreeNode *child = children[idx], *sib = children[idx+1];
    child->keys[child->n] = keys[idx]; child->rids[child->n] = rids[idx];
    if(!child->leaf) child->children[child->n+1] = sib->children[0];
    keys[idx] = sib->keys[0]; rids[idx] = sib->rids[0];
    for(int i = 1; i < sib->n; i++){sib->keys[i-1] = sib->keys[i]; sib->rids[i-1] = sib->rids[i];}
    if(!sib->leaf) for(int i = 1; i <= sib->n; i++) sib->children[i-1] = sib->children[i];
    child->n++; sib->n--;
}

void BTreeNode::merge(int idx){
    BTreeNode *child = children[idx], *sib = children[idx+1];
    int pos = child->n;
    child->keys[pos] = keys[idx]; child->rids[pos] = rids[idx];
    for(int i = 0; i < sib->n; i++){child->keys[pos+1+i] = sib->keys[i]; child->rids[pos+1+i] = sib->rids[i];}
    if(!child->leaf)
        for(int i = 0; i <= sib->n; i++) child->children[pos+1+i] = sib->children[i];
    for(int i = idx+1; i < n; i++){keys[i-1] = keys[i]; rids[i-1] = rids[i];}
    for(int i = idx+2; i <= n; i++) children[i-1] = children[i];
    children[n] = nullptr;
    child->n = pos + sib->n + 1;
    n--;
    sib->n = 0;
    for(int i = 0; i <= (int)(2*sib->t); i++) sib->children[i] = nullptr;
    delete sib;
}

void BTreeNode::traverse(){
    int i;
    for(i = 0; i < n; i++){
        if(!leaf) children[i]->traverse();
        cout << keys[i] << " ";
    }
    if(!leaf) children[i]->traverse();
}

BTree::BTree(int order) : t(order), root(nullptr), splitCount(0){}

BTree::~BTree(){if(root) delete root;}

int BTree::search(int k){
    if(!root) return -1;
    int rid = -1;
    root->search(k, rid);
    return rid;
}

void BTree::insert(int k, int rid){
    if(!root){
        root = new BTreeNode(t, true);
        root->keys[0] = k; root->rids[0] = rid; root->n = 1;
        return;
    }
    if(root->n == 2*t - 1){
        splitCount++;
        BTreeNode *s = new BTreeNode(t, false);
        s->children[0] = root;
        s->splitChild(0, root);
        int i = (s->keys[0] < k) ? 1 : 0;
        s->children[i]->insertNonFull(k, rid);
        root = s;
    } else{
        root->insertNonFull(k, rid);
    }
}

bool BTree::remove(int k){
    if(!root) return false;
    bool res = root->remove(k);
    if(root->n == 0){
        BTreeNode *old = root;
        root = root->leaf ? nullptr : root->children[0];
        old->n = 0;
        for(int i = 0; i <= (int)(2*old->t); i++) old->children[i] = nullptr;
        delete old;
    }
    return res;
}

void BTree::traverse(){
    if(root){root->traverse(); cout << endl;}
}

double BTree::nodeUtilization(){
    if(!root) return 0.0;
    int totalKeys = 0, totalNodes = 0, maxKeys = 2*t - 1;
    struct V{
        int &tk, &tn;
        void visit(BTreeNode *node){
            if(!node) return;
            tk += node->n; tn++;
            if(!node->leaf)
                for(int i = 0; i <= node->n; i++) visit(node->children[i]);
        }
    } v{totalKeys, totalNodes};
    v.visit(root);
    return (double)totalKeys / (totalNodes * maxKeys);
}
