# SCoOL - Scalable Common Optimization Library

**Authors:**
Zainul Abideen Sayed <zsayed@buffalo.edu>,
Jaroslaw Zola <jaroslaw.zola@hush.com>

[[_TOC_]]

## About

Optimization problems can be thought of as processes exploring trees or directed (acyclic) graphs that represent specific search spaces. For a given search space, nodes of the tree (or graph) represent feasible states in that search space. The states in turn encode a partial or a complete solution for which objective function (or its bounding function) should be evaluated. In this formulation, when performing optimization we seek to explore the search space (traverse tree/graph) in the most efficient way to find a complete and optimal final solution. Of course, the exploration process is dynamic in the sense that the tree/graph representing the search space cannot be instantiated *a-priori* but rather has to be discovered as we go.

SCoOL is a simple programming model designed to facilitate and accelerate the search space exploration phase of the optimization processes. The main premise of the model is to allow end-users to focus on the optimization details (e.g., implementing objective function, defining constraints for the search space, describing a direction in which search space should be explored, etc.), and to delegate the exploration of the resulting search space to an efficient runtime system. The runtime system itself is abstracted from the end-user, and is akin to the classic Bulk Synchronous Parallel (BSP) model. As in BSP, the exploration proceeds in super-steps, where each super-step is distributed across many processing elements running in parallel. However, SCoOL introduces two additional concepts. First, it allows end-users to specify and maintain a globally shared state that can be used to keep (in a consistent way) auxiliary information (for example, information about the best discovered solution thus far). Second, it introduces a concept of partitioner. Partitioner can be specified by end-user, and is applied to distribute the search space between processing elements to maximize locality (e.g., the neighboring portions of the search space are handled by the same element reducing potential communication overheads due to data exchange between processing elements).

SCoOL provides several ready-to-use runtime implementations that can be readily used to perform parallel search space exploration on shared memory systems as well as distributed memory clusters.

The SCoOL project has been supported by [NSF](https://www.nsf.gov/) under the award [OAC-1845840](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1845840).


## User Guide

The core SCoOL functionality has been implemented in C++. In this guide, we assume that you are familiar with basic C++ and have at least some basic understanding of C++ templates. At the same time, please be advised that we hope to provide a Python binding soon.

### Install

SCoOL is a header only library. It means that to start using it, all you need is access to [`include`](include/) directory in the SCoOL root directory. At the same time, we understand that in many applications you may want to use SCoOL to build a standalone project. For such scenarios, we provide `scool-project-init` in [`tools`](tools/) folder.

To create a new project, simply run `scool-project-init` and pass the name of the directory in which the project should be created, and the project name. For example, running

```
tools/scool-project-init . toy
```

will create `toy` project in the current directory. The tool creates a stub from which you can start implementing your [`Task`](https://cse.buffalo.edu/~jzola/scool/#File:task.hpp) and [`State`](https://cse.buffalo.edu/~jzola/scool/index.html#File:state.hpp) types, and also copies the required header files, license information, etc. Please note that inside the newly created project, `include/` folder is called `scool/`.


### Illustrative Example

To give you an example of how writing SCoOL application may look like, we implement a toy problem of exhaustive tree-like search space exploration. Our search space is a tree over the set of all possible permutations of $n$ objects (i.e., leaves of the tree store all possible $n!$ permutations). Of course, in reality, when dealing with such a search space we would first think about bounding function to constrain it. As you will see, adding a bounding function will be a simple addition to the basic exploration code.

#### Creating Project

To create the project, we would normally run `tools/scool-project-init` as explained earlier. However, we already provide the `toy` example in [`examples/toy`](examples/toy), which you can check to see all details we discuss below.


#### Defining Task

The first step is to define what we call [`Task`](https://cse.buffalo.edu/~jzola/scool/#File:task.hpp). Task represents a node in the search space, and it describes computations that you want to perform when the state is discovered. The task may store both local and static data. The SCoOL API describes in detail [requirements that a type must fulfill to model a Task](https://cse.buffalo.edu/~jzola/scool/#File:task.hpp).

In our example, we are going to implement `toy_task` in the `toy_task.hpp` file. The stub already provides a summary of the methods and functions we have to provide.

To start, we want our task to store the problem size. Since the problem size is a global property we will make it a static attribute:

```c++
class toy_task {
public:
    inline static int n = 0; // problem size

}; // class toy_task
```

To represent the actual permutation, we are going to use `std::vector`, where at position $i$ in the vector we store $i$-th element of the permutation:


```c++
class toy_task {
public:
    inline static int n = 0; // problem size

    int level = 0;      // permutation size
    std::vector<int> p; // state
}; // class toy_task
```

Notice that we also keep the size of the permutation in attribute `level`.

The API requirement is that `toy_task` is default-constructible, but it is frequently handy to have an additional constructor to create an initial solution:


```c++
class toy_task {
public:
    inline static int n = 0; // problem size

    int level = 0;      // permutation size
    std::vector<int> p; // state

    explicit toy_task(bool init = false) : level(0), p{} {
        if (init) {
            p.resize(n);
            std::iota(std::begin(p), std::end(p), 0);
        }
    } // toy_task
}; // class toy_task
```
In the construct, we simply initialize ordering of the element from which we are going to create permutations.

Now comes the critical part, that is the method `process`. This method specifies what should happen when the search space state is discovered/processed. The method takes two arguments: one is a reference to the object representing [Context](https://cse.buffalo.edu/~jzola/scool/index.html#File:context.hpp), and the other is a reference to the global shared state (which we are going to define later on). When visiting the state, we usually want to do two things: evaluate objective function and check constraints that would help us to limit the search space, and decide which new states we should visit next. We can express this as follows in the code:

```c++
class toy_task {
public:
    inline static int n = 0; // problem size

    int level = 0;      // permutation size
    std::vector<int> p; // state

    explicit toy_task(bool init = false) : level(0), p{} {
        if (init) {
            p.resize(n);
            std::iota(std::begin(p), std::end(p), 0);
        }
    } // toy_task

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

            for (int i = level; i < n; ++i) {
                std::swap(t.p[level], t.p[i]); // construct another (partial) permutation
                // here we could check if a bounding function holds for t
                // to decide if it should be pushed to explore
                ctx.push(t); // we want to explore t
                std::swap(t.p[level], t.p[i]);
            }
        }
    } // process

}; // class toy_task
```

Here, we interact we the SCoOL runtime system through the `ctx` reference that binds to the appropriate object modeling [Context](https://cse.buffalo.edu/~jzola/scool/index.html#File:context.hpp). The details of this object are not important from the end-user perspective, except that it provides method `push` that allows us to add more tasks to the execution (like in line 31 of our example). These tasks are passed to the runtime, and are guaranteed to execute in the subsequent superstep.

Finally, the last critical element we must define is method `merge`. This method is relevant only if the search space is a graph, and given search space state may be discovered via different paths. In such case, semantically equivalent tasks (i.e., tasks representing the same point in the search space) must be resolved into a single task that should be preserved. In our case, since the search space is a tree, we can leave method this method empty:

```c++
class toy_task {
public:
    inline static int n = 0; // problem size

    int level = 0;      // permutation size
    std::vector<int> p; // state

    explicit toy_task(bool init = false) : level(0), p{} {
        if (init) {
            p.resize(n);
            std::iota(std::begin(p), std::end(p), 0);
        }
    } // toy_task

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

            for (int i = level; i < n; ++i) {
                std::swap(t.p[level], t.p[i]); // construct another (partial) permutation
                // here we could check if a bounding function holds for t
                // to decide if it should be pushed to explore
                ctx.push(t); // we want to explore t
                std::swap(t.p[level], t.p[i]);
            }
        }
    } // process

    void merge(const toy_task& t); // no merging will be needed

}; // class toy_task
```

To finalize our `toy_task` class, we need to implement few additional functions. Because C++ can't provide automatic serialization/deserialization routines for our class, we must provide them by hand (via `operator<<` and `operator>>`). Here we can use simple text or binary serialization, but of course we want to stay light-weight. In addition, we must provide equality operator (`operator==`) and, for some executors, we may be required to provide a hashing function (it is a good practice to always implement hashing function anyway).


#### Defining Shared State

TBD

#### Setting Executor

TBD

#### Adding Partitioner

TBD


### C++ API Specification

The core SCoOL functionality is implemented in C++. You can find an up-to-date and complete SCoOL API specification at [https://cse.buffalo.edu/~jzola/scool/](https://cse.buffalo.edu/~jzola/scool/). The documentation covers both SCoOL concepts and currently available SCoOL models (e.g., [`mpi_executor`](include/mpi_executor.hpp) for distributed memory systems with the Message Passing Interface).

### Python Bindings

We are currently investigating the best way to provide bindings that would expose SCoOL runtime in Python. As soon as we have a working prototype we will share it!


## References

To cite SCoOL, please refer to this repository.
