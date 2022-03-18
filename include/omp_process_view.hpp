/***
 *  $Id$
 **
 *  File: omp_process_view.hpp
 *  Created: Sep 20, 2019
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2019 Jaroslaw Zola
 *  Distributed under the MIT License.
 *  See accompanying file LICENSE.
 */

#ifndef omp_process_view_HPP
#define omp_process_view_HPP

#include <atomic>
#include <cstddef>
#include <iterator>
#include <vector>


//TODO: value of the key is not required, may be remove mapped_type 'typename T'
template <typename Task, typename Hash, template <typename A> class Alloc = std::allocator>
class omp_process_view {
    
public:
    using task_type = Task;
    // using mapped_type = T;

    // using value_type = std::pair<key_type, mapped_type>;
    using task_table = std::vector<task_type, Alloc<task_type>>;

    using size_type = std::size_t;

    // minimal iterator
    template <typename Base, bool Const = false>
    struct iterator {
        using iterator_category = std::forward_iterator_tag;

        //using value_type = typename Base::value_type;

        using reference = typename std::conditional_t<Const, const task_type&, task_type&>;
        using pointer = typename std::conditional_t<Const, const task_type*, task_type*>;

        explicit iterator(Base* aptr = nullptr, int b = -1, int p = 0, bool init = false)
            : ptr(aptr), bucket(b), pos(p) {
            if (init == true) {
                bool flag = false;
                for (int i = 0; i < ptr->B_; ++i) {
                    if (ptr->S_[i].empty() == false && ptr->M_[i]==true) {
                        flag = true;
                        bucket = i;
                        break;
                    }
                    // if (ptr->S_[i].empty() == false) {
                    //     flag = true;
                    //     bucket = i;
                    //     break;
                    // }
                } // for i
                if(flag==false){
                    bucket = -1;
                }
            } // if
        } // iterator

        bool operator==(const iterator& rhs) const { return ((bucket == rhs.bucket) && (pos == rhs.pos)); }

        bool operator!=(const iterator& rhs) const { return ((bucket != rhs.bucket) || (pos != rhs.pos)); }

        reference& operator*() { return ptr->S_[bucket][pos]; }

        pointer operator->() { return &(ptr->S_[bucket][pos]); }

        iterator& operator++() {
            pos++;
            if (pos == ptr->S_[bucket].size()) {
                pos = 0;
                do { bucket++; } while ((bucket < ptr->B_) && (ptr->M_[bucket]==false));
                if (bucket == ptr->B_) bucket = -1;
            }
            return *this;
        } // ++operator


        iterator operator++(int) {
            iterator it = *this;
            ++(*this);
            return it;
        } // operator++

        Base* ptr = nullptr;

        int bucket = -1;
        int pos = 0;
    }; // iterator

    // init is like clear, except that is prepares
    // the table to work with B buckets
    void init(int B) {
        B_ = B;
        if (S_.size() < B_) S_.resize(B_);
        if (M_.size() < B_) M_.resize(B_);
        lazy_clear();
    } // init

    // should bring table into a consistent state
    // e.g., after it has been updated via merge
    void update(size_type s, int b) {
        size_ += s;
        last_b_ = std::max(last_b_, b);
    } // update


    // lazy clear will not release memory at all, just marks M_ state to false --> means table is empty
    // If we need to insert data into a bucket,we check if it's old and clear it during the insert and merge operations
    void lazy_clear(){
        std::fill(std::begin(M_), std::begin(M_) + B_, false);
        size_ = 0;
        last_b_ = -1;
    }

    // soft clear does not release allocated memory
    // but marks table for reuse
    void soft_clear() {
        for (auto& s : S_) task_table().swap(s);
        std::fill(std::begin(M_), std::begin(M_) + B_, false);
        size_ = 0;
        last_b_ = -1;
    } // soft_clear

    // we go nuclear here (release all memory)
    void release() {
        B_ = 0;
        std::vector<task_table, Alloc<task_table>>().swap(S_);
        std::vector<char>().swap(M_);
        soft_clear();
    } // release

    // returns true if table has been initialized
    // using the table without init() is UB
    bool ready() const { return (B_ != 0); }


    iterator<omp_process_view> begin() { return iterator<omp_process_view>(this, 0, 0, true); }

    iterator<omp_process_view> end() { return iterator<omp_process_view>(); }

    iterator<const omp_process_view, true> begin() const { return iterator<const omp_process_view, true>(this, 0, 0, true); }

    iterator<const omp_process_view, true> end() const { return iterator<const omp_process_view, true>(); }

    void reserve(int n) {
        S_.resize(n);
        M_.resize(n);
    } // reserve

    bool empty() const { return (size_ == 0); }

    const size_type num_tasks() { return size_; }

    const bool isEmpty(task_table& t){
        if(t.size()>0) return false;
        else return true;
    }

    //Added merge functionality only on key which is the task
    void insert(const task_type& v) {
        Hash h;
        auto hash = h(v);

        int b = hash % B_;
        
        task_table& t = S_[b];

        //means that one is in use, we clear and mark it 
        if(M_[b]==false && !isEmpty(t)){
            t.clear();
        }

        //Outside if because this will work for first usage as well
        M_[b] = true;
        
        last_b_ = std::max(b, last_b_);
        int pos = m_find_pos__(t, v);

        if (pos == -1) {
            t.push_back(v);
            ++size_;
        } else {
            //Calling merge on key(task provided by user)
            //std::cout << "Calling merge during insert " << std::endl;
            //std::cout << "Hash before merge : " << h(t[pos]) << std::endl;
            t[pos].merge(v);
            //std::cout << "Hash  after merge : " << h(t[pos]) << std::endl;
            //std::cout << "**********************************"<< std::endl;
        }
    } // insert

    iterator<omp_process_view> find(const task_type& k) {
        if (size_ == 0) return iterator<omp_process_view>();

        Hash h;
        auto hash = h(k);

        int b = hash % B_;

        //if it is not in use, don't even check and return empty iterator
        if(M_[b]==false){
            return iterator<omp_process_view>();
        }

        const task_table& t = S_[b];
        int pos = m_find_pos__(t, k);

        // here we are lazy instead of implementing proper const_iterator
        return (pos == -1) ? iterator<omp_process_view>() : iterator<omp_process_view>(this, b, pos);
    } // find

    iterator<const omp_process_view, true> find(const task_type& k) const {
        if (size_ == 0) return iterator<const omp_process_view, true>();

        Hash h;
        auto hash = h(k);

        int b = hash % B_;
        
        //if it is not in use, don't even check and return empty iterator
        if(M_[b]==false){
            return iterator<const omp_process_view, true>();
        }
        const task_table& t = S_[b];
        int pos = m_find_pos__(t, k);

        // here we are lazy instead of implementing proper const_iterator
        return (pos == -1) ? iterator<const omp_process_view, true>() : iterator<const omp_process_view, true>(this, b, pos);
    } // find


    const int B() { return B_; }

    const int last_bucket() { return last_b_; }

    const task_table& bucket(int b) { return S_[b]; }


    // this is the only method that may run in parallel
    // provided that different buckets b are involved
    // the state of the table has to be reconciled
    // via call to update
    int merge_by_bucket(const omp_process_view& S, int b) {
        task_table& mainTable = S_[b];
        const task_table& mergeTable = S.S_[b];
        const char mergeTableFlag = S.M_[b];

        int sz = 0;

        if(mergeTableFlag==false){
            //This means the other table has no values
            //don't merge, just return
            return sz;
        }

        if(M_[b]==false  && !isEmpty(mainTable)){
            mainTable.clear();
        }

        M_[b] = true;

        for (auto& entry : mergeTable) {

            //std::cout << "Trying to merge " << entry.value << " at bucket " << b << std::endl;

            int pos = m_find_pos__(mainTable, entry);

            if (pos == -1) {
                mainTable.push_back(entry);
                ++sz;
                //size_++;
            } 
            //We just compare tasks here. == operator for the task should be defined by the user
            else if (mainTable[pos]==entry){
                // merge should be defined by the user
                // std::cout << "Calling merge during merge_by_bucket " << std::endl;
                mainTable[pos].merge(entry);
            };
        } // for entry

        last_b_ = std::max(b, last_b_);
        //std::cout << "Added "<<sz<< " to 0th view" << std::endl;

        return sz;
    } // merge_by_bucket

    const int get_last_used_bucket()
    {
        return last_b_;
    }

// private:
    int m_find_pos__(const task_table& t, const task_type& k) const {
        int sz = t.size();
        for (int i = 0; i < sz; ++i) if (t[i] == k) return i;
        return -1;
    } // m_find_pos__

    // the actual hash table
    std::vector<task_table, Alloc<task_table>> S_;

    // state indicator for bucket: if M_[b] == false
    // bucket b is not used (cleaned)
    std::vector<char> M_;

    int B_ = 0;

    std::size_t size_ = 0;
    int last_b_ = -1;

}; // class omp_process_view

#endif // omp_process_view_HPP
