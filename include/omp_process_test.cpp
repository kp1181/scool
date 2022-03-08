#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>
#include <omp.h>


#include "omp_process_table.hpp"
#include "simple_task.hpp"


int main(){
    const int b = 10;
    int p = 0;

    #pragma omp parallel
    {
        p = omp_get_num_threads();
    }

    omp_process_table<simple_task,std::hash<simple_task>,std::allocator> mainTable;
    mainTable.init(b,p);

    simple_task s1;
    s1.value = 6;

    simple_task s2;
    s2.value = 16;

    simple_task s3;
    s3.value = 18;

    simple_task s4;
    s4.value = 19;

    simple_task s5;
    s5.value = 20;

    #pragma omp parallel
    {
        mainTable.insert(s1);
        mainTable.insert(s2);
    }

    mainTable.omp_process_views_[1].insert(s4);

    mainTable.reconcile();

    //Using the iterator from turbo_table
    auto start = mainTable.begin();
    auto end = mainTable.end();

    while(start!=end){

       std::cout << "Value is " << start->value <<std::endl;
       start++;
    }
}

