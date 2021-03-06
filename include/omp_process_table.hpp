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
    using view_type = omp_process_view<task_type, task_hash, Alloc>;
    using task_table = std::vector<task_type, Alloc<task_type>>;

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
        int cur_t = omp_get_thread_num();
        omp_process_views_[cur_t].insert(v);
    }

    void update_table(){
        int p = 1;

        #pragma omp parallel
        #pragma omp single
        {
            p = omp_get_num_threads();
        }

        for(int i = 0; i < p; ++i)
        {
            last_b_ = std::max(last_b_, omp_process_views_[i].get_last_used_bucket());
        }
    }

    void reconcile()
    {
        //Merge all hash tables

        int p = 1;

        #pragma omp parallel
        #pragma omp single
        {
            p = omp_get_num_threads();
        }
        std::vector<int> updated_size(p,0);

        update_table();

        #pragma omp parallel for default(none) shared(updated_size, omp_process_views_,std::cout,B_,n_views_) schedule(static)
        for (int i = 0; i < last_b_; ++i) {
            for (int j = 1; j < n_views_; ++j) {
                int cur_t = omp_get_thread_num();
                int added = omp_process_views_[0].merge_by_bucket(omp_process_views_[j], i);
                updated_size[cur_t]+=added;
            }
        }

        //Update size of 0th view
        long long int new_size = 0;
        for(long long int i = 0; i < p; i++){
           new_size += updated_size[i];
        }

        long long int original_size = omp_process_views_[0].get_size();
        omp_process_views_[0].set_size(original_size + new_size);
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

    const int num_views(){
        return n_views_;
    }

    const bool empty(){
        return omp_process_views_[0].empty();
    }

    const long long int master_view_size(){
        return static_cast<long long int>(omp_process_views_[0].num_tasks());
    }

    using hashtable = std::vector<task_table, Alloc<task_table>>;
    hashtable& get_mastertable_view()
    {
        return (omp_process_views_[0].get_hash_table());
    }

    std::vector<char>& get_mastertable_state()
    {
        return (omp_process_views_[0].get_bucket_state());
    }

    private:
    std::vector<view_type> omp_process_views_;
    int B_ = 0;
    int n_views_ = 0;
    int last_b_ = -1;
    view_type m_view;
    
    int m_curr_thread = -1;

};

#endif // omp_process_view_HPP