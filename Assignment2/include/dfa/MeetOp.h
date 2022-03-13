#pragma once // NOLINT(llvm-header-guard)

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <vector>

namespace dfa {

template <typename TDomainElemRepr> //
class MeetOp {
protected:
  using DomainVal_t = std::vector<TDomainElemRepr>;

public:
  virtual DomainVal_t operator()(const DomainVal_t &LHS,
                                 const DomainVal_t &RHS) const = 0;
  virtual DomainVal_t top(const size_t DomainSize) const = 0;
};

/**
 * @brief Intersection Meet Operator
 *
 * @todo(cscd70) Please complete the definition of the intersection meet
 *               operator, and modify the existing definition if necessary.
 */
class Intersect final : public MeetOp<bool> {
public:
  virtual DomainVal_t operator()(const DomainVal_t &LHS,
                                 const DomainVal_t &RHS) const override {

    DomainVal_t tmpDV = DomainVal_t(LHS.size());
    assert(LHS.size() == RHS.size());

    for (size_t i = 0; i < LHS.size(); i++) {
      if (LHS[i] && RHS[i])
        tmpDV[i] = true;
    }
    return tmpDV;
  }
  virtual DomainVal_t top(const size_t DomainSize) const override {

    return DomainVal_t(DomainSize, true);
  }
};

class Union final : public MeetOp<bool> {
public:
  virtual DomainVal_t operator()(const DomainVal_t &LHS,
                                 const DomainVal_t &RHS) const override {
    DomainVal_t tmpDV = DomainVal_t(LHS.size());
    assert(LHS.size() == RHS.size());

    for (size_t i = 0; i < LHS.size(); i++) {
      if (LHS[i] || RHS[i])
        tmpDV[i] = true;
    }
    return tmpDV;
  }

  virtual DomainVal_t top(const size_t DomainSize) const override {
    return DomainVal_t(DomainSize, false);
  }
};

} // namespace dfa
