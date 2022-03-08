#ifndef omp_process_table_HPP
#define omp_process_tableHPP

#include <atomic>
#include <cstddef>
#include <iterator>
#include <vector>
#include <omp.h>

#include "omp_process_view.hpp"

template <typename Task, typename Hash, template <typename A> class Alloc = std::allocator>
class omp_process_table {
public:

    using task_type = Task;
    using task_hash = std::hash<task_type>;
    using view_type = omp_process_view<task_type, task_hash, std::allocator>;

    typename view_type::iterator<view_type> find(const task_type& k)
    {
        return omp_process_views_[0].find(k);
    }
    
    typename view_type::iterator<view_type> begin()
    {
        return omp_process_views_[0].begin();
    }

    typename view_type::iterator<view_type> end()
    {
        return omp_process_views_[0].end();
    }

    void init(int b, int p)
    {
        n_views_ = p;
        B_ = b;
        omp_process_views_.resize(p);
        for (auto& v : omp_process_views_) v.init(b);
    }

    void insert(const task_type& v) {
        if (m_curr_thread == -1)
            m_curr_thread = omp_get_thread_num();
        omp_process_views_[m_curr_thread].insert(v);
    }

    void reconcile()
    {
        //Merge all hash tables
        #pragma omp parallel for default(none) shared(omp_process_views_,std::cout,B_,n_views_) schedule(static)
        for (int i = 0; i < B_; i++) {
            for (int j = 1; j < n_views_; j++) {
                omp_process_views_[0].merge_by_bucket(omp_process_views_[j], i);
            }
        }
    }

    void release(){
        for (auto& v : omp_process_views_){
            v.release();
        }
    }

    void lazy_clear(){
        for (auto& v : omp_process_views_){
            v.lazy_clear();
        }
    }

    void soft_clear(){
        for (auto& v : omp_process_views_){
            v.soft_clear();
        }
    }

    int num_views(){
        return n_views_;
    }

    bool empty(){
        return omp_process_views_[0].empty();
    }

    int size(){
        return static_cast<int>(omp_process_views_[0].size());
    }


    std::vector<view_type> omp_process_views_;
    int B_ = 0;
    int n_views_ = 0;
    int last_b_ = -1;
    view_type m_view;

    int m_curr_thread = -1;

};

#endif // omp_process_view_HPP