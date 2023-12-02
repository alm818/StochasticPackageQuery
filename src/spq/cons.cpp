#include <fmt/core.h>

#include <boost/variant.hpp>
#include "util/udebug.hpp"
#include "cons.hpp"

BoundConstraint::BoundConstraint(const Bound& lb, const Bound& ub) : lb(lb), ub(ub) {
}

bool BoundConstraint::isViolate(const vector<double>& info) const{
    if (!info.size()) return false;
    if (lb.which() == 1 && info.front() < boost::get<double>(lb)) return true;
    if (ub.which() == 1 && info.front() > boost::get<double>(ub)) return true;
    return false;
}

ProbConstraint::ProbConstraint(const Bound& v, const Bound& p, const string& vsign, const string& psign) : v(v), p(p) {
    this->vsign = toInequality[vsign];
    this->psign = toInequality[psign];
}

AttrConstraint::AttrConstraint(const string& attr): attr(attr){
}

CountConstraint::CountConstraint(const Bound& lb, const Bound& ub): BoundConstraint(lb, ub){
}

string CountConstraint::toStr(const vector<double>& info) const{
    if (!info.size()) return fmt::format("COUNT(*) {}", str({lb, ub}));
    auto color = isViolate(info) ? RED : GREEN;
    return fmt::format("COUNT(*)={}{}{} {}", color, info.front(), RESET, str({lb, ub}));
}

SumConstraint::SumConstraint(const string& attr, const Bound& lb, const Bound& ub): AttrConstraint(attr), BoundConstraint(lb, ub){
    attrType = Column::numeric_type;
}

string SumConstraint::toStr(const vector<double>& info) const{
    if (!info.size()) return fmt::format("SUM({}) {}", attr, str({lb, ub}));
    auto color = isViolate(info) ? RED : GREEN;
    return fmt::format("SUM({})={}{}{} {}", attr, color, info.front(), RESET, str({lb, ub}));
}

ExpectedSumConstraint::ExpectedSumConstraint(const string& attr, const Bound& lb, const Bound& ub): AttrConstraint(attr), BoundConstraint(lb, ub){
    attrType = Column::array_type;
}

string ExpectedSumConstraint::toStr(const vector<double>& info) const{
    if (!info.size()) return fmt::format("EXPECTED SUM({}) {}", attr, str({lb, ub}));
    auto color = isViolate(info) ? RED : GREEN;
    return fmt::format("EXPECTED SUM({})={}{}{} {}", attr, color, info.front(), RESET, str({lb, ub}));
}

VarConstraint::VarConstraint(const string& attr, const Bound& v, const Bound& p, const string& vsign, const string& psign): AttrConstraint(attr), ProbConstraint(v, p, vsign, psign){
    attrType = Column::array_type;
}

bool VarConstraint::isViolate(const vector<double>& info) const{
    if (!info.size()) return false;
    if (p.which() == 1){
        if (psign == Inequality::gteq && info.front() < boost::get<double>(p)) return true;
        if (psign == Inequality::lteq && info.front() > boost::get<double>(p)) return true;
    }
    return false;
}

string VarConstraint::toStr(const vector<double>& info) const{
    if (!info.size()) return fmt::format("SUM({}) {} {} WITH PROBABILITY {} {}", attr, str(vsign), str(v), str(psign), str(p));
    auto color = isViolate(info) ? RED : GREEN;
    return fmt::format("SUM({}) {} {} WITH PROBABILITY={}{}{} {} {}", attr, str(vsign), str(v), color, info.front(), RESET, str(psign), str(p));
}