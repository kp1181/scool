#ifndef toy_state_hpp
#define toy_state_hpp

// Class: toy_state
// This is a code template demonstrating requirements (i.e., interface specification)
// that a type must satisfy to model the *StateType* concept. The concept has been
// inspired by the Cilk+ reducers <(see e.g., here): https://cilk.mit.edu/docs/OpenCilkLanguageExtensionSpecification.htm#hyper.reduce>,
// with the difference that *StateType* is a commutative monoid.
class toy_state {
public:
    // Constructor: toy_state
    // Any class modeling *StateType* must provide default constructor. The default
    // constructor must be putting the object in a state that is identity of
    // the corresponding monoid. While other constructors may be provided for
    // the end-user's convenience, they are ignored by the runtime system.
    // *toy_state* must be:
    // <DefaultConstructible: https://en.cppreference.com/w/cpp/named_req/DefaultConstructible>,
    // <CopyConstructible: https://en.cppreference.com/w/cpp/named_req/CopyConstructible>,
    // <CopyAssignable: https://en.cppreference.com/w/cpp/named_req/CopyAssignable>.
    toy_state() { }

    // Function: identity
    // Sets the state of the calling object to identity.
    void identity() { }

    // Function: operator+=
    // Implements an associative and commutative operator of the underlying monoid.
    // The operator is used to perform reduction of local views of the *toy_state*,
    // to obtain a consistent global view. Note that commutative property implies
    // that order in which the operator is used by the runtime system is not defined.
    //
    // Parameters:
    // st - toy_state to reduce with, i.e., the expected behavior is *this* = *this* + *st*.
    void operator+=(const toy_state& st);

    // Function: operator==
    // Implements equality comparison.
    // The routine is critical as it enables efficient distribution of the global state.
    void operator==(const toy_state& st) const;

}; // class toy_state


// Function: operator<<
// Implements the state serialization routine. toy_state might be serialized
// by the runtime at different points of execution, e.g., to be staged in
// a persistent storage or to be sent over a network. It is recommended
// that the routine is lightweight, and generates compact representation
// of the object (e.g., without unnecessary decorations).
//
// Parameters:
// os - Output stream to store serialized object.
// st - Object to serialize.
std::ostream& operator<<(std::ostream& os, const toy_state& st);

// Function: operator>>
// Implements the state deserialization routine. The routine must
// complement serialization routine as implemented in <operator<<()>.
//
// Parameters:
// is - Input stream to deserialize from.
// st - Object to deserialize into.
std::istream& operator>>(std::istream& is, toy_state& st);

#endif // toy_state_hpp
