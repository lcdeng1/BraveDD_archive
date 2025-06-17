#include <iostream>
#include <vector>
#include <brave_dd.h>
#include <cassert>

using namespace BRAVE_DD;

void printGraph(Forest *forest, Func func ,std::string name) {
    std::cout << "Printing " << name << std::endl;
    DotMaker dot(forest, name);
    dot.buildGraph(func);
    dot.runDot("pdf");
    std::cout << "Done printing " << name << std::endl;

}

void test_constant_quasi_int_function() {
    std::cout << "Testing quasi constant int function" << std::endl;
    ForestSetting setting = ForestSetting("ev+qbdd", 3);
    setting.setValType(INT);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5);

    printGraph(forest, func, "quasi_int");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    int dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, INT);
    res.getValueTo(&eval,INT);
    assert(dangling_ev == 5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished quasi constant int function" << std::endl;
}
void test_constant_fully_int_function() {
    std::cout << "Testing fully constant int function" << std::endl;
    ForestSetting setting = ForestSetting("ev+fbdd", 3);
    setting.setValType(INT);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5);

    printGraph(forest, func, "fully_int");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    int dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, INT);
    res.getValueTo(&eval,INT);
    assert(dangling_ev == 5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished fully constant int function" << std::endl;
}
void test_infinite_quasi_function() {
    std::cout << "Testing quasi infinite int function" << std::endl;
    ForestSetting setting = ForestSetting("ev+qbdd", 3);
    setting.setValType(INT);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(SpecialValue::POS_INF);

    printGraph(forest, func, "pos_inf_quasi");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);

    SpecialValue eval;
    res.getValueTo(&eval, VOID);
    assert(eval == SpecialValue::POS_INF);

    delete forest;
    std::cout << "Finished quasi infinite int function" << std::endl;
}
void test_infinite_fully_function() {
    std::cout << "Testing fully infinite int function" << std::endl;
    ForestSetting setting = ForestSetting("ev+fbdd", 3);
    setting.setValType(INT);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(SpecialValue::POS_INF);

    printGraph(forest, func, "pos_inf_fully");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    
    SpecialValue eval;
    res.getValueTo(&eval, VOID);
    assert(eval == SpecialValue::POS_INF);

    delete forest;
    std::cout << "Finished fully infinite int function" << std::endl;
}
void test_quasi_int_function() {
    std::cout << "Testing quasi constant int function" << std::endl;
    ForestSetting setting = ForestSetting("ev+qbdd", 3);
    setting.setValType(INT);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5);

    printGraph(forest, func, "quasi_int");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    int dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, INT);
    res.getValueTo(&eval,INT);
    assert(dangling_ev == 5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished quasi constant int function" << std::endl;
}

void test_constant_quasi_long_function() {
    std::cout << "Testing quasi constant int function" << std::endl;
    ForestSetting setting = ForestSetting("ev+qbdd", 3);
    setting.setValType(LONG);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5);

    printGraph(forest, func, "quasi_long");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    long dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, LONG);
    res.getValueTo(&eval,LONG);
    assert(dangling_ev == 5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished quasi constant int function" << std::endl;
}
void test_constant_fully_long_function() {
    std::cout << "Testing fully constant int function" << std::endl;
    ForestSetting setting = ForestSetting("ev+fbdd", 3);
    setting.setValType(LONG);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5);

    printGraph(forest, func, "fully_long");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    long dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, LONG);
    res.getValueTo(&eval, LONG);
    assert(dangling_ev == 5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished fully constant int function" << std::endl;
}

void test_constant_quasi_float_function() {
    std::cout << "Testing quasi constant float function" << std::endl;
    ForestSetting setting = ForestSetting("ev+qbdd", 3);
    setting.setValType(FLOAT);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5.5f);

    printGraph(forest, func, "quasi_float");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    float dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, FLOAT);
    res.getValueTo(&eval,FLOAT);
    assert(dangling_ev == 5.5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished quasi constant float function" << std::endl;
}
void test_constant_fully_float_function() {
    std::cout << "Testing fully constant float function" << std::endl;
    ForestSetting setting = ForestSetting("ev+fbdd", 3);
    setting.setValType(FLOAT);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5.5f);

    printGraph(forest, func, "fully_float");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    float dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, FLOAT);
    res.getValueTo(&eval,FLOAT);
    assert(dangling_ev == 5.5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished fully constant float function" << std::endl;
}

void test_constant_quasi_double_function() {
    std::cout << "Testing quasi constant double function" << std::endl;
    ForestSetting setting = ForestSetting("ev+qbdd", 3);
    setting.setValType(DOUBLE);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5.5);

    printGraph(forest, func, "quasi_double");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    double dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, DOUBLE);
    res.getValueTo(&eval,DOUBLE);

    assert(dangling_ev == 5.5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished quasi constant double function" << std::endl;
}
void test_constant_fully_double_function() {
    std::cout << "Testing fully constant double function" << std::endl;
    ForestSetting setting = ForestSetting("ev+fbdd", 3);
    setting.setValType(DOUBLE);

    Forest* forest = new Forest(setting);
    Func func(forest);
    func.constant(5.5);

    printGraph(forest, func, "fully_double");

    std::vector<bool> assignment = {0,0,0,0};
    Value res = func.evaluate(assignment);
    double dangling_ev, eval;
    func.getEdge().getValue().getValueTo(&dangling_ev, DOUBLE);
    res.getValueTo(&eval,DOUBLE);
    assert(dangling_ev == 5.5);
    assert((eval == dangling_ev));
    delete forest;
    std::cout << "Finished fully constant double function" << std::endl;
}


int main() {
    /* Testing constant function with int value */
    std::cout << "Starting ev+ tests with more detailed debug output" << std::endl;

    test_constant_quasi_int_function();
    std::cout <<std::endl;
    test_constant_fully_int_function();
    std::cout <<std::endl;
    test_infinite_quasi_function();
    std::cout <<std::endl;
    test_infinite_fully_function();
    std::cout <<std::endl;

    // TODO: Ask Lichuan about double - the use case when do we use double ? How do we avoid precision issues?
    // test_constant_quasi_float_function();
    // std::cout <<std::endl;
    // test_constant_fully_float_function();
    // std::cout <<std::endl;
    // test_constant_quasi_double_function();
    // std::cout <<std::endl;
    // test_constant_fully_double_function();
    // std::cout <<std::endl;

    return 0;
} 