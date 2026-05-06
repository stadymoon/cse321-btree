#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <sstream>
#include "student.h"
#include "btree.h"
#include "bplustree.h"
#include "bstartree.h"

using namespace std;
using namespace chrono;

struct Timer{
    time_point<high_resolution_clock> s;
    void start(){s = high_resolution_clock::now();}
    double ms(){
        auto e = high_resolution_clock::now();
        return duration<double, milli>(e - s).count();
    }
};

struct Row{
    string exp, tree, param;
    int d;
    double time_ms, splits, util_pct, extra;
};
vector<Row> gRows;

void addRow(const string &exp, const string &tree, int d, const string &param,
            double time_ms, double splits, double util_pct, double extra = 0.0){
    gRows.push_back({exp, tree, param, d, time_ms, splits, util_pct, extra});
}

void saveCSV(const string &path){
    ofstream f(path);
    f << "experiment,tree,d,param,time_ms,splits,util_pct,extra\n";
    for(auto &r : gRows)
        f << r.exp << "," << r.tree << "," << r.d << "," << r.param << ","
          << fixed << setprecision(4)
          << r.time_ms << "," << r.splits << "," << r.util_pct << "," << r.extra << "\n";
    cout << "Results saved to " << path << "\n";
}

void printHeader(const string &title){
    cout << "\n" << title << "\n" << string(title.size(), '-') << "\n";
}

void expInsert(const vector<Student> &students, int d){
    cout << "\n[Insert] d=" << d << "\n";

    BTree bt(d);
    BPlusTree bpt(d);
    BStarTree bst(d);
    Timer timer;

    bt.resetStats();
    timer.start();
    for(int i = 0; i < (int)students.size(); i++) bt.insert(students[i].id, i);
    double btT = timer.ms();

    bpt.resetStats();
    timer.start();
    for(int i = 0; i < (int)students.size(); i++) bpt.insert(students[i].id, i);
    double bptT = timer.ms();

    bst.resetStats();
    timer.start();
    for(int i = 0; i < (int)students.size(); i++) bst.insert(students[i].id, i);
    double bstT = timer.ms();

    cout << fixed << setprecision(3);
    cout << left << setw(12) << "Tree" << setw(14) << "Time(ms)" << setw(14) << "Splits" << "Util(%)\n";
    cout << string(54, '-') << "\n";
    cout << setw(12) << "B-Tree"  << setw(14) << btT  << setw(14) << bt.getSplitCount()  << bt.nodeUtilization()  * 100 << "\n";
    cout << setw(12) << "B+Tree"  << setw(14) << bptT << setw(14) << bpt.getSplitCount() << bpt.nodeUtilization() * 100 << "\n";
    cout << setw(12) << "B*Tree"  << setw(14) << bstT << setw(14) << bst.getSplitCount() << bst.nodeUtilization() * 100 << "\n";

    addRow("insert", "BTree",     d, "-", btT,  bt.getSplitCount(),  bt.nodeUtilization()  * 100);
    addRow("insert", "BPlusTree", d, "-", bptT, bpt.getSplitCount(), bpt.nodeUtilization() * 100);
    addRow("insert", "BStarTree", d, "-", bstT, bst.getSplitCount(), bst.nodeUtilization() * 100);
}

void expSearch(const vector<Student> &students, int d, int N = 10000){
    cout << "\n[Search] d=" << d << " | sample=" << N << "\n";

    BTree bt(d);
    BPlusTree bpt(d);
    BStarTree bst(d);
    for(int i = 0; i < (int)students.size(); i++){
        bt.insert(students[i].id, i);
        bpt.insert(students[i].id, i);
        bst.insert(students[i].id, i);
    }

    mt19937 rng(42);
    uniform_int_distribution<int> dist(0, students.size()-1);
    vector<int> keys(N);
    for(auto &k : keys) k = students[dist(rng)].id;

    Timer timer;
    volatile int sink = 0;
    timer.start(); for(int k : keys) sink += bt.search(k);  double btT  = timer.ms();
    timer.start(); for(int k : keys) sink += bpt.search(k); double bptT = timer.ms();
    timer.start(); for(int k : keys) sink += bst.search(k); double bstT = timer.ms();
    (void)sink;

    cout << fixed << setprecision(6);
    cout << left << setw(12) << "Tree" << setw(18) << "Total(ms)" << "Avg(us)\n";
    cout << string(40, '-') << "\n";
    cout << setw(12) << "B-Tree"  << setw(18) << btT  << btT  * 1000 / N << "\n";
    cout << setw(12) << "B+Tree"  << setw(18) << bptT << bptT * 1000 / N << "\n";
    cout << setw(12) << "B*Tree"  << setw(18) << bstT << bstT * 1000 / N << "\n";

    addRow("search", "BTree",     d, "-", btT,  0, 0, btT  * 1000 / N);
    addRow("search", "BPlusTree", d, "-", bptT, 0, 0, bptT * 1000 / N);
    addRow("search", "BStarTree", d, "-", bstT, 0, 0, bstT * 1000 / N);
}

// Query: avg GPA and height of Male students, ID in [202000000, 202100000]
void expRange(const vector<Student> &students, int d){
    cout << "\n[Range Query] d=" << d << "\n";

    const int LO = 202000000;
    const int HI = 202100000;

    BPlusTree bpt(d);
    for(int i = 0; i < (int)students.size(); i++) bpt.insert(students[i].id, i);

    Timer timer;
    timer.start();
    vector<int> rids = bpt.rangeQuery(LO, HI);
    double bptT = timer.ms();

    int bptMale = 0; double bptGPA = 0, bptH = 0;
    for(int rid : rids){
        if(students[rid].gender == "Male"){
            bptMale++;
            bptGPA += students[rid].gpa;
            bptH   += students[rid].height;
        }
    }
    double bptAvgGPA = bptMale ? bptGPA / bptMale : 0.0;
    double bptAvgH   = bptMale ? bptH   / bptMale : 0.0;

    BTree bt(d);
    for(int i = 0; i < (int)students.size(); i++) bt.insert(students[i].id, i);

    timer.start();
    int btMale = 0; double btGPA = 0, btH = 0;
    for(int i = 0; i < (int)students.size(); i++){
        if(students[i].id >= LO && students[i].id <= HI && students[i].gender == "Male"){
            int rid = bt.search(students[i].id);
            if(rid != -1){ btMale++; btGPA += students[rid].gpa; btH += students[rid].height; }
        }
    }
    double btT = timer.ms();
    double btAvgGPA = btMale ? btGPA / btMale : 0.0;
    double btAvgH   = btMale ? btH   / btMale : 0.0;

    BStarTree bst(d);
    for(int i = 0; i < (int)students.size(); i++) bst.insert(students[i].id, i);

    timer.start();
    int bstMale = 0; double bstGPA = 0, bstH = 0;
    for(int i = 0; i < (int)students.size(); i++){
        if(students[i].id >= LO && students[i].id <= HI && students[i].gender == "Male"){
            int rid = bst.search(students[i].id);
            if(rid != -1){ bstMale++; bstGPA += students[rid].gpa; bstH += students[rid].height; }
        }
    }
    double bstT = timer.ms();
    double bstAvgGPA = bstMale ? bstGPA / bstMale : 0.0;
    double bstAvgH   = bstMale ? bstH   / bstMale : 0.0;

    cout << fixed << setprecision(4);
    cout << "Query    : Male students, ID in [" << LO << ", " << HI << "]\n";
    cout << left << setw(12) << "Tree" << setw(12) << "Time(ms)"
         << setw(8) << "Males" << setw(12) << "Avg GPA" << "Avg Height\n";
    cout << string(56, '-') << "\n";
    cout << setw(12) << "B-Tree"  << setw(12) << btT  << setw(8) << btMale  << setw(12) << btAvgGPA  << btAvgH  << "\n";
    cout << setw(12) << "B+Tree"  << setw(12) << bptT << setw(8) << bptMale << setw(12) << bptAvgGPA << bptAvgH << "\n";
    cout << setw(12) << "B*Tree"  << setw(12) << bstT << setw(8) << bstMale << setw(12) << bstAvgGPA << bstAvgH << "\n";

    addRow("range", "BTree",     d, "Male_ID_range", btT,  0, 0, btAvgGPA);
    addRow("range", "BPlusTree", d, "Male_ID_range", bptT, 0, 0, bptAvgGPA);
    addRow("range", "BStarTree", d, "Male_ID_range", bstT, 0, 0, bstAvgGPA);
}

void expDelete(const vector<Student> &students, int d, double ratio){
    int delCount = (int)(students.size() * ratio);
    cout << "\n[Delete " << (int)(ratio*100) << "%] d=" << d << "\n";

    BTree bt(d);
    BPlusTree bpt(d);
    BStarTree bst(d);
    for(int i = 0; i < (int)students.size(); i++){
        bt.insert(students[i].id, i);
        bpt.insert(students[i].id, i);
        bst.insert(students[i].id, i);
    }

    mt19937 rng(123);
    vector<int> idx(students.size());
    iota(idx.begin(), idx.end(), 0);
    shuffle(idx.begin(), idx.end(), rng);
    vector<int> delKeys(delCount);
    for(int i = 0; i < delCount; i++) delKeys[i] = students[idx[i]].id;

    Timer timer;
    timer.start(); for(int k : delKeys) bt.remove(k);  double btT  = timer.ms();
    timer.start(); for(int k : delKeys) bpt.remove(k); double bptT = timer.ms();
    timer.start(); for(int k : delKeys) bst.remove(k); double bstT = timer.ms();

    cout << fixed << setprecision(3);
    cout << left << setw(12) << "Tree" << setw(16) << "Time(ms)" << "Util(%)\n";
    cout << string(36, '-') << "\n";
    cout << setw(12) << "B-Tree"  << setw(16) << btT  << bt.nodeUtilization()  * 100 << "\n";
    cout << setw(12) << "B+Tree"  << setw(16) << bptT << bpt.nodeUtilization() * 100 << "\n";
    cout << setw(12) << "B*Tree"  << setw(16) << bstT << bst.nodeUtilization() * 100 << "\n";

    string p = to_string((int)(ratio*100)) + "pct";
    addRow("delete", "BTree",     d, p, btT,  0, bt.nodeUtilization()  * 100);
    addRow("delete", "BPlusTree", d, p, bptT, 0, bpt.nodeUtilization() * 100);
    addRow("delete", "BStarTree", d, p, bstT, 0, bst.nodeUtilization() * 100);
}

int main(int argc, char *argv[]){
    string csvPath = "student.csv";
    if(argc > 1) csvPath = argv[1];

    cout << "Loading: " << csvPath << " ...\n";
    vector<Student> students = loadCSV(csvPath);
    if(students.empty()){ cerr << "No data.\n"; return 1; }
    cout << "Loaded " << students.size() << " students.\n";

    vector<int> orders = {3, 5, 10};

    printHeader("Experiment 1: Insert");
    for(int d : orders) expInsert(students, d);

    printHeader("Experiment 2: Search");
    for(int d : orders) expSearch(students, d);

    printHeader("Experiment 3: Range Query");
    for(int d : orders) expRange(students, d);

    printHeader("Experiment 4: Delete 10%");
    for(int d : orders) expDelete(students, d, 0.1);

    printHeader("Experiment 4b: Delete 20%");
    for(int d : orders) expDelete(students, d, 0.2);

    saveCSV("results.csv");
    return 0;
}
