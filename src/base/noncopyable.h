#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace symlog {

class noncopyable {
   public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

   protected:
    noncopyable() = default;
    ~noncopyable() = default;
};
}  // namespace symlog

#endif