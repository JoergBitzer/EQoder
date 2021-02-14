/* 
from 
https://stackoverflow.com/questions/27827923/c-object-pool-that-provides-items-as-smart-pointers-that-are-returned-to-pool

//*/
#pragma once

#include <stack>
#include <cassert>

template <class T>
class PointerPool
{
 private:
  struct External_Deleter {
    explicit External_Deleter(std::weak_ptr<PointerPool<T>* > pool)
        : pool_(pool) {}

    void operator()(T* ptr) {
      if (auto pool_ptr = pool_.lock()) {
        try {
          (*pool_ptr.get())->add(std::unique_ptr<T>{ptr});
          return;
        } catch(...) {}
      }
      std::default_delete<T>{}(ptr);
    }
   private:
    std::weak_ptr<PointerPool<T>* > pool_;
  };

 public:
  using ptr_type = std::unique_ptr<T, External_Deleter >;

  PointerPool() : this_ptr_(new PointerPool<T>*(this)) {}
  virtual ~PointerPool(){}

  void add(std::unique_ptr<T> t) {
    pool_.push(std::move(t));
  }

  ptr_type acquire() {
    assert(!pool_.empty());
    ptr_type tmp(pool_.top().release(),
                 External_Deleter{std::weak_ptr<PointerPool<T>*>{this_ptr_}});
    pool_.pop();
    return std::move(tmp);
  }

  bool empty() const {
    return pool_.empty();
  }

  size_t size() const {
    return pool_.size();
  }

 private:
  std::shared_ptr<PointerPool<T>* > this_ptr_;
  std::stack<std::unique_ptr<T> > pool_;
};

