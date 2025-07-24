#include <vector>
#include <unordered_set>
#include "/home/dara/Git/brave_dd/src/brave_dd.h"



enum VarType{
    v_AlphBelongsToq_i,
    deltaq_iXIsq_j,
    belongsAndDelta// and of the previous two
};

enum alphabet{
    bot,
    top
};



int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments);
int boolVariableToInt(int varType, int* parms);
std::string boolVariableToInt(int varIndex);

using namespace BRAVE_DD;

int main () {

    //create bdd x_1
    //needs to also have 
    ForestSetting settingx_1("QBDD",1);

    Forest* forestx_1 = new Forest(settingx_1);

    Func rootx_1(forestx_1);

    rootx_1.variable(1);

    Func resultx_1 = rootx_1;

    //create bdd x_1
    //needs to also have 
    ForestSetting settingx_2("QBDD",4);

    Forest* forestx_2 = new Forest(settingx_2);

    Func root1(forestx_2);
    Func root2(forestx_2);
    Func root3(forestx_2);
    Func root4(forestx_2);

    root1.variable(1);
    root2.variable(2);
    root3.variable(3);
    root4.variable(4);

    Func result(forestx_2);
    result = (root4&root2)|(root3&root1);

    DotMaker dot1(forestx_2, "func2");
    dot1.buildGraph(result);
    dot1.runDot("pdf");



    qRBDDToBoolForDFA(result,4,2);


    delete forestx_1;
    delete forestx_2;
    

    printInfo();
    return 0;

}


int numverticis = 12;
int numInAlphabet = 2;  
int numStatesG;

/**
 *Mathod QRBDDToBoolForDFA
 *@brief Takes in a non terminal QRMDD(QRBDD for now), a size for number of states in the DFA, and number of possible assignments of each variable (2 for BDD)
 * creates a DIMACS CNF formatted text file that can go to a sat solver
 * this the sat solver will give wether or not it is possible to create a DFA of the given size
 * if it is possible, you can use the assignments to construct it.
**/
int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments){
    //this counter increaments everytime a clause is added to our boolean function
    int numClauses = 0;
    numStatesG = numStates;
    int vertecisOnLvl;
    int boolVariableToIntParms1 [4];
    int boolVariableToIntParms2 [4];
    int curVertex = 0;
    boolVariableToIntParms1[0] = curVertex;
    std::string function;
    std::string childFunction;


    std::vector<uint64_t> curLevelVerticies; //has all of the verticies on current level
    std::unordered_set<uint64_t> seen; //says weither a vertex has been seen yet (I want controlled order so I can't just have it here)
    BRAVE_DD::NodeHandle curVertexHandle = qrbdd.getEdge().getNodeHandle();
    uint64_t uniqueVertexName = (static_cast<uint64_t>(0x4) << 48) | curVertexHandle;
    std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;


    curLevelVerticies.push_back(uniqueVertexName);
    seen.insert(uniqueVertexName);

    
    
    //build sat problem
    //root(v_a) belongs to q0
    function += "1 0\n";
    curVertex++;
    //so when boolVariableToInt setting i for (delta (q_i,x) = q_j)
    boolVariableToIntParms1[0] = curVertex;
    numClauses++;



    //find number of children that root has and add them to the stack
    vertecisOnLvl = curLevelVerticies.size();
    curVertexHandle = curLevelVerticies.back();
    //qrbdd.getEdge().getNodeLevel();
    for (int i = 0; i < numAssignments; i++){
        uniqueVertexName = static_cast<uint64_t>(0x3) << 48 | curVertexHandle;
        std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
        //check to see if already part of curLevelVerticies
        if(seen.insert(static_cast<uint64_t>(0x3) << 48 | qrbdd.getForest()->getChildNodeHandle(4,curVertexHandle,i)).second){
            //add to the stack
            curLevelVerticies.push_back(static_cast<uint64_t>(3) << 48 | qrbdd.getForest()->getChildNodeHandle(4,curVertexHandle,i));
        }
    }
    curLevelVerticies.pop_back();


    //root's children belong to 1 of three states q0, q1, or q2 but they don't ever belong to the same
    //start with at least one
    vertecisOnLvl = curLevelVerticies.size();
    for(uint64_t i = 0; i < curLevelVerticies.size(); i++){
        for(int j = 0; j < std::max(vertecisOnLvl,3); j++){
            boolVariableToIntParms1[1] = i;
            function += "%d ", boolVariableToInt(v_AlphBelongsToq_i, boolVariableToIntParms1);
        }
        function += "0\n";
        curVertex++;
        boolVariableToIntParms1[0] = curVertex;
        numClauses++;
    }
    curVertex -= curLevelVerticies.size();
    boolVariableToIntParms1[0] = curVertex;
    //do each pair of illegal connections
    for(uint64_t i = 0; i < curLevelVerticies.size(); i++){
        for(uint64_t j = i+1; i < curLevelVerticies.size(); j++){
            boolVariableToIntParms2[0] = curVertex + (j-i); 
            function += "-%d -%d 0\n", boolVariableToInt(v_AlphBelongsToq_i, boolVariableToIntParms1), boolVariableToInt(v_AlphBelongsToq_i, boolVariableToIntParms2); 
            numClauses++;
        }
        curVertex++;
        boolVariableToIntParms1[0] = curVertex;  
    }

    curVertex -= curLevelVerticies.size();
    boolVariableToIntParms1[0] = curVertex;
    //Link variabls together 



    //add p cnf <num variables> <num clauses> to tope of file
    return 0;
}




/**
 * @brief from boolean variable to int: takes in an int and an int array formatted as below. then retruns an int:
 * 
 * format of (v_alpha belongs to q_i) is (v_AlphBelongsToq_i, {alpha, i})
 * 
 * format of (delta (q_i,x) = q_j) is (deltaq_iXIsq_j, {i, x, j})
 * 
 * format of ((v_alpha belongs to q_i) & (delta (q_i,x) = q_j), {alpha, i, x, j})
*/
int boolVariableToInt(int varType, int* parms){
    //lets set up the variables
    int numberOfv_AlphBelongsToq_i = numStatesG * numverticis;
    u_int32_t numberOfdeltaq_iXIsq_j = numStatesG * numverticis;
    //vertex alpha belongs to state i
    //there are n verticies (including terminal), there are |Q| states. there will be n * |Q| variables representing these from 1 -> n * |Q|
    //format of (v_alpha belongs to q_i) is (v_AlphBelongsToq_i, {alpha, i})
    if(varType = v_AlphBelongsToq_i){
        // take vertex and multiply it by the number of states, add the state number and add 1 since minimum is 1
        numStatesG * parms[0]+ 
        // alpha increments each time
        parms[1] + 
        // ofset up one so that we don't double dip an index since we are 1 indexing not 0 indexing
        1;
    }
    //delta (state i, assingment x) = state j
    //each of the |Q| states has x edges, each edge can go to |Q| states, so number of variables are |Q|(number of states edges from)*x*|Q|(number of states edges too)
    //goes from (n * |Q| + 1) -> (n * |Q| + |Q|*x*|Q|)
    //format of (delta (q_i,x) = q_j) is (deltaq_iXIsq_j, {i, x, j})
    else if(varType = deltaq_iXIsq_j){
        // for each number of symbols in the alphavet times of incrementing x, increment j
        numInAlphabet * numverticis * parms[1] +
        // for each number of vertex times of incrementing i, increment x
        numverticis * parms[1] +
        // increment i each time
        parms[0] +
        // move to the location of the last variable index before deltaq_iXIsq_j
        numberOfv_AlphBelongsToq_i +
        // ofset up one so that we don't double dip an index since we are 1 indexing not 0 indexing
        1;
    }
    //format of ((v_alpha belongs to q_i) & (delta (q_i,x) = q_j), {alpha, i, x, j})
    else if(varType = belongsAndDelta){
        // for each numberOfSymbols in the alphabet increments of x, increment j
        numInAlphabet * numverticis * numStatesG * parms[3] + 
        // there will be, the number of verticies befor incrementing x, so for each numberOfVerticie increments in i, incrment x. 
        numverticis * numStatesG * parms[2] +  
        // there are numStatesG options for i to change befor alpha changes. so for each increment of alpha, increase by numStatesG
        numStatesG * parms[0] + 
        // i will change every time
        parms [1] + 
        // move to the location of the last variable index before belongsAndDelta
        numberOfv_AlphBelongsToq_i + numberOfv_AlphBelongsToq_i + 
        // ofset up one so that we don't double dip an index since we are 1 indexing not 0 indexing
        1;
    }       

    return -1;
}


//from number to variable
std::string boolVariableToInt(int varIndex){
    return "";
}