#include "/home/dara/Git/brave_dd/src/brave_dd.h"
#include <unordered_map>

using namespace BRAVE_DD;

#include <iostream>

enum VarType
{
    v_AlphBelongsToq_i,
    deltaq_iXIsq_j,
    belongsAndDelta // and of the previous two
};

enum alphabet
{
    bot,
    top
};

/**
 * @brief Represents a variable with an index and a boolean flag.
 *
 * This class maintains a static maximum index shared across all instances,
 * and each instance has its own index and boolean flag.
 */
class variable
{
private:
    /**
     * @brief Static member to track the maximum index assigned.
     * Shared across all instances of Variable.
     */
    static int maxIndex;

    /**
     * @brief Instance-specific index value.
     * Initialized to 0 by default.
     */
    int myIndex;

    /**
     * @brief Boolean flag associated with the variable.
     */
    bool isTrue;

    static std::vector<variable *> registry;



public:
    /**
     * @brief Default constructor for the Variable class.
     *
     */
        variable()
    {
        myIndex = 0;
    }
    /**
     * @brief flag associated with the variable type.
     * v_AlphBelongsToq_i = 0
     * deltaq_iXIsq_j = 1
     */
    int varTypeG;

    int alpha;

    int i;

    int j;

    int x;


    variable(int varType, int alphaT, int iT)
    {
        varTypeG = varType;
        alpha = alphaT;
        i = iT;
        myIndex = 0;
    }

    variable(int varType, int iT, int jT, int xT)
    {
        varTypeG = varType;
        i = iT;
        j = jT;
        x = xT;
        myIndex = 0;
    }
    void activate()
    {
        if (myIndex > 0)
            return;
        myIndex = maxIndex++;
        registry.push_back(this);
    }
    static void init()
    {
        maxIndex = 1;
        registry.push_back(nullptr);
    }
    inline static variable *getvariableIndex(int i)
    {
        return registry.at(i);
    }
    int getMyIndex()
    {
        return myIndex;
    }
    int getVarType(){
        return varTypeG;
    }
};

int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments);
std::unordered_map<uint64_t, int> renameVertices(const std::vector<uint64_t> &vertexNames);
std::string variableName(variable var);

int variable::maxIndex;
std::vector<variable *> variable::registry;

int main()
{

    // create bdd x_1
    // needs to also have
    ForestSetting settingx_1("QBDD", 1);

    Forest *forestx_1 = new Forest(settingx_1);

    Func rootx_1(forestx_1);

    rootx_1.variable(1);

    Func resultx_1 = rootx_1;

    // create bdd x_1
    // needs to also have
    ForestSetting settingx_2("QBDD", 4);

    Forest *forestx_2 = new Forest(settingx_2);

    Func root1(forestx_2);
    Func root2(forestx_2);
    Func root3(forestx_2);
    Func root4(forestx_2);

    root1.variable(1);
    root2.variable(2);
    root3.variable(3);
    root4.variable(4);

    Func result(forestx_2);
    result = (root4 & root2) | (root3 & root1);

    DotMaker dot1(forestx_2, "func2");
    dot1.buildGraph(result);
    dot1.runDot("pdf");

    qRBDDToBoolForDFA(result, 4, 2);

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
int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments)
{
    numStatesG = numStates;

    std::unordered_set<uint64_t> seen; // says weither a vertex has been seen yet (I want controlled order so I can't just have it here)
    BRAVE_DD::NodeHandle parentVertexHandle = qrbdd.getEdge().getNodeHandle();
    uint64_t uniqueVertexName = (static_cast<uint64_t>(0x4) << 48) | parentVertexHandle;
    std::vector<uint64_t> curLevelVerticies; // has all of the verticies on current level
    BRAVE_DD::NodeHandle curVertexHandle;

    // Rename verticies
    std::vector<uint64_t> allVerticies;
    allVerticies.push_back(uniqueVertexName);
    int verticiesOnLvl = curLevelVerticies.size();
    for (int lvl = maxLvl - 1; lvl >= 0; lvl--)
    {
        for (verticiesOnLvl; verticiesOnLvl > 0; verticiesOnLvl--)
        {
            std::cout << std::hex << "Result: 0x" << curLevelVerticies.front() << std::endl;
            parentVertexHandle = curLevelVerticies.front();
            for (int x = 0; x < numAssignments; x++)
            {
                // get handle of the vertecy
                curVertexHandle = qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x);
                // create a unique name from handle and level
                uniqueVertexName = static_cast<uint64_t>(lvl) << 48 | curVertexHandle;
                std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
                // check if seen befor and add it to seen if not
                if (seen.insert(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x)).second)
                {
                    // add to the stack
                    curLevelVerticies.push_back(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x));
                    // add to all verticies
                    allVerticies.push_back(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x));
                }
            }
            curLevelVerticies.erase(curLevelVerticies.begin());
        }
        verticiesOnLvl = curLevelVerticies.size();
    }

    auto renamed = renameVertices(allVerticies);

    // initialize variable
    variable::init();
    // instanitiate all possible variables
    // start with belongs to
    variable belongsTo[numverticis][numStates];
    for (int alpha = 0; alpha < numverticis; alpha++)
    {
        for (int i = 0; i < numStates; i++)
        {
            belongsTo[alpha][i] = variable(v_AlphBelongsToq_i,alpha,i);
            // belongsTo[alpha][i].activate();
        }
    }
    // then do delta function
    variable deltaFun[numStates][numStates][numAssignments];
    for (int x = 0; x < numAssignments; x++)
    {
        for (int i = 0; i < numStates; i++)
        {
            for (int j = 0; j < numStates; j++)
            {
                deltaFun[i][j][x] = variable(deltaq_iXIsq_j,i,j,x);
                std::cout << variableName(deltaFun[i][j][x]) << std::endl;
                // delta[i][j][x].activate();
            }
        }
    }

    std::string function;
    int curVertex = 0;
    int numClauses = 0;

    // build sat problem
    // root(v_a) belongs to q0
    belongsTo[0][0].activate();
    // std::cout << belongsTo[0][0].getMyIndex() << std::endl;
    function += std::to_string(belongsTo[0][0].getMyIndex()) + " 0\n";
    numClauses++;
    curVertex++;
    // each of the remaning vertecies can be in at least 1 of each state
    for (curVertex; curVertex < numverticis; curVertex++)
    {
        // incrementing the state
        for (int i = 0; i < numStates; i++)
        {
            belongsTo[curVertex][i].activate();
            // std::cout << belongsTo[curVertex][i].getMyIndex() << std::endl;
            function += std::to_string(belongsTo[curVertex][i].getMyIndex()) + " ";
        }
        function += "0\n";
        numClauses++;
    }
    // std::cout << function << std::endl;

    // no two states on the same levl can belong to the same state

    parentVertexHandle = qrbdd.getEdge().getNodeHandle();
    seen.clear();
    curLevelVerticies.clear();
    uniqueVertexName = (static_cast<uint64_t>(0x4) << 48) | parentVertexHandle;
    std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
    curLevelVerticies.push_back(uniqueVertexName);
    seen.insert(uniqueVertexName);
    verticiesOnLvl = curLevelVerticies.size();
    for (int lvl = maxLvl - 1; lvl >= 0; lvl--)
    {
        for (verticiesOnLvl; verticiesOnLvl > 0; verticiesOnLvl--)
        {
            parentVertexHandle = curLevelVerticies.front();
            for (int x = 0; x < numAssignments; x++)
            {
                // get handle of the vertecy
                curVertexHandle = qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x);
                // create a unique name from handle and level
                uniqueVertexName = static_cast<uint64_t>(lvl) << 48 | curVertexHandle;
                if (seen.insert(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x)).second)
                {
                    // add to the stack
                    curLevelVerticies.push_back(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x));
                }
            }
            curLevelVerticies.erase(curLevelVerticies.begin());
        }
        verticiesOnLvl = curLevelVerticies.size();
        for (int i = 0; i < numStates; i++)
        {
            // start the start of the line
            for (int alpha = 0 + seen.size() - verticiesOnLvl; alpha < seen.size(); alpha++)
            {
                for (int beta = alpha + 1; beta < seen.size(); beta++)
                {
                    belongsTo[alpha][i].activate();
                    function += "-" + std::to_string(belongsTo[alpha][i].getMyIndex()) + " ";
                    belongsTo[beta][i].activate();
                    function += "-" + std::to_string(belongsTo[beta][i].getMyIndex()) + " ";
                    function += "0\n";
                    numClauses++;
                }
            }
        }
        // std::cout << function << std::endl;
    }

    // each delta must map to at least 1.
    // ourter is based on variable assignments
    for (int x = 0; x < numAssignments; x++)
    {
        for (int j = 0; j < numStates; j++)
        {
            for (int i = 0; i < numStates; i++)
            {

                deltaFun[j][i][x].activate();
                std::cout << variableName(deltaFun[j][i][x]) << std::endl;
                // printf("%d\n",delta[x][i][j].getMyIndex());
                function += std::to_string(deltaFun[j][i][x].getMyIndex()) + " ";
            }
            function += "0\n";
            numClauses++;
        }
        std::cout << function << std::endl;
    }

    // each delta must be at most 1
    for (int x = 0; x < numAssignments; x++)
    {
        for (int i = 0; i < numStates; i++)
        {
            for (int jPrime = 0; jPrime < numStates; jPrime++)
            {
                for (int jSub = jPrime + 1; jSub < numStates; jSub++)
                {
                    deltaFun[i][jPrime][x].activate();
                    std::cout << variableName(deltaFun[x][jPrime][i]) << std::endl;
                    function += "-" + std::to_string(deltaFun[x][jPrime][i].getMyIndex()) + " ";
                    deltaFun[i][jSub][x].activate();
                    std::cout << variableName(deltaFun[i][jSub][x]) << std::endl;
                    function += "-" + std::to_string(deltaFun[x][jSub][i].getMyIndex()) + " ";
                    function += "0\n";
                    std::cout << function << std::endl;
                    numClauses++;
                }
            }
            std::cout << function << std::endl;
        }
    }

    // each delta must be at most 1
    /*for (int x = 0; x < numAssignments; x++)
    {
        boolVariableToIntParms1[2] = x;
        for (int i = 0; i < numStates; i++)
        {
            boolVariableToIntParms1[1] = i;
            for (int jPrime = 0; jPrime < numStates; jPrime++)
            {
                for (int jSub = jPrime + 1; jSub < numStates; jSub++)
                {
                    boolVariableToIntParms1[0] = jPrime;
                    function += "-" + std::to_string(boolVariableToInt(deltaq_iXIsq_j, boolVariableToIntParms1)) + " ";
                    boolVariableToIntParms1[0] = jSub;
                    function += "-" + std::to_string(boolVariableToInt(deltaq_iXIsq_j, boolVariableToIntParms1)) + " ";
                    function += "0\n";
                    numClauses++;
                }
            }
            std::cout << function << std::endl;
        }
    }*/

    return 0;
}

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
std::unordered_map<uint64_t, int> renameVertices(const std::vector<uint64_t> &vertexNames)
{
    std::unordered_map<uint64_t, int> nameToId;

    for (size_t i = 0; i < vertexNames.size(); ++i)
    {
        nameToId[vertexNames[i]] = static_cast<int>(i);
    }

    return nameToId;
}

/**
 * @brief Converts an integer to a custom alphabetic string (like Excel columns but 0-indexed).
 *        a = 0, b = 1, ..., z = 25, aa = 26, ab = 27, ...
 * @param number The integer to convert (must be >= 0).
 * @return A string representing the encoded value.
 * @note Time Complexity: O(log₍₂₆₎n) — each division reduces the number by a factor of 26.
 */
std::string intToAlphaString(int number)
{
    std::string result;

    // Loop runs O(log₍₂₆₎n) times
    while (number >= 0)
    {
        int remainder = number % 26;                          // O(1)
        result = static_cast<char>('a' + remainder) + result; // O(1)
        number = number / 26 - 1;                             // O(1)
    }

    return result;
}

std::string variableName(variable var){
    switch (var.getVarType()){
        case v_AlphBelongsToq_i:
                return "v_"+ intToAlphaString(var.alpha)+ " \\relSym\\q_" + std::to_string(var.i);
            break;
        case deltaq_iXIsq_j:
                return "\\delta (q_" + std::to_string(var.i) + " ," + std::to_string(var.x) + ") = q_" + std::to_string(var.j);
            break;
    }
}