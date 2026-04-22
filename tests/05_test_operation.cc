#include "gen_random_functions.h"

bool testUnaryOperation(uint16_t num, const std::string bdd, UnaryOperationType opt)
{
    // forest setting
    ForestSetting setting("sfbdd", num);
    Forest* forest = new Forest(setting);
    bool isRel = setting.isRelation();
    // function
    Func f(forest), res(forest);
    // truth table
    unsigned long size = (isRel) ? 0x01UL<<(2*num) : 0x01UL<<(num);
    std::vector<bool> fun(size);
    unsigned long countOne = 0;
    for (unsigned long i=0; i<size; i++) {
        fun[i] = (random01() > 0.5f)? 1 : 0;
        if (fun[i]) countOne++;
    }
    std::string operation = "COMP";
    // build edge
    Edge e;
    if (isRel) {
        e = buildRelEdge(forest, num, fun, 0, size-1);
    } else {
        e = buildSetEdge(forest, num, fun, 0, size-1);
    }
    std::cout << "Func: " << std::endl;
    e.print(std::cout);
    std::cout << std::endl;
    // call apply and check result
    f.setEdge(e);
    if (opt == UnaryOperationType::UOP_COMPLEMENT) {
        res = !f;
    } else {
        std::cout << "Not implemented!" << std::endl;
    }
    std::cout << "result (" << operation << "): " << std::endl;
    res.getEdge().print(std::cout);
    std::cout << std::endl;
    // evaluation
    bool isPass = 1;
    std::vector<bool> assignment(num+1, 0);
    std::vector<bool> assignmentTo(num+1, 0);
    for (unsigned long n=0; n<size; n++) {
        if (isRel) {
            for (uint16_t l=1; l<=num; l++) {
                assignment[l] = n & (0x01 << (2*l-1));
                assignmentTo[l] = n & (0x01 << (2*l-2));
            }
        } else {
            for (size_t k=1; k<=assignment.size()-1; k++) {
                assignment[k] = n & (1<<(k-1));
            }
        }
        Value val1, valRes;
        val1 = (isRel) ? f.evaluate(assignment, assignmentTo) : f.evaluate(assignment);
        valRes = (isRel) ? res.evaluate(assignment, assignmentTo) : res.evaluate(assignment);
        int valInt;
        val1.getValueTo(&valInt, INT);
        if (fun[n] != valInt) {
            std::cout << "Func evaluation failed!" << std::endl;
            isPass = 0;
        }
        valRes.getValueTo(&valInt, INT);
        int compRes = 0;
        if (opt == UnaryOperationType::UOP_COMPLEMENT) {
            compRes = (int)!fun[n];
        }
        if (compRes != valInt) {
            std::cout << "result (" << operation << ") evaluation failed!" << std::endl;
            isPass = 0;
        }
        if (!isPass && (num <=6)) {
            std::cout << "Assignment: ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << assignment[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "Correct: " << compRes << "; result: " << valInt << std::endl;
            break;
        }
    }
    if (!isPass) {
        if (num < 6) {
            std::cout<<"Full function\n";
            std::cout << "\tfun: \t\t";
            for (unsigned long k=0; k<size; k++){
                std::cout << fun[k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        DotMaker dot1(forest);
        dot1.buildGraph(f, "func");
        dot1.runDot("func", "pdf");
        DotMaker dot3(forest);
        dot3.buildGraph(res, "res");
        dot3.runDot("res", "pdf");
    }
    delete forest;
    return isPass;

}

bool testXOR(uint16_t num, PredefForest bdd)
{
    // forest setting
    // ForestSetting setting(bdd, num);
    ForestSetting setting(bdd, num);
    Forest* forest = new Forest(setting);
    bool isRel = setting.isRelation();
    // functions
    Func f1(forest), f2(forest), res(forest);
    // truth table
    long long size = (isRel) ? 0x01LL<<(2*num) : 0x01LL<<(num);
    std::vector<bool> fun1(size);
    std::vector<bool> fun2(size);
    // countings
    long long countOne1 = 0, countOne2 = 0, countOneRes = 0;
    for (long long i=0; i<size; i++) {
        fun1[i] = (random01() > 0.5f)? 1 : 0;
        fun2[i] = (random01() > 0.5f)? 1 : 0;
        if (fun1[i]) countOne1++;
        if (fun2[i]) countOne2++;
        if (fun1[i] ^ fun2[i]) countOneRes++;
    }
    std::string operation = "XOR";
    std::cout << "Ones counting:" << std::endl;
    std::cout << "Func 1: " << countOne1 << ";" << std::endl;
    std::cout << "Func 2: " << countOne2 << ";" << std::endl;
    std::cout << "res (" << operation << ") should be: " << countOneRes << ";" << std::endl;

    // build edges
    Edge e1, e2;
    if (isRel) {
        e1 = buildRelEdge(forest, num, fun1, 0, size-1);
        e2 = buildRelEdge(forest, num, fun2, 0, size-1);
    } else {
        e1 = buildSetEdge(forest, num, fun1, 0, size-1);
        e2 = buildSetEdge(forest, num, fun2, 0, size-1);
    }
    
    std::cout << "Func 1: " << std::endl;
    e1.print(std::cout);
    std::cout << std::endl;
    std::cout << "Func 2: " << std::endl;
    e2.print(std::cout);
    std::cout << std::endl;

    // call apply, and check results
    f1.setEdge(e1);
    f2.setEdge(e2);
    res = f1 ^ f2;
    std::cout << "result (" << operation << "): " << std::endl;
    res.getEdge().print(std::cout);
    std::cout << std::endl;

    // evaluation
    bool isPass = 1;
    std::vector<bool> assignment(num+1, 0);
    std::vector<bool> assignmentTo(num+1, 0);
    for (long long n=0; n<size; n++) {
        if (isRel) {
            for (uint16_t l=1; l<=num; l++) {
                assignment[l] = n & (0x01 << (2*l-1));
                assignmentTo[l] = n & (0x01 << (2*l-2));
            }
        } else {
            for (size_t k=1; k<=assignment.size()-1; k++) {
                assignment[k] = n & (1<<(k-1));
            }
        }
        Value val1, val2, valRes;
        val1 = (isRel) ? f1.evaluate(assignment, assignmentTo) : f1.evaluate(assignment);
        val2 = (isRel) ? f2.evaluate(assignment, assignmentTo) : f2.evaluate(assignment);
        valRes = (isRel) ? res.evaluate(assignment, assignmentTo) : res.evaluate(assignment);
        int valInt;
        val1.getValueTo(&valInt, INT);
        if (fun1[n] != valInt) {
            std::cout << "Func 1 evaluation failed!" << std::endl;
            isPass = 0;
        }
        val2.getValueTo(&valInt, INT);
        if (fun2[n] != valInt) {
            std::cout << "Func 2 evaluation failed!" << std::endl;
            isPass = 0;
        }
        valRes.getValueTo(&valInt, INT);
        int compRes = fun1[n] ^ fun2[n];
        if (compRes != valInt) {
            std::cout << "result (" << operation << ") evaluation failed!" << std::endl;
            isPass = 0;
        }
        if (!isPass && (num <=6)) {
            std::cout << "Assignment: ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << assignment[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "Correct: " << compRes << "; result: " << valInt << std::endl;
            break;
        }
    }
    if (!isPass) {
        if (num < 6) {
            std::cout<<"Full function\n";
            std::cout << "\tfun1: \t\t";
            for (long long k=0; k<size; k++){
                std::cout << fun1[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "\tfun2: \t\t";
            for (long long k=0; k<size; k++){
                std::cout << fun2[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "\tres should be: \t";
            for (long long k=0; k<size; k++){
                std::cout << (fun1[k] && fun2[k]) << " ";
            }
        }
        std::cout << std::endl;
        DotMaker dot1(forest);
        dot1.buildGraph(f1, "func1");
        dot1.runDot("func1", "pdf");
        DotMaker dot2(forest);
        dot2.buildGraph(f2, "func2");
        dot2.runDot("func2", "pdf");
        DotMaker dot3(forest);
        dot3.buildGraph(res, "res");
        dot3.runDot("res", "pdf");
    }
    delete forest;
    return isPass;
}

bool testBinaryOperation(uint16_t num, PredefForest bdd, BinaryOperationType opt)
{
    // forest setting
    // ForestSetting setting(bdd, num);
    ForestSetting setting("sfbdd", num);
    Forest* forest = new Forest(setting);
    bool isRel = setting.isRelation();
    // functions
    Func f1(forest), f2(forest), res(forest);
    // truth table
    long long size = (isRel) ? 0x01LL<<(2*num) : 0x01LL<<(num);
    std::vector<bool> fun1(size);
    std::vector<bool> fun2(size);
    // countings
    long long countOne1 = 0, countOne2 = 0, countOneRes = 0;
    for (long long i=0; i<size; i++) {
        fun1[i] = (random01() > 0.5f)? 1 : 0;
        fun2[i] = (random01() > 0.5f)? 1 : 0;
        if (fun1[i]) countOne1++;
        if (fun2[i]) countOne2++;
        if (opt == BinaryOperationType::BOP_INTERSECTION) {
            if (fun1[i] && fun2[i]) countOneRes++;
        } else if (opt == BinaryOperationType::BOP_UNION) {
            if (fun1[i] || fun2[i]) countOneRes++;
        } else if (opt == BinaryOperationType::BOP_MINIMUM) {
            //
        } else if (opt == BinaryOperationType::BOP_MAXIMUM) {
            //
        }
    }
    std::string operation = "AND";
    if (opt == BinaryOperationType::BOP_UNION) {
        operation = "OR";
    }
    std::cout << "Ones counting:" << std::endl;
    std::cout << "Func 1: " << countOne1 << ";" << std::endl;
    std::cout << "Func 2: " << countOne2 << ";" << std::endl;
    std::cout << "res (" << operation << ") should be: " << countOneRes << ";" << std::endl;

    // build edges
    Edge e1, e2;
    if (isRel) {
        e1 = buildRelEdge(forest, num, fun1, 0, size-1);
        e2 = buildRelEdge(forest, num, fun2, 0, size-1);
    } else {
        e1 = buildSetEdge(forest, num, fun1, 0, size-1);
        e2 = buildSetEdge(forest, num, fun2, 0, size-1);
    }
    
    std::cout << "Func 1: " << std::endl;
    e1.print(std::cout);
    std::cout << std::endl;
    std::cout << "Func 2: " << std::endl;
    e2.print(std::cout);
    std::cout << std::endl;

    // call apply, and check results
    f1.setEdge(e1);
    f2.setEdge(e2);
    if (opt == BinaryOperationType::BOP_INTERSECTION) {
        res = f1 & f2;
    } else if (opt == BinaryOperationType::BOP_UNION) {
        res = f1 | f2;
    } else {
        std::cout << "Not implemented!" << std::endl;
    }
    std::cout << "result (" << operation << "): " << std::endl;
    res.getEdge().print(std::cout);
    std::cout << std::endl;

    // evaluation
    bool isPass = 1;
    std::vector<bool> assignment(num+1, 0);
    std::vector<bool> assignmentTo(num+1, 0);
    for (long long n=0; n<size; n++) {
        if (isRel) {
            for (uint16_t l=1; l<=num; l++) {
                assignment[l] = n & (0x01 << (2*l-1));
                assignmentTo[l] = n & (0x01 << (2*l-2));
            }
        } else {
            for (size_t k=1; k<=assignment.size()-1; k++) {
                assignment[k] = n & (1<<(k-1));
            }
        }
        Value val1, val2, valRes;
        val1 = (isRel) ? f1.evaluate(assignment, assignmentTo) : f1.evaluate(assignment);
        val2 = (isRel) ? f2.evaluate(assignment, assignmentTo) : f2.evaluate(assignment);
        valRes = (isRel) ? res.evaluate(assignment, assignmentTo) : res.evaluate(assignment);
        int valInt;
        val1.getValueTo(&valInt, INT);
        if (fun1[n] != valInt) {
            std::cout << "Func 1 evaluation failed!" << std::endl;
            isPass = 0;
        }
        val2.getValueTo(&valInt, INT);
        if (fun2[n] != valInt) {
            std::cout << "Func 2 evaluation failed!" << std::endl;
            isPass = 0;
        }
        valRes.getValueTo(&valInt, INT);
        int compRes = 0;
        if (opt == BinaryOperationType::BOP_INTERSECTION) {
            compRes = fun1[n] && fun2[n];
        } else if (opt == BinaryOperationType::BOP_UNION) {
            compRes = fun1[n] || fun2[n];
        }
        if (compRes != valInt) {
            std::cout << "result (" << operation << ") evaluation failed!" << std::endl;
            isPass = 0;
        }
        if (!isPass && (num <=6)) {
            std::cout << "Assignment: ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << assignment[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "Correct: " << compRes << "; result: " << valInt << std::endl;
            break;
        }
    }
    if (!isPass) {
        if (num < 6) {
            std::cout<<"Full function\n";
            std::cout << "\tfun1: \t\t";
            for (long long k=0; k<size; k++){
                std::cout << fun1[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "\tfun2: \t\t";
            for (long long k=0; k<size; k++){
                std::cout << fun2[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "\tres should be: \t";
            for (long long k=0; k<size; k++){
                std::cout << (fun1[k] && fun2[k]) << " ";
            }
        }
        std::cout << std::endl;
        DotMaker dot1(forest);
        dot1.buildGraph(f1, "func1");
        dot1.runDot("func1", "pdf");
        DotMaker dot2(forest);
        dot2.buildGraph(f2, "func2");
        dot2.runDot("func2", "pdf");
        DotMaker dot3(forest);
        dot3.buildGraph(res, "res");
        dot3.runDot("res", "pdf");
    }
    delete forest;
    return isPass;
}

bool testOperationValue(uint16_t num, PredefForest bdd, BinaryOperationType opt)
{
    // forest setting
    ForestSetting setting(bdd, num);
    setting.setValType(INT);
    setting.setMaxRange(12);
    Forest* forest = new Forest(setting);
    bool isRel = setting.isRelation();
    // functions
    Func f1(forest), f2(forest), res(forest);
    // truth table
    long long size = (isRel) ? 0x01LL<<(2*num) : 0x01LL<<(num);
    std::vector<Value> fun1(size);
    std::vector<Value> fun2(size);
    // countings
    long long countInf1 = 0, countInf2 = 0, countInfRes = 0;
    // build functions and countings
    int randomNum = 0;
    for (long long i=0; i<size; i++) {
        randomNum = random06(gen);
        if (randomNum == 6) {
            fun1[i] = Value(SpecialValue::POS_INF);
            countInf1++;
            // countInfRes++;
        } else {
            fun1[i] = randomNum;
        }
        randomNum = random06(gen);
        if (randomNum == 6) {
            fun2[i] = Value(SpecialValue::POS_INF);
            countInf2++;
            // countInfRes++;
        } else {
            fun2[i] = randomNum;
        }
        if (opt == BinaryOperationType::BOP_MINIMUM) {
            if ((fun1[i].getType() != VOID) && (fun2[i].getType() != VOID)) {
                countInfRes++;
            }
        } else if (opt == BinaryOperationType::BOP_MAXIMUM) {
            // TBD
        } else if (opt == BinaryOperationType::BOP_PLUS) {
            if ((fun1[i] == Value(SpecialValue::POS_INF)) || (fun2[i] == Value(SpecialValue::POS_INF))){
                countInfRes++;
            }
        }
    }
    std::string operation = "MINIMUM";
    if (opt == BinaryOperationType::BOP_MAXIMUM) {
        operation = "MAXIMUM";
    } else if (opt == BinaryOperationType::BOP_PLUS) {
        operation = "PLUS";
    }
    std::cout << "Infs counting:" << std::endl;
    std::cout << "Func 1: " << countInf1 << ";" << std::endl;
    std::cout << "Func 2: " << countInf2 << ";" << std::endl;
    std::cout << "res (" << operation << ") should be: " << countInfRes << ";" << std::endl;

    // build edges
    Edge e1, e2;
    if (isRel) {
        e1 = buildEvRelEdge(forest, num, fun1, 0, size-1);
        e2 = buildEvRelEdge(forest, num, fun2, 0, size-1);
    } else {
        e1 = buildEvSetEdge(forest, num, fun1, 0, size-1);
        e2 = buildEvSetEdge(forest, num, fun2, 0, size-1);
    }
    
    std::cout << "Func 1: " << std::endl;
    e1.print(std::cout);
    std::cout << std::endl;
    std::cout << "Func 2: " << std::endl;
    e2.print(std::cout);
    std::cout << std::endl;

    // call apply, and check results
    f1.setEdge(e1);
    f2.setEdge(e2);
    if (opt == BinaryOperationType::BOP_MINIMUM) {
        apply(MINIMUM, f1, f2, res);
    } else if (opt == BinaryOperationType::BOP_MAXIMUM) {
        apply(MAXIMUM, f1, f2, res);
    } else if (opt == BinaryOperationType::BOP_PLUS) {
        apply(PLUS, f1, f2, res);
    } else {
        std::cout << "Not implemented!" << std::endl;
    }
    std::cout << "result (" << operation << "): " << std::endl;
    res.getEdge().print(std::cout);
    std::cout << std::endl;

    // evaluation
    bool isPass = 1;
    std::vector<bool> assignment(num+1, 0);
    std::vector<bool> assignmentTo(num+1, 0);
    for (long long n=0; n<size; n++) {
        if (isRel) {
            for (uint16_t l=1; l<=num; l++) {
                assignment[l] = n & (0x01 << (2*l-1));
                assignmentTo[l] = n & (0x01 << (2*l-2));
            }
        } else {
            for (size_t k=1; k<=assignment.size()-1; k++) {
                assignment[k] = n & (1<<(k-1));
            }
        }
        Value val1, val2, valRes, correct;
        val1 = (isRel) ? f1.evaluate(assignment, assignmentTo) : f1.evaluate(assignment);
        val2 = (isRel) ? f2.evaluate(assignment, assignmentTo) : f2.evaluate(assignment);
        valRes = (isRel) ? res.evaluate(assignment, assignmentTo) : res.evaluate(assignment);
        if (fun1[n] != val1) {
            std::cout << "Func 1 evaluation failed!" << std::endl;
            isPass = 0;
        }
        if (fun2[n] != val2) {
            std::cout << "Func 2 evaluation failed!" << std::endl;
            isPass = 0;
        }
        if ((opt == BinaryOperationType::BOP_MINIMUM) || (opt == BinaryOperationType::BOP_MAXIMUM)) {
            if ((valRes != val1) && (valRes != val2)) {
                // wrong result for MINIMUM or MAXIMUM
                std::cout << "result (" << operation << ") evaluation failed!" << std::endl;
                isPass = 0;
            }
        } else {
            // other operations, TBD
        }
        
        if (opt == BinaryOperationType::BOP_MINIMUM) {
            correct = ((val1 == val2) || (val1 < val2)) ? val1 : val2;
            isPass = valRes == correct;
        } else if (opt == BinaryOperationType::BOP_MAXIMUM) {
            correct = ((val1 == val2) || (val1 < val2)) ? val2 : val1;
            isPass = valRes == correct;
        }else if (opt == BinaryOperationType::BOP_PLUS) {
            isPass = (valRes == (val1 + val2));
        } else {
            // TBD
        }
        if (!isPass && (num <=6)) {
            std::cout << "Assignment: ";
            for (uint16_t k=1; k<=num; k++){
                std::cout << assignment[k] << " ";
            }
            std::cout << std::endl;
            std::cout << "Correct: ";
            correct.print(std::cout);
            std::cout << "; result: ";
            valRes.print(std::cout);
            std::cout << std::endl;
            break;
        }
    }
    if (!isPass) {
        if (num < 6) {
            std::cout<<"Full function\n";
            std::cout << "\tfun1: \t\t";
            for (long long k=0; k<size; k++){
                fun1[k].print(std::cout);
                std::cout << " ";
            }
            std::cout << std::endl;
            std::cout << "\tfun2: \t\t";
            for (long long k=0; k<size; k++){
                fun2[k].print(std::cout);
                std::cout << " ";
            }
            std::cout << std::endl;
            // std::cout << "\tres should be: \t";
            // for (long long k=0; k<size; k++){
            //     std::cout << (fun1[k] && fun2[k]) << " ";
            // }
        }
        std::cout << std::endl;
        DotMaker dot1(forest);
        dot1.buildGraph(f1, "func1");
        dot1.runDot("func1", "pdf");
        DotMaker dot2(forest);
        dot2.buildGraph(f2, "func2");
        dot2.runDot("func2", "pdf");
        DotMaker dot3(forest);
        dot3.buildGraph(res, "res");
        dot3.runDot("res", "pdf");
    }
    delete forest;
    return isPass;
}

int main(int argc, char** argv){
    if (argc == 1) {
      printf("Usage: ./test_operation [PredefForest::bdd] [num_val]\n");
      printf("\tThis will randomly generate two boolean functions to test logic operation\n");
      exit(0);
    }
    bool isValued = 0;
    int TESTS = 1000;
    uint16_t numVals = 10;
    PredefForest bdd = PredefForest::REXBDD;
    BinaryOperationType opt = BinaryOperationType::BOP_UNION;
    // processing arguments TBD
    if ((argc == 3) || (argc == 4) || (argc == 5)) {
        bdd = (PredefForest)atoi(argv[1]);
        numVals = atoi(argv[2]);
        if (argc == 4) TESTS = atoi(argv[3]);
        if ((argc == 5) && (strcmp("-v", argv[4]) == 0)) {
            TESTS = atoi(argv[3]);
            isValued = 1;
        }
    }
    
    // forest setting
    // ForestSetting setting(bdd, numVals);
    // ForestSetting setting("sfbdd", numVals);
    // setting.output(std::cout);

    bool isPass = 0;
    // Randomly generate assignments and build two BDDs
    if (!isValued) {
        for (int test=0; test<TESTS; test++) {
            // isPass = testBinaryOperation(numVals, bdd, opt);
            // isPass = testUnaryOperation(numVals, "sfbdd", UnaryOperationType::UOP_COMPLEMENT);
            isPass = testXOR(numVals, bdd);
            if (!isPass) break;
        }
    } else {
        for (int test=0; test<TESTS; test++) {
            isPass = testOperationValue(numVals, bdd, opt);
            if (!isPass) break;
        }
    }
    

    if (!isPass) {
        std::cout << "Test Failed!" << std::endl;
    } else {
        std::cout << "Test Pass!" << std::endl;
    }
    return 0;
}