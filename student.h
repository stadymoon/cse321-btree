#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

struct Student{
    int id;
    std::string name;
    std::string gender;
    double gpa;
    double height;
    double weight;
};

inline std::vector<Student> loadCSV(const std::string &path){
    std::vector<Student> v;
    std::ifstream f(path);
    if(!f){std::cerr << "Cannot open " << path << std::endl; return v;}
    std::string line;
    std::getline(f, line);
    while(std::getline(f, line)){
        if(line.empty()) continue;
        if(!line.empty() && line.back() == '\r') line.pop_back();
        std::stringstream ss(line);
        Student s;
        std::string tok;
        std::getline(ss, tok, ','); s.id     = std::stoi(tok);
        std::getline(ss, tok, ','); s.name   = tok;
        std::getline(ss, tok, ','); s.gender = tok;
        std::getline(ss, tok, ','); s.gpa    = std::stod(tok);
        std::getline(ss, tok, ','); s.height = std::stod(tok);
        std::getline(ss, tok, ','); s.weight = std::stod(tok);
        v.push_back(s);
    }
    return v;
}

#endif
