#ifndef toy_task_hpp
#define toy_task_hpp

#include <iostream>
#include <numeric>
#include <vector>


// Class: toy_task
// This is a code template demonstrating requirements (i.e., interface specification)
// that a type must satisfy to model the *TaskType* concept.
class toy_task {
public:
    inline static int n = 0; // problem size

    int level = 0;      // permutation size
    std::vector<int> p; // state

    // Constructor: toy_task
    // Any *TaskType* model must provide default constructor.
    // While other constructors may be provided for the end-user's convenience,
    // they are ignored by the runtime system. In the essence, *TaskType* must be:
    // <DefaultConstructible: https://en.cppreference.com/w/cpp/named_req/DefaultConstructible>,
    // <CopyConstructible: https://en.cppreference.com/w/cpp/named_req/CopyConstructible>,
    // <CopyAssignable: https://en.cppreference.com/w/cpp/named_req/CopyAssignable>.
    explicit toy_task(bool init = false) : level(0), p{} {
        if (init) {
            p.resize(n);
            std::iota(std::begin(p), std::end(p), 0);
        }
    } // toy_task

    // Function: process
    // Implements the task processing routine. This function is called
    // implicitly by the runtime system. When executing, a task may create
    // new tasks, which can be pushed to the runtime by calling <Context::push()>
    // method from *ctx*. A task can also read/write the global state *st*.
    //
    // Parameters:
    // ctx - Object of *ContextType* model representing the current runtime.
    // st  - Object of *StateType* model maintaining a local view of the global state.
    template <typename ContextType, typename StateType>
    void process(ContextType& ctx, StateType& st) {
        if (level == n) {
            // potential final solution, evaluate
            // ...
        } else {
            // let's explore
            toy_task t;
            t.level = level + 1;
            t.p = p;
            //std::cout << "Current level is " << t.level << std::endl;

            for (int i = level; i < n; ++i) {
                std::swap(t.p[level], t.p[i]); // construct another (partial) permutation
                // here we could check if a bounding function holds for t
                // to decide if it should be pushed to explore
                ctx.push(t); // we want to explore t
                std::swap(t.p[level], t.p[i]);
                //std::cout << "pushing... " <<std::endl;
            }
        }
    } // process

    // Function: merge
    // Implements the task merging routine. In some cases, the same task may be generated
    // via two different execution paths, especially when running in parallel. In such
    // cases, two equal tasks would exist unnecessarily increasing computational work.
    // When such situation is detected, the runtime merges tasks x and y
    // by calling x.merge(y) and discarding task y. The order in which merging is executed
    // is unspecified, hence x.merge(y) and y.merge(x) must be semantically equivalent.
    //
    // Parameters:
    // t - Object of *TaskType* model that should be merged with the calling task.
    void merge(const toy_task& t){
        int a = 1;
    };  // no merging will be needed

}; // class toy_task

// Function: operator==
// Implements equality comparison of tasks. The function should return
// true if and only if two tasks are semantically identical. It is recommended
// that the routine is lightweight.
bool operator==(const toy_task& t1, const toy_task& t2) {
    if (t1.level != t2.level) return false;
    for (int i = 0; i < t1.level; ++i) if (t1.p[i] != t2.p[i]) return false;
    return true;
} // operator==

// Function: operator<<
// Implements the task serialization routine. toy_tasks might be serialized
// by the runtime at different points of execution, e.g., to be staged in
// a persistent storage or to be sent over a network. It is recommended
// that the routine is lightweight, and generates compact representation
// of the object (e.g., without unnecessary decorations).
//
// Parameters:
// os - Output stream to store serialized object.
// t  - Object to serialize.
std::ostream& operator<<(std::ostream& os, const toy_task& t) {
    os << t.level;
    for (int i = 0; i < t.n; ++i) os << " " << t.p[i];
    os << "\n";
    return os;
} // operator<<

// Function: operator>>
// Implements the task deserialization routine. The routine must
// complement serialization routine as implemented in <operator<<()>.
//
// Parameters:
// is - Input stream to deserialize from.
// t  - Object to deserialize into.
std::istream& operator>>(std::istream& is, toy_task& t) {
    t.p.resize(t.n);
    is >> t.level;
    for (int i = 0; i < t.n; ++i) is >> t.p[i];
    return is;
} // operator>>


// Namespace: C++ Standard Namespace
namespace std {
  // Class: hash
  // Any *TaskType* model must provide a custom specialization
  // of the standard C++ <hash: https://en.cppreference.com/w/cpp/utility/hash>.
  // The hashing function is necessary to enable efficient task storage management.
  template <> struct hash<toy_task> {
      // Function: operator()
      // Implements hashing routine according to the standard hash
      // <requirements: https://en.cppreference.com/w/cpp/utility/hash>.
      // It is recommended that the routine is lightweight.
      std::size_t operator()(const toy_task& t) const noexcept{
          return t.level*rand();
      };
  }; // struct hash
} // namespace std

#endif // toy_task_hpp
