#ifndef PARTITIONER_HPP
#define PARTITIONER_HPP

// Class: Partitioner
// Specification of the Partitioner type.
class Partitioner {
public:
    // Function: operator()
    //
    // Returns:
    //   the index of a logical partition to which a task should be assigned.
    //   If two tasks are assigned to the same logical partition, the runtime
    //   system will use it as a hint to collocate their execution.
    int operator()(const TaskType& t) const;
}; // class Partitioner

#endif // PARTITIONER_HPP
