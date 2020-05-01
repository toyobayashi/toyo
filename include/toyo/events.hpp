#ifndef __TOYO_EVENTS_HPP__
#define __TOYO_EVENTS_HPP__

#include <cstddef>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <iostream>
#include <utility>

namespace toyo {

namespace events {

class event_emitter {
 private:
  class listener_base {
   private:
    unsigned int id_;
   public:
    virtual ~listener_base() {}
    listener_base(unsigned int id): id_(id) {}
    unsigned int get_id() const {
      return this->id_;
    }
  };

  template <typename... ArgTypes>
  class listener : public listener_base {
   private:
    bool once_;

    std::function<void (ArgTypes...)> fn_;
   public:
    listener(const std::function<void (ArgTypes...)>& fn, unsigned int id, bool once = false): listener_base(id), once_(once), fn_(fn) {}
    void invoke(ArgTypes... args) const {
      fn_(args...);
    }
    bool is_once() const {
      return once_;
    }
  };

  size_t max_listeners_;
  unsigned int id_;
  std::map<std::string, std::vector<listener_base*>> events_;

  template <typename... ArgTypes, typename Callable>
  event_emitter& _add_listener(
    const std::string& event_name,
    const Callable& l,
    unsigned int* id = nullptr,
    bool prepend = false,
    bool once = false
  ) {
    this->emit("newListener", event_name, id_);

    listener<ArgTypes...>* listener_pointer = new listener<ArgTypes...>(l, id_, once);
    id_++;

    if (this->events_.find(event_name) != this->events_.end()) {
      auto& listeners = this->events_.at(event_name);
      if (prepend) {
        listeners.insert(listeners.begin(), listener_pointer);
      } else {
        listeners.push_back(listener_pointer);
      }
    } else {
      this->events_[event_name] = { listener_pointer };
    }

    if (id != nullptr) {
      *id = id_ - 1;
    }

    size_t max = this->get_max_listeners();
    if (max > 0 && this->listener_count(event_name) > max) {
      std::cerr << "Possible EventEmitter memory leak detected." << std::endl
        << listener_count(event_name) << " " << event_name.c_str() << " listeners." << std::endl
        << "Use toyo::event_emitter::set_max_listeners() to increase limit" << std::endl;
    }

    return *this;
  }
 public:
  virtual ~event_emitter() {
    this->remove_all_listeners();
  }

  event_emitter(): max_listeners_(10), id_(0), events_(std::map<std::string, std::vector<listener_base*>>()) {}

  event_emitter(const event_emitter&) = delete;
  event_emitter& operator=(const event_emitter&) = delete;

  event_emitter(event_emitter&& other): max_listeners_(other.max_listeners_), id_(other.id_), events_(std::move(other.events_)) {
    other.max_listeners_ = 10;
    other.id_ = 0;
  }

  event_emitter& operator=(event_emitter&& other) {
    max_listeners_ = other.max_listeners_;
    other.max_listeners_ = 10;

    id_ = other.id_;
    other.id_ = 0;

    this->remove_all_listeners();
    events_ = std::move(other.events_);
    return *this;
  }

  template <typename... ArgTypes, typename Callable>
  event_emitter& add_listener(const std::string& event_name, const Callable& l, unsigned int* id = nullptr) {
    return this->_add_listener<ArgTypes...>(event_name, l, id, false);
  }

  template <typename... ArgTypes, typename Callable>
  event_emitter& on(const std::string& event_name, const Callable& l, unsigned int* id = nullptr) {
    return this->_add_listener<ArgTypes...>(event_name, l, id, false);
  }

  template <typename... ArgTypes, typename Callable>
  event_emitter& once(const std::string& event_name, const Callable& l, unsigned int* id = nullptr) {
    return this->_add_listener<ArgTypes...>(event_name, l, id, false, true);
  }

  template <typename... ArgTypes, typename Callable>
  event_emitter& prepend_listener(const std::string& event_name, const Callable& l, unsigned int* id = nullptr) {
    return this->_add_listener<ArgTypes...>(event_name, l, id, true);
  }

  template <typename... ArgTypes, typename Callable>
  event_emitter& prepend_once_listener(const std::string& event_name, const Callable& l, unsigned int* id = nullptr) {
    return this->_add_listener<ArgTypes...>(event_name, l, id, true, true);
  }

  bool emit(const std::string& event_name) {
    if (this->events_.find(event_name) != this->events_.end()) {
      std::vector<listener_base*>& listeners = this->events_.at(event_name);
      for (size_t i = 0; i < listeners.size(); i++) {
        const auto& l = listeners[i];
        try {
          listener<>* lp = static_cast<listener<>*>(l);
          lp->invoke();
          if (lp->is_once()) {
            listeners.erase(listeners.begin() + i);
            i--;
          }
        } catch (const std::exception& err) {
          this->emit<const std::exception&>("error", err);
        }
      }
      return true;
    } else if (event_name == "error") {
      throw std::runtime_error("Unhandled error.");
    }
    return false;
  }

  template <typename DataType>
  bool emit(const std::string& event_name, DataType data) {
    if (this->events_.find(event_name) != this->events_.end()) {
      std::vector<listener_base*>& listeners = this->events_.at(event_name);
      for (size_t i = 0; i < listeners.size(); i++) {
        const auto& l = listeners[i];
        try {
          listener<DataType>* lp = static_cast<listener<DataType>*>(l);
          lp->invoke(data);
          if (lp->is_once()) {
            listeners.erase(listeners.begin() + i);
            i--;
          }
        } catch (const std::exception& err) {
          this->emit<const std::exception&>("error", err);
        }
      }
      return true;
    } else if (event_name == "error") {
      throw data;
    }
    return false;
  }

  template <typename... ArgTypes>
  bool emit(const std::string& event_name, ArgTypes... data) {
    if (this->events_.find(event_name) != this->events_.end()) {
      std::vector<listener_base*>& listeners = this->events_.at(event_name);
      for (size_t i = 0; i < listeners.size(); i++) {
        const auto& l = listeners[i];
        try {
          listener<ArgTypes...>* lp = static_cast<listener<ArgTypes...>*>(l);
          lp->invoke(data...);
          if (lp->is_once()) {
            listeners.erase(listeners.begin() + i);
            i--;
          }
        } catch (const std::exception& err) {
          this->emit<const std::exception&>("error", err);
        }
      }
      return true;
    } else if (event_name == "error") {
      throw std::runtime_error("Unhandled error.");
    }
    return false;
  }

  size_t get_max_listeners() const {
    return this->max_listeners_;
  }

  void set_max_listeners(size_t n) {
    this->max_listeners_ = n;
  }

  size_t listener_count (const std::string& event_name) const {
    if (this->events_.find(event_name) != this->events_.end()) {
      return this->events_.at(event_name).size();
    } else {
      return 0;
    }
  }

  std::vector<std::string> event_names() const {
    std::vector<std::string> res;
    for (const auto& p : this->events_) {
      res.push_back(p.first);
    }
    return res;
  }

  event_emitter& remove_all_listeners(const std::string& event_name) {
    if (this->events_.find(event_name) != this->events_.end()) {
      auto& listeners = this->events_.at(event_name);
      for (size_t i = 0; i < listeners.size(); i++) {
        delete listeners[i];
      }
      listeners.clear();
    }
    return *this;
  }

  event_emitter& remove_all_listeners() {
    for (const auto& p : events_) {
      remove_all_listeners(p.first);
    }
    events_.clear();
    return *this;
  }

  event_emitter& remove_listener(const std::string& event_name, unsigned int id) {
    if (this->events_.find(event_name) != this->events_.end()) {
      auto& listeners = this->events_.at(event_name);
      for (size_t i = 0; i < listeners.size(); i++) {
        if (listeners[i]->get_id() == id) {
          delete listeners[i];
          listeners.erase(listeners.begin() + i);
          break;
        }
      }
    }
    return *this;
  }

  event_emitter& off(const std::string& event_name, unsigned int id) {
    return this->remove_listener(event_name, id);
  }
};

} // events

} // toyo

#endif
