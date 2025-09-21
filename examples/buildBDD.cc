#include <vector>
#include <unordered_set>
#include "/home/dara/Git/brave_dd/src/brave_dd.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>




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
std::string intToBoolVariable(int varIndex);
std::string boolVariableToInt(int varIndex);
void printCurLevelVerticies(std::vector<uint64_t> curLevelVerticies);
std::string intToAlphaString(int number);
std::unordered_map<uint64_t, int> renameVertices(const std::vector<uint64_t>& vertexNames);


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
int maxLvl = 4;

/**
 *Mathod QRBDDToBoolForDFA
 *@brief Takes in a non terminal QRMDD(QRBDD for now), a size for number of states in the DFA, and number of possible assignments of each variable (2 for BDD)
 * creates a DIMACS CNF formatted text file that can go to a sat solver
 * this the sat solver will give wether or not it is possible to create a DFA of the given size
 * if it is possible, you can use the assignments to construct it.
**/
int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments){
    //this counter increaments everytime a clause is added to our boolean function
    int numClauses = numverticis;
    numStatesG = numStates;
    int vertecisOnLvl;
    int boolVariableToIntParms1 [4];
    int boolVariableToIntParms2 [4];
    int curVertex = 0;
    boolVariableToIntParms1[0] = curVertex;
    std::string function;
    std::string tempFunction = "";
    std::string childFunction;


    std::vector<uint64_t> curLevelVerticies; //has all of the verticies on current level
    std::unordered_set<uint64_t> seen; //says weither a vertex has been seen yet (I want controlled order so I can't just have it here)
    BRAVE_DD::NodeHandle curVertexHandle = qrbdd.getEdge().getNodeHandle();
    uint64_t uniqueVertexName = (static_cast<uint64_t>(0x4) << 48) | curVertexHandle;
    std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
    curLevelVerticies.push_back(uniqueVertexName);
    seen.insert(uniqueVertexName);

    std::vector<uint64_t> allVerticies;
    allVerticies.push_back(uniqueVertexName);
    int verticiesOnLvl = curLevelVerticies.size();
    for(int lvl = maxLvl -1; lvl > 0; lvl --){
        for (verticiesOnLvl; verticiesOnLvl > 0; verticiesOnLvl --){
            for (int x = 0; x < numAssignments; x++){
                //get handle of the vertecy
                curVertexHandle = qrbdd.getForest()->getChildNodeHandle(lvl + 1,curVertexHandle,x);
                //create a unique name from handle and level
                uniqueVertexName = static_cast<uint64_t>(lvl) << 48 | curVertexHandle;
                std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
                //check if seen befor and add it to seen if not
                if(seen.insert(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl+1,curVertexHandle,x)).second){
                    //add to the stack
                    curLevelVerticies.push_back(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl+1,curVertexHandle,x));
                    //add to all verticies
                    allVerticies.push_back(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl+1,curVertexHandle,x));
                }
            }
            curLevelVerticies.erase(curLevelVerticies.begin());
        }
        verticiesOnLvl = curLevelVerticies.size();
    }



    auto renamed = renameVertices(allVerticies);

    for (const auto& [name, id] : renamed) {
        std::cout << "Vertex " << name << " -> ID " << id << "\n";
    }

    return 0;


    //build sat problem
    //root(v_a) belongs to q0
    function += "1 0\n";
    curVertex++;
    numClauses++;
    //each of the remaning vertecies can be in at least 1 of each state
    for (curVertex; curVertex < numverticis; curVertex++){
        boolVariableToIntParms1[0] = curVertex;
        //incrementing the state
        for (int i = 0; i < numStatesG; i++){
            boolVariableToIntParms1[1] = i;
            function += std::to_string(boolVariableToInt(v_AlphBelongsToq_i, boolVariableToIntParms1))+ " ";
        }
        function += "0\n" + tempFunction;
    }
    std::cout << function << std::endl; 

    
    curVertex = 0;
    int parent = curVertex;
    int child;
    //find number of children that root has and add them to the stack
    for(int lvl = 3; lvl > 0; lvl --){
        for (int x = 0; x < numAssignments; x++){
            curVertexHandle = qrbdd.getForest()->getChildNodeHandle(lvl + 1,curVertexHandle,x);
            uniqueVertexName = static_cast<uint64_t>(lvl) << 48 | curVertexHandle;
            std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
            //check to see if already part of curLevelVerticies
            if(seen.insert(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl+1,curVertexHandle,x)).second){
                //add to the stack
                curLevelVerticies.push_back(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl+1,curVertexHandle,x));
            }
        }
    }
    


    /*//find number of children that root has and add them to the stack
    vertecisOnLvl = curLevelVerticies.size();
    curVertexHandle = curLevelVerticies.back();
    //qrbdd.getEdge().getNodeLevel();
    //gets two children of root
    for (int x = 0; x < numAssignments; x++){
        //prep for implication
        boolVariableToIntParms1[0] = 0;
        boolVariableToIntParms1[1] = 0;
        //prep for high low child
        //boolVariableToIntParms1[2] = x;
        //in this the parent is always 0
        //boolVariableToIntParms1[3] = 0;
        uniqueVertexName = static_cast<uint64_t>(0x3) << 48 | curVertexHandle;
        std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
        //check to see if already part of curLevelVerticies
        if(seen.insert(static_cast<uint64_t>(0x3) << 48 | qrbdd.getForest()->getChildNodeHandle(4,curVertexHandle,x)).second){
            //add to the stack
            curLevelVerticies.push_back(static_cast<uint64_t>(3) << 48 | qrbdd.getForest()->getChildNodeHandle(4,curVertexHandle,x));
            //root's children belong to 1 of three states q0, q1, or q2 and if root belongs to 0 then we need 0 to point to what roots child belongs to
            std::cout << function << std::endl;
            //function += "-" + std::to_string(boolVariableToInt(v_AlphBelongsToq_i, boolVariableToIntParms1))+ " ";
            //std::cout << function << std::endl;
            boolVariableToIntParms1[0] = x + 1;
            for(int i = 0; i < numStates; i++){
                //if root belongs to 0 then we need 0 to point to what roots child belongs to
                //format of ((v_alpha belongs to q_i) & (delta (q_j,x) = q_i), {alpha, i, x, j})
                boolVariableToIntParms1[1] = i;
                //function += std::to_string(boolVariableToInt(belongsAndDelta, boolVariableToIntParms1))+ " ";
                //root's children belong to 1 of three states q0, q1, or q2
                function += std::to_string(boolVariableToInt(v_AlphBelongsToq_i, boolVariableToIntParms1))+ " ";
            }
            //tempFunction += "0\n";
            function += "0\n" + tempFunction;
            std::cout << function << std::endl; 
        }
    }
    curLevelVerticies.erase(curLevelVerticies.begin());
    printCurLevelVerticies(curLevelVerticies);*/



    /*// but they don't ever belong to the same
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

    curVertex -= curLevelVerticies.size();std::endl
    boolVariableToIntParms1[0] = curVertex;*/
    //Link variabls together 



    std::cout << function;
    //add p cnf <num variables> <num clauses> to tope of file   
    std::ofstream outFile("satFunctionForQRBDDtoDFA.txt");
    if (!outFile) {
        std::cerr << "Error opening file: " << "satFunctionForQRBDDtoDFA.txt" << std::endl; // O(1)
        return;
    }

    outFile << function; // O(n) — writes each character of the string
    outFile.close();    // O(1) — flushes and closes the file
    

    std::ifstream inFile("satFunctionForQRBDDtoDFA.txt");
    if (!inFile) {
        std::cerr << "Error opening file: " << "satFunctionForQRBDDtoDFA.txt" << std::endl; // O(1)
        return;
    }

    std::string line;
    // Loop over each line — O(m), where m is the number of lines
    while (std::getline(inFile, line)) {
        std::istringstream lineStream(line); // O(1)
        std::string word;
        bool isFirst = true;

        // Loop over each word in the line — O(k), where k is the number of words in the line
        while (lineStream >> word && word != "0") {
            if (!isFirst){
                std::cout << "\\vee ";
            }
            std::cout << intToBoolVariable(std::stoi(word)) << " "; // O(1)
            isFirst = false;
        }
        std::cout << "\\wedge \\\\" << std::endl;
    }

    inFile.close(); // O(1)

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
    if(varType == v_AlphBelongsToq_i){
        // take vertex and multiply it by the number of states, add the state number and add 1 since minimum is 1
        return numStatesG * parms[0]+ 
        // i increments each time
        parms[1] + 
        // ofset up one so that we don't double dip an index since we are 1 indexing not 0 indexing
        1;
    }
    //delta (state i, assingment x) = state j
    //each of the |Q| states has x edges, each edge can go to |Q| states, so number of variables are |Q|(number of states edges from)*x*|Q|(number of states edges too)
    //goes from (n * |Q| + 1) -> (n * |Q| + |Q|*x*|Q|)
    //format of (delta (q_i,x) = q_j) is (deltaq_iXIsq_j, {i, x, j})
    else if(varType == deltaq_iXIsq_j){
        // for each number of symbols in the alphavet times of incrementing x, increment j
        return numInAlphabet * numverticis * parms[1] +
        // for each number of vertex times of incrementing i, increment x
        numverticis * parms[1] +
        // increment i each time
        parms[0] +
        // move to the location of the last variable index before deltaq_iXIsq_j
        numberOfv_AlphBelongsToq_i +
        // ofset up one so that we don't double dip an index since we are 1 indexing not 0 indexing
        1;
    }
    //format of ((v_alpha belongs to q_i) & (delta (q_j,x) = q_i), {alpha, i, x, j})
    else if(varType == belongsAndDelta){
        // for each numberOfSymbols in the alphabet increments of x, increment j
        return numInAlphabet * numverticis * numStatesG * parms[3] + 
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
std::string intToBoolVariable(int varIndex){
    //lets set up the variables
    int numberOfv_AlphBelongsToq_i = numStatesG * numverticis;
    u_int32_t numberOfdeltaq_iXIsq_j = numStatesG * numverticis;
    std::string isNeg = "";
    std::string alpha = "";
    std::string i = "";
    std::string x = "";
    std::string j = "";
    //vertex alpha belongs to state i
    //there are n verticies (including terminal), there are |Q| states. there will be n * |Q| variables representing these from 1 -> n * |Q|
    //format of (v_alpha belongs to q_i) isNegis (v_AlphBelongsToq_i, {alpha, i})
    if(varIndex < 0){
        isNeg = "\\neg ";
    }
    if(varIndex <= numberOfv_AlphBelongsToq_i){
        // take vertex and multiply it by the number of states, add the state number and add 1 since minimum is 1
        alpha = intToAlphaString(varIndex / numStatesG);
        i = std::to_string(varIndex % numStatesG - 1);
        return isNeg + "\\v_"+ alpha + "\\relSym\\q_"+ i ;
    }
    //delta (state i, assingment x) = state j
    //each of the |Q| states has x edges, each edge can go to |Q| states, so number of variables are |Q|(number of states edges from)*x*|Q|(number of states edges too)
    //goes from (n * |Q| + 1) -> (n * |Q| + |Q|*x*|Q|)
    //format of (delta (q_i,x) = q_j) is (deltaq_iXIsq_j, {i, x, j})
    /*else if(varIndex == deltaq_iXIsq_j){
        // for each number of symbols in the alphavet times of incrementing x, increment j
        return numInAlphabet * numverticis * parms[1] +
        // for each number of vertex times of incrementing i, increment x
        numverticis * parms[1] +
        // increment i each time
        parms[0] +
        // move to the location of the last variable index before deltaq_iXIsq_j
        numberOfv_AlphBelongsToq_i +
        // ofset up one so that we don't double dip an index since we are 1 indexing not 0 indexing
        1;
    }*/
    //format of ((v_alpha belongs to q_i) & (delta (q_j,x) = q_i), {alpha, i, x, j})
    else {
        // for each numberOfSymbols in the alphabet increments of x, increment j
        varIndex -= numberOfv_AlphBelongsToq_i + numberOfv_AlphBelongsToq_i;
        j = std::to_string(varIndex / (numInAlphabet * numverticis * numStatesG)); 
        //std::cout << j << std::endl;
        x = std::to_string(((varIndex / (numverticis * numStatesG)) % (numInAlphabet))); 
        //std::cout << x << std::endl;
        // there will be, the number of verticies befor incrementing x, so for each numberOfVerticie increments in i, incrment x. 
        /*numverticis * numStatesG * parms[2] +  
        // there are numStatesG options for i to change befor alpha changes. so for each increment of alpha, increase by numStatesG
        numStatesG * parms[0] + 
        // i will change every time
        parms [1] + 
        // ofset up one so that we don't double dip an index since we are 1 indexing not 0 indexing
        1;*/
        return isNeg + "\\v_ NIAlph \\relSym\\q_ NIi \\wedge (\\delta(\\q_" + j +", " + x + ")) = \\q_NIi";
    }    


    return "NI";//not implemented
}
void printCurLevelVerticies(std::vector<uint64_t> curLevelVerticies){
    std::vector<uint64_t> temp = curLevelVerticies;
    std::cout << "test" << std::endl;
    for(int i = 0; i < temp.size(); i++){
        std::cout << std::hex << "Result: 0x" << temp[i] << std::endl;
    }
}
/**
 * @brief Converts an integer to a custom alphabetic string (like Excel columns but 0-indexed).
 *        a = 0, b = 1, ..., z = 25, aa = 26, ab = 27, ...
 * @param number The integer to convert (must be >= 0).
 * @return A string representing the encoded value.
 * @note Time Complexity: O(log₍₂₆₎n) — each division reduces the number by a factor of 26.
 */
std::string intToAlphaString(int number) {
    std::string result;

    // Loop runs O(log₍₂₆₎n) times
    while (number >= 0) {
        int remainder = number % 26; // O(1)
        result = static_cast<char>('a' + remainder) + result; // O(1)
        number = number / 26 - 1; // O(1)
    }

    return result;
}

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

/**
 * @brief Renames each vertex in a DAG to a unique integer ID from 0 to N-1.
 *
 * @param vertexNames A vector of strings representing the original names of the vertices.
 * @return std::unordered_map<std::string, int> A map from original vertex names to new integer IDs.
 *
 * @note The order of IDs is based on the order of names in the input vector.
 *       If you want topological order, you must run a topological sort first.
 *
 * @complexity Time Complexity: O(N), where N is the number of vertices.
 *             Space Complexity: O(N), for storing the mapping.
 */
std::unordered_map<uint64_t, int> renameVertices(const std::vector<uint64_t>& vertexNames) {
    std::unordered_map<uint64_t, int> nameToId;

    for (size_t i = 0; i < vertexNames.size(); ++i) {
        nameToId[vertexNames[i]] = static_cast<int>(i);
    }

    return nameToId;
}


/*int main() {
    int input = 27;

    // Call to intToAlphaString — O(log₍₂₆₎1 0
5 6 7 8 0
9 10 11 12 0
13 14 15 16 0
17 18 19 20 0
21 22 23 24 0
25 26 27 28 0
29 30 31 32 0
33 34 35 36 0
37 38 39 40 0
41 42 43 44 0
45 46 47 48 0n)
    std::string output = intToAlphaString(input);

    std::cout << "Encoded string: " << output << std::endl; // O(1)
    return 0;
}*/