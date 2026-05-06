#include "bplustree.h"
using namespace std;

BPlusNode::BPlusNode(int t, bool leaf) : t(t), leaf(leaf), n(0), next(nullptr){
    keys = new int[2*t + 1]();
    rids = new int[2*t + 1]();
    children = new BPlusNode*[2*t + 2]();
}

BPlusNode::~BPlusNode(){
    if(!leaf)
        for(int i = 0; i <= n; i++)
            if(children[i]) delete children[i];
    delete[] keys;
    delete[] rids;
    delete[] children;
}

BPlusTree::BPlusTree(int order) : t(order), root(nullptr), splitCount(0){}
BPlusTree::~BPlusTree(){if(root) delete root;}

int BPlusTree::search(int k){
    if(!root) return -1;
    BPlusNode *cur = root;
    while(!cur->leaf){
        int i = 0;
        while(i < cur->n && k >= cur->keys[i]) i++;
        cur = cur->children[i];
    }
    for(int i = 0; i < cur->n; i++)
        if(cur->keys[i] == k) return cur->rids[i];
    return -1;
}

vector<int> BPlusTree::rangeQuery(int lo, int hi){
    vector<int> result;
    if(!root) return result;
    BPlusNode *cur = root;
    while(!cur->leaf){
        int i = 0;
        while(i < cur->n && lo >= cur->keys[i]) i++;
        cur = cur->children[i];
    }
    while(cur){
        for(int i = 0; i < cur->n; i++){
            if(cur->keys[i] > hi) return result;
            if(cur->keys[i] >= lo) result.push_back(cur->rids[i]);
        }
        cur = cur->next;
    }
    return result;
}

BPlusNode* BPlusTree::findParent(BPlusNode *cursor, BPlusNode *child){
    if(cursor->leaf) return nullptr;
    for(int i = 0; i <= cursor->n; i++){
        if(cursor->children[i] == child) return cursor;
        BPlusNode *res = findParent(cursor->children[i], child);
        if(res) return res;
    }
    return nullptr;
}

void BPlusTree::insert(int k, int rid){
    if(!root){
        root = new BPlusNode(t, true);
        root->keys[0] = k; root->rids[0] = rid; root->n = 1;
        return;
    }
    BPlusNode *cur = root;
    while(!cur->leaf){
        int i = 0;
        while(i < cur->n && k >= cur->keys[i]) i++;
        cur = cur->children[i];
    }
    int i = cur->n - 1;
    while(i >= 0 && cur->keys[i] > k){cur->keys[i+1] = cur->keys[i]; cur->rids[i+1] = cur->rids[i]; i--;}
    cur->keys[i+1] = k; cur->rids[i+1] = rid; cur->n++;

    if(cur->n <= 2*t - 1) return;

    splitCount++;
    BPlusNode *newLeaf = new BPlusNode(t, true);
    int mid = t;
    newLeaf->n = cur->n - mid;
    for(int j = 0; j < newLeaf->n; j++){newLeaf->keys[j] = cur->keys[j+mid]; newLeaf->rids[j] = cur->rids[j+mid];}
    cur->n = mid;
    newLeaf->next = cur->next;
    cur->next = newLeaf;
    int pushKey = newLeaf->keys[0];

    if(cur == root){
        BPlusNode *newRoot = new BPlusNode(t, false);
        newRoot->keys[0] = pushKey; newRoot->n = 1;
        newRoot->children[0] = cur; newRoot->children[1] = newLeaf;
        root = newRoot;
        return;
    }

    BPlusNode *child = newLeaf;
    int childKey = pushKey;
    while(true){
        BPlusNode *par = findParent(root, cur);
        if(!par) break;
        int j = par->n - 1;
        while(j >= 0 && par->keys[j] > childKey){par->keys[j+1] = par->keys[j]; par->children[j+2] = par->children[j+1]; j--;}
        par->keys[j+1] = childKey; par->children[j+2] = child; par->n++;

        if(par->n <= 2*t - 1) break;

        splitCount++;
        BPlusNode *newInternal = new BPlusNode(t, false);
        int mid2 = t - 1;
        int promoteKey = par->keys[mid2];
        newInternal->n = par->n - mid2 - 1;
        for(int x = 0; x < newInternal->n; x++) newInternal->keys[x] = par->keys[x+mid2+1];
        for(int x = 0; x <= newInternal->n; x++) newInternal->children[x] = par->children[x+mid2+1];
        par->n = mid2;

        if(par == root){
            BPlusNode *newRoot = new BPlusNode(t, false);
            newRoot->keys[0] = promoteKey; newRoot->n = 1;
            newRoot->children[0] = par; newRoot->children[1] = newInternal;
            root = newRoot;
            break;
        }
        cur = par; child = newInternal; childKey = promoteKey;
    }
}

bool BPlusTree::remove(int k){
    if(!root) return false;
    BPlusNode *cur = root, *parent = nullptr;
    int parIdx = 0;
    while(!cur->leaf){
        int i = 0;
        while(i < cur->n && k >= cur->keys[i]) i++;
        parent = cur; parIdx = i; cur = cur->children[i];
    }
    int idx = -1;
    for(int i = 0; i < cur->n; i++){if(cur->keys[i] == k){idx = i; break;}}
    if(idx == -1) return false;

    for(int i = idx+1; i < cur->n; i++){cur->keys[i-1] = cur->keys[i]; cur->rids[i-1] = cur->rids[i];}
    cur->n--;

    if(idx == 0 && parent){
        for(int i = 1; i <= parent->n; i++){
            if(parent->children[i] == cur && i <= parent->n){
                if(cur->n > 0) parent->keys[i-1] = cur->keys[0];
                break;
            }
        }
    }

    int minKeys = t - 1;
    if(cur == root){if(root->n == 0){delete root; root = nullptr;} return true;}
    if(cur->n >= minKeys) return true;

    if(parIdx > 0){
        BPlusNode *leftSib = parent->children[parIdx-1];
        if(leftSib->n > minKeys){
            for(int i = cur->n; i > 0; i--){cur->keys[i] = cur->keys[i-1]; cur->rids[i] = cur->rids[i-1];}
            cur->keys[0] = leftSib->keys[leftSib->n-1]; cur->rids[0] = leftSib->rids[leftSib->n-1];
            leftSib->n--; cur->n++;
            parent->keys[parIdx-1] = cur->keys[0];
            return true;
        }
    }
    if(parIdx < parent->n){
        BPlusNode *rightSib = parent->children[parIdx+1];
        if(rightSib->n > minKeys){
            cur->keys[cur->n] = rightSib->keys[0]; cur->rids[cur->n] = rightSib->rids[0]; cur->n++;
            for(int i = 1; i < rightSib->n; i++){rightSib->keys[i-1] = rightSib->keys[i]; rightSib->rids[i-1] = rightSib->rids[i];}
            rightSib->n--;
            parent->keys[parIdx] = rightSib->keys[0];
            return true;
        }
    }

    if(parIdx > 0){
        BPlusNode *leftSib = parent->children[parIdx-1];
        for(int i = 0; i < cur->n; i++){leftSib->keys[leftSib->n+i] = cur->keys[i]; leftSib->rids[leftSib->n+i] = cur->rids[i];}
        leftSib->n += cur->n; leftSib->next = cur->next;
        for(int i = parIdx; i < parent->n; i++){parent->keys[i-1] = parent->keys[i]; parent->children[i] = parent->children[i+1];}
        parent->n--;
        cur->n = 0; delete cur;
    } else{
        BPlusNode *rightSib = parent->children[parIdx+1];
        for(int i = 0; i < rightSib->n; i++){cur->keys[cur->n+i] = rightSib->keys[i]; cur->rids[cur->n+i] = rightSib->rids[i];}
        cur->n += rightSib->n; cur->next = rightSib->next;
        for(int i = parIdx+1; i < parent->n; i++){parent->keys[i-1] = parent->keys[i]; parent->children[i] = parent->children[i+1];}
        parent->n--;
        rightSib->n = 0; delete rightSib;
    }

    if(parent == root && parent->n == 0){
        root = parent->children[0];
        parent->n = 0;
        for(int i = 0; i <= (int)(2*parent->t); i++) parent->children[i] = nullptr;
        delete parent;
    }
    return true;
}

void BPlusTree::traverse(){
    if(!root) return;
    BPlusNode *cur = root;
    while(!cur->leaf) cur = cur->children[0];
    while(cur){
        for(int i = 0; i < cur->n; i++) cout << cur->keys[i] << "(" << cur->rids[i] << ") ";
        cur = cur->next;
    }
    cout << endl;
}

double BPlusTree::nodeUtilization(){
    if(!root) return 0.0;
    int totalKeys = 0, totalNodes = 0, maxKeys = 2*t - 1;
    struct V{
        int &tk, &tn;
        void visit(BPlusNode *node){
            if(!node) return;
            tk += node->n; tn++;
            if(!node->leaf)
                for(int i = 0; i <= node->n; i++) visit(node->children[i]);
        }
    } v{totalKeys, totalNodes};
    v.visit(root);
    return (double)totalKeys / (totalNodes * maxKeys);
}
