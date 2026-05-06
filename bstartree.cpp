#include "bstartree.h"
using namespace std;

BStarNode::BStarNode(int t, bool leaf) : t(t), leaf(leaf), n(0){
    keys = new int[4*t + 2]();
    rids = new int[4*t + 2]();
    children = new BStarNode*[4*t + 4]();
}

BStarNode::~BStarNode(){
    for(int i = 0; i <= n; i++)
        if(children[i]) delete children[i];
    delete[] keys;
    delete[] rids;
    delete[] children;
}

BStarNode* BStarNode::search(int k, int &rid){
    int i = 0;
    while(i < n && k > keys[i]) i++;
    if(i < n && keys[i] == k){rid = rids[i]; return this;}
    if(leaf) return nullptr;
    return children[i]->search(k, rid);
}

void BStarNode::traverse(){
    int i;
    for(i = 0; i < n; i++){
        if(!leaf) children[i]->traverse();
        cout << keys[i] << " ";
    }
    if(!leaf) children[i]->traverse();
}

// borrow from sibling
bool BStarNode::borrowFromSibling(int i, bool left){
    if(left){
        if(i == 0) return false;
        BStarNode *sib = children[i-1];
        BStarNode *cur = children[i];
        if(sib->n <= t - 1) return false;
        if(cur->n >= 2*t - 1) return false;
        for(int j = cur->n; j > 0; j--){cur->keys[j] = cur->keys[j-1]; cur->rids[j] = cur->rids[j-1];}
        if(!cur->leaf)
            for(int j = cur->n; j >= 0; j--) cur->children[j+1] = cur->children[j];
        cur->keys[0] = keys[i-1];
        cur->rids[0] = rids[i-1];
        if(!cur->leaf) cur->children[0] = sib->children[sib->n];
        keys[i-1] = sib->keys[sib->n - 1];
        rids[i-1] = sib->rids[sib->n - 1];
        sib->n--; cur->n++;
        return true;
    } else{
        if(i >= n) return false;
        BStarNode *sib = children[i+1];
        BStarNode *cur = children[i];
        if(sib->n <= t - 1) return false;
        if(cur->n >= 2*t - 1) return false;
        cur->keys[cur->n] = keys[i];
        cur->rids[cur->n] = rids[i];
        if(!cur->leaf) cur->children[cur->n+1] = sib->children[0];
        keys[i] = sib->keys[0];
        rids[i] = sib->rids[0];
        for(int j = 1; j < sib->n; j++){sib->keys[j-1] = sib->keys[j]; sib->rids[j-1] = sib->rids[j];}
        if(!sib->leaf)
            for(int j = 1; j <= sib->n; j++) sib->children[j-1] = sib->children[j];
        sib->n--; cur->n++;
        return true;
    }
}

// 2->3 split
void BStarNode::splitTwoIntoThree(int i){
    BStarNode *left = children[i];
    BStarNode *right = children[i+1];
    bool isLeaf = left->leaf;
    int total = left->n + 1 + right->n;

    int *tmpK = new int[total + 1]();
    int *tmpR = new int[total + 1]();
    BStarNode **tmpC = new BStarNode*[total + 2]();

    for(int j = 0; j < left->n; j++){
        tmpK[j] = left->keys[j];
        tmpR[j] = left->rids[j];
        if(!isLeaf) tmpC[j] = left->children[j];
    }
    if(!isLeaf) tmpC[left->n] = left->children[left->n];

    int pk = left->n;
    tmpK[pk] = keys[i];
    tmpR[pk] = rids[i];

    for(int j = 0; j < right->n; j++){
        tmpK[pk+1+j] = right->keys[j];
        tmpR[pk+1+j] = right->rids[j];
        if(!isLeaf) tmpC[pk+1+j] = right->children[j];
    }
    if(!isLeaf) tmpC[pk+1+right->n] = right->children[right->n];

    if(!isLeaf){
        for(int j = 0; j <= 2*t; j++){
            left->children[j] = nullptr;
            right->children[j] = nullptr;
        }
    }

    int sz1 = (total - 2) / 3;
    int sz2 = (total - 2 - sz1) / 2;
    int mid1 = sz1;
    int mid2 = sz1 + 1 + sz2;
    int sz3 = total - mid2 - 1;

    left->n = sz1;
    for(int j = 0; j < sz1; j++){
        left->keys[j] = tmpK[j];
        left->rids[j] = tmpR[j];
        if(!isLeaf) left->children[j] = tmpC[j];
    }
    if(!isLeaf) left->children[sz1] = tmpC[sz1];

    BStarNode *mid = new BStarNode(t, isLeaf);
    mid->n = sz2;
    for(int j = 0; j < sz2; j++){
        mid->keys[j] = tmpK[sz1+1+j];
        mid->rids[j] = tmpR[sz1+1+j];
        if(!isLeaf) mid->children[j] = tmpC[sz1+1+j];
    }
    if(!isLeaf) mid->children[sz2] = tmpC[sz1+1+sz2];

    right->n = sz3;
    for(int j = 0; j < sz3; j++){
        right->keys[j] = tmpK[mid2+1+j];
        right->rids[j] = tmpR[mid2+1+j];
        if(!isLeaf) right->children[j] = tmpC[mid2+1+j];
    }
    if(!isLeaf) right->children[sz3] = tmpC[mid2+1+sz3];

    for(int j = n; j > i+1; j--){
        keys[j] = keys[j-1];
        rids[j] = rids[j-1];
        children[j+1] = children[j];
    }
    keys[i] = tmpK[mid1];
    rids[i] = tmpR[mid1];
    keys[i+1] = tmpK[mid2];
    rids[i+1] = tmpR[mid2];
    children[i+1] = mid;
    children[i+2] = right;
    n++;

    delete[] tmpK;
    delete[] tmpR;
    delete[] tmpC;
}

// 1->2 split (root)
void BStarNode::splitChild(int i, BStarNode *y){
    BStarNode *z = new BStarNode(y->t, y->leaf);
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

// insert B* strategy
void BStarNode::insertNonFull(int k, int rid){
    int i = n - 1;
    if(leaf){
        while(i >= 0 && keys[i] > k){keys[i+1] = keys[i]; rids[i+1] = rids[i]; i--;}
        keys[i+1] = k; rids[i+1] = rid; n++;
        return;
    }
    while(i >= 0 && keys[i] > k) i--;
    i++;

    if(children[i]->n == 2*t - 1){
        if(borrowFromSibling(i, true)){
            if(i > 0 && keys[i-1] >= k) i--;
        } else if(borrowFromSibling(i, false)){
            if(i < n && keys[i] <= k) i++;
        } else{
            if(i > 0 && children[i-1]->n == 2*t - 1){
                splitTwoIntoThree(i-1);
                i--;
                while(i < n && keys[i] < k) i++;
            } else if(i < n && children[i+1] != nullptr && children[i+1]->n == 2*t - 1){
                splitTwoIntoThree(i);
                if(keys[i] < k){
                    i++;
                    if(i < n && keys[i] < k) i++;
                }
            } else{
                splitChild(i, children[i]);
                if(keys[i] < k) i++;
            }
        }
    }
    children[i]->insertNonFull(k, rid);
}

// delete helpers
int BStarNode::findKey(int k){
    int idx = 0;
    while(idx < n && keys[idx] < k) idx++;
    return idx;
}

bool BStarNode::remove(int k){
    int idx = findKey(k);
    if(idx < n && keys[idx] == k){
        if(leaf) removeFromLeaf(idx);
        else removeFromNonLeaf(idx);
        return true;
    }
    if(leaf) return false;
    bool last = (idx == n);
    if(children[idx] && children[idx]->n < t) fill(idx);
    if(last && idx > n) return children[idx-1]->remove(k);
    if(!children[idx]) return false;
    return children[idx]->remove(k);
}

void BStarNode::removeFromLeaf(int idx){
    for(int i = idx+1; i < n; i++){keys[i-1] = keys[i]; rids[i-1] = rids[i];}
    n--;
}

void BStarNode::removeFromNonLeaf(int idx){
    int k = keys[idx];
    if(children[idx] && children[idx]->n >= t){
        keys[idx] = getPredecessor(idx); rids[idx] = getPredecessorRid(idx);
        children[idx]->remove(keys[idx]);
    } else if(children[idx+1] && children[idx+1]->n >= t){
        keys[idx] = getSuccessor(idx); rids[idx] = getSuccessorRid(idx);
        children[idx+1]->remove(keys[idx]);
    } else{
        merge(idx);
        if(children[idx]) children[idx]->remove(k);
    }
}

int BStarNode::getPredecessor(int idx){
    BStarNode *cur = children[idx];
    while(!cur->leaf) cur = cur->children[cur->n];
    return cur->keys[cur->n - 1];
}

int BStarNode::getPredecessorRid(int idx){
    BStarNode *cur = children[idx];
    while(!cur->leaf) cur = cur->children[cur->n];
    return cur->rids[cur->n - 1];
}

int BStarNode::getSuccessor(int idx){
    BStarNode *cur = children[idx+1];
    while(!cur->leaf) cur = cur->children[0];
    return cur->keys[0];
}

int BStarNode::getSuccessorRid(int idx){
    BStarNode *cur = children[idx+1];
    while(!cur->leaf) cur = cur->children[0];
    return cur->rids[0];
}

void BStarNode::fill(int idx){
    if(idx != 0 && children[idx-1] && children[idx-1]->n >= t)
        borrowFromPrev(idx);
    else if(idx != n && children[idx+1] && children[idx+1]->n >= t)
        borrowFromNext(idx);
    else{if(idx != n) merge(idx); else merge(idx-1);}
}

void BStarNode::borrowFromPrev(int idx){
    BStarNode *child = children[idx], *sib = children[idx-1];
    for(int i = child->n-1; i >= 0; i--){child->keys[i+1] = child->keys[i]; child->rids[i+1] = child->rids[i];}
    if(!child->leaf) for(int i = child->n; i >= 0; i--) child->children[i+1] = child->children[i];
    child->keys[0] = keys[idx-1]; child->rids[0] = rids[idx-1];
    if(!child->leaf) child->children[0] = sib->children[sib->n];
    keys[idx-1] = sib->keys[sib->n-1]; rids[idx-1] = sib->rids[sib->n-1];
    child->n++; sib->n--;
}

void BStarNode::borrowFromNext(int idx){
    BStarNode *child = children[idx], *sib = children[idx+1];
    child->keys[child->n] = keys[idx]; child->rids[child->n] = rids[idx];
    if(!child->leaf) child->children[child->n+1] = sib->children[0];
    keys[idx] = sib->keys[0]; rids[idx] = sib->rids[0];
    for(int i = 1; i < sib->n; i++){sib->keys[i-1] = sib->keys[i]; sib->rids[i-1] = sib->rids[i];}
    if(!sib->leaf) for(int i = 1; i <= sib->n; i++) sib->children[i-1] = sib->children[i];
    child->n++; sib->n--;
}

void BStarNode::merge(int idx){
    BStarNode *child = children[idx];
    BStarNode *sib = children[idx+1];
    if(!child || !sib) return;
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
    for(int i = 0; i <= (int)(4*sib->t + 3); i++) sib->children[i] = nullptr;
    delete sib;
}

// BStarTree
BStarTree::BStarTree(int order) : t(order), root(nullptr), splitCount(0), redistributeCount(0){}

BStarTree::~BStarTree(){if(root) delete root;}

int BStarTree::search(int k){
    if(!root) return -1;
    int rid = -1;
    root->search(k, rid);
    return rid;
}

void BStarTree::insert(int k, int rid){
    if(!root){
        root = new BStarNode(t, true);
        root->keys[0] = k; root->rids[0] = rid; root->n = 1;
        return;
    }
    if(root->n == 2*t - 1){
        splitCount++;
        BStarNode *s = new BStarNode(t, false);
        s->children[0] = root;
        s->splitChild(0, root);
        int i = (s->keys[0] < k) ? 1 : 0;
        s->children[i]->insertNonFull(k, rid);
        root = s;
    } else{
        root->insertNonFull(k, rid);
    }
}

bool BStarTree::remove(int k){
    if(!root) return false;
    bool res = root->remove(k);
    if(root->n == 0){
        BStarNode *old = root;
        root = root->leaf ? nullptr : root->children[0];
        old->n = 0;
        for(int i = 0; i <= (int)(2*old->t); i++) old->children[i] = nullptr;
        delete old;
    }
    return res;
}

void BStarTree::traverse(){
    if(root){root->traverse(); cout << endl;}
}

double BStarTree::nodeUtilization(){
    if(!root) return 0.0;
    int totalKeys = 0, totalNodes = 0, maxKeys = 2*t - 1;
    struct V{
        int &tk, &tn;
        void visit(BStarNode *node){
            if(!node) return;
            tk += node->n; tn++;
            if(!node->leaf)
                for(int i = 0; i <= node->n; i++) visit(node->children[i]);
        }
    } v{totalKeys, totalNodes};
    v.visit(root);
    return (double)totalKeys / (totalNodes * maxKeys);
}
