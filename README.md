# SCoOL - Scalable Common Optimization Library


## About

Optimization problems can be thought of as processes exploring trees or directed (acyclic) graphs that represent specific search spaces. For a given search space, nodes of the tree (or graph) represent feasible states in that search space. The states in turn encode a partial or a complete solution for which objective function (or its bounding function) should be evaluated. In this formulation, when performing optimization we seek to explore the search space (traverse tree/graph) in the most efficient way to find a complete and optimal final solution. Of course, the exploration process is dynamic in the sense that the tree/graph representing the search space cannot be instantiated *a-priori* but rather has to be discovered as we go.

SCoOL is a simple programming model designed to facilitate and accelerate the search space exploration phase of the optimization processes. The main premise of the model is to allow end-users to focus on the optimization details (e.g., implementing objective function, defining constraints for the search space, describing a direction in which search space should be explored, etc.), and to delegate the exploration of the resulting search space to an efficient runtime system. The runtime system itself is abstracted from the end-user, and is akin to the classic Bulk Synchronous Parallel (BSP) model. As in BSP, the exploration proceeds in super-steps, where each super-step is distributed across many processing elements running in parallel. However, SCoOL introduces two additional concepts. First, it allows end-users to specify and maintain a globally shared state that can be used to keep (in a consistent way) auxiliary information (for example, information about the best discovered solution thus far). Second, it introduces a concept of partitioner. Partitioner can be specified by end-user, and is applied to distribute the search space between processing elements to maximize locality (e.g., the neighboring portions of the search space are handled by the same element reducing potential communication overheads due to data exchange between processing elements).

SCoOL provides several ready-to-use runtime implementations that can be readily used to perform parallel search space exploration on shared memory systems as well as distributed memory clusters.

The SCoOL project has been supported by [NSF](https://www.nsf.gov/) under the award [OAC-1845840](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1845840).

For more details, visit: https://gitlab.com/SCoRe-Group/scool/-/tree/master
