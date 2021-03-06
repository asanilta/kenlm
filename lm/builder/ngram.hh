#ifndef LM_BUILDER_NGRAM__
#define LM_BUILDER_NGRAM__

#include "lm/weights.hh"
#include "lm/word_index.hh"

#include <cstddef>

#include <assert.h>
#include <stdint.h>
#include <string.h>

namespace lm {
namespace builder {

struct Uninterpolated {
  float prob;  // Uninterpolated probability.
  float gamma; // Interpolation weight for lower order.
};

union Payload {
  uint64_t count;
  Uninterpolated uninterp;
  ProbBackoff complete;
};

class NGram {
  public:
    NGram(void *begin, std::size_t order) 
      : begin_(static_cast<WordIndex*>(begin)), end_(begin_ + order) {}

    const uint8_t *Base() const { return reinterpret_cast<const uint8_t*>(begin_); }
    uint8_t *Base() { return reinterpret_cast<uint8_t*>(begin_); }

    void ReBase(void *to) {
      std::size_t difference = end_ - begin_;
      begin_ = reinterpret_cast<WordIndex*>(to);
      end_ = begin_ + difference;
    }

    // Would do operator++ but that can get confusing for a stream.  
    void NextInMemory() {
      ReBase(&Value() + 1);
    }

    // Lower-case in deference to STL.  
    const WordIndex *begin() const { return begin_; }
    WordIndex *begin() { return begin_; }
    const WordIndex *end() const { return end_; }
    WordIndex *end() { return end_; }

    const Payload &Value() const { return *reinterpret_cast<const Payload *>(end_); }
    Payload &Value() { return *reinterpret_cast<Payload *>(end_); }

    uint64_t &Count() { return Value().count; }
    const uint64_t Count() const { return Value().count; }

    std::size_t Order() const { return end_ - begin_; }

    static std::size_t TotalSize(std::size_t order) {
      return order * sizeof(WordIndex) + sizeof(Payload);
    }
    std::size_t TotalSize() const {
      // Compiler should optimize this.  
      return TotalSize(Order());
    }
    static std::size_t OrderFromSize(std::size_t size) {
      std::size_t ret = (size - sizeof(Payload)) / sizeof(WordIndex);
      assert(size == TotalSize(ret));
      return ret;
    }

  private:
    WordIndex *begin_, *end_;
};

const WordIndex kUNK = 0;
const WordIndex kBOS = 1;
const WordIndex kEOS = 2;

} // namespace builder
} // namespace lm

#endif // LM_BUILDER_NGRAM__
