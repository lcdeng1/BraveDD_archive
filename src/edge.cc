#include "edge.h"
#include "terminal.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                                                                *
// *                      Value  methods                        *
// *                                                                *
// *                                                                *
// ******************************************************************

Value::Value()
{
    valueType = INT;
    intValue = 0;
}
Value::Value(int i)
{
    valueType = INT;
    intValue = i;
}
Value::Value(long l)
{
    valueType = LONG;
    longValue = l;
}
Value::Value(double d)
{
    valueType = DOUBLE;
    doubleValue = d;
}
Value::Value(float f)
{
    valueType = FLOAT;
    floatValue = f;
}
Value::Value(SpecialValue sv)
{
    valueType = VOID;
    special = sv;
}
Value::Value(const Value& val)
{
    init(val);
}

void Value::print(std::ostream& out, int format) const
{
    if (format == 0) {
        switch (valueType) {
        case INT:
            out << intValue;
            break;
        case LONG:
            out << longValue;
            break;
        case FLOAT:
            out << floatValue;
            break;
        case DOUBLE:
            out << doubleValue;
            break;
        case VOID:
            out << speciaValue2String(special);
            break;
        default:
            out << "Unknown";
            break;
        }
    } else {
        // more format TBD
    }
}

// ******************************************************************
// *                                                                *
// *                                                                *
// *                         Edge  methods                          *
// *                                                                *
// *                                                                *
// ******************************************************************

Edge::Edge()
{
    handle = 0;
    value = Value(0);
}
Edge::Edge(const Edge &e)
{
    handle = e.handle;
    value = e.value;
}
Edge::Edge(const EdgeHandle h, Value val):value(val)
{
    //
    handle = h;
}
Edge::~Edge()
{

}

Edge Edge::part(bool xy) const {
    Edge ans;
    ReductionRule rule = unpackRule(handle);
    if ((isRuleEL(rule) || isRuleAL(rule) || isRuleEH(rule) || isRuleAH(rule))
        && (xy == (isRuleEH(rule) || isRuleAH(rule)))) {
        ans.setEdgeHandle(makeTerminal(INT, 0));
        ans.setComp(hasRuleTerminalOne(rule));
        ans.setSwap(0, 0);
    } else if (isRuleI(rule)) {
        // not supported, throw error
        std::cout << "[BRAVE_DD] ERROR!\t Edge::part(bool xy): Not supportted for I edge!"<< std::endl;
        exit(0);
    } else {
        ans = *this;
    }
    ans.setRule(RULE_X);
    return ans;
}

bool Edge::isComplementTo(const Edge& e) const {
    if ((getNodeLevel() == e.getNodeLevel()) && (getNodeLevel() == 0)) {
        if ((!isTerminalOne(handle) && !isTerminalZero(handle)) 
            || (!isTerminalOne(e.handle) && !isTerminalZero(e.handle))) {
            std::cout << "[BRAVE_DD] ERROR!\t Edge::isComplementTo(Edge e): Not supportted for terminal value > 1!"<< std::endl;
            print(std::cout);
            std::cout << std::endl;
            e.print(std::cout);
            std::cout << std::endl;
            exit(0);
        }
        return (((isTerminalOne(handle) ^ getComp()) != (isTerminalOne(e.handle) ^ e.getComp()))
                && (this->getRule() == compRule(e.getRule())));
    }
    if ((getNodeLevel() != e.getNodeLevel()) || (getNodeHandle() != e.getNodeHandle())) return false;
    if ((getSwap(0) != e.getSwap(0)) || (getSwap(1) != e.getSwap(1))) return false;
    return (getComp() != e.getComp()) && (getRule() == compRule(e.getRule()));
}

bool Edge::isConstantOne() const {
    if (getNodeLevel() > 0) return false;
    bool isTermOne = isTerminalOne(handle);
    bool isTermZero = isTerminalZero(handle);
    if (!isTermOne && !isTermZero) return false;
    if (getRule() == RULE_X) return getComp() ^ isTermOne;
    return (hasRuleTerminalOne(getRule()) == (getComp() ^ isTermOne)) && (getComp() ^ isTermOne);
}

bool Edge::isConstantZero() const {
    if (getNodeLevel() > 0) return false;
    bool isTermOne = isTerminalOne(handle);
    bool isTermZero = isTerminalZero(handle);
    if (!isTermOne && !isTermZero) return false;
    if (getRule() == RULE_X) return !(getComp() ^ isTermOne);
    return (hasRuleTerminalOne(getRule()) == (getComp() ^ isTermOne)) && !(getComp() ^ isTermOne);
}

bool Edge::isConstantOmega() const {
    if (getNodeLevel() > 0) return false;
    return isTerminalSpecial(SpecialValue::OMEGA, handle);
}

bool Edge::isConstantPosInf() const {
    if (getNodeLevel() > 0) return false;
    return isTerminalSpecial(SpecialValue::POS_INF, handle);
}

bool Edge::isConstantNegInf() const {
    if (getNodeLevel() > 0) return false;
    return isTerminalSpecial(SpecialValue::NEG_INF, handle);
}

bool Edge::isConstantUnDef() const {
    if (getNodeLevel() > 0) return false;
    return isTerminalSpecial(SpecialValue::UNDEF, handle);
}

void Edge::print(std::ostream& out, int format) const
{
    printEdgeHandle(handle, out, format);
    out << " v: ";
    value.print(out, format);
    if (format == 0) {
        //
    } else {
        // more format TBD
    }
}
