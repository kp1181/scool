#ifndef simple_task_hpp
#define simple_task_hpp

#include <iostream>
#include <numeric>
#include <vector>


class simple_task {
public:
    int value = 0;      // value size

    explicit simple_task() {    } 

    void merge(const simple_task& t){
        //std::cout << "Merging " <<t.value <<" into "<<value <<std::endl;
        value = t.value;
    }  // no merging will be needed
}; // class simple_task

bool operator==(const simple_task& t1, const simple_task& t2) {
    if (t1.value != t2.value) return false;
    else return true;
} // operator==


// Namespace: C++ Standard Namespace
namespace std {

  template <> struct hash<simple_task> {
      std::size_t operator()(const simple_task& t) const noexcept{
          return t.value*2;
      };
  }; // struct hash
} // namespace std

#endif // simple_task_hpp
