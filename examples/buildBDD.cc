#include "/home/dara/Git/brave_dd/src/brave_dd.h"
#include <unordered_map>

using namespace BRAVE_DD;

int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments);
std::unordered_map<uint64_t, int> renameVertices(const std::vector<uint64_t> &vertexNames);


#include <iostream>

/**
 * @brief Represents a variable with an index and a boolean flag.
 *
 * This class maintains a static maximum index shared across all instances,
 * and each instance has its own index and boolean flag.
 */
class variable {
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

    static std::vector<variable*> registry;

public:
    /**
     * @brief Default constructor for the Variable class.
     *
     */
    variable() {
        myIndex = 0;
    }
    void activate(){
        if (myIndex > 0) return;
        myIndex = maxIndex++;
        registry.push_back(this);
    }
    static void init(){
        maxIndex = 1;
        registry.push_back(nullptr);
    }
    inline static variable* getvariableIndex(int i){
        return registry.at(i);
    }
    int getMyIndex(){
        return myIndex;
    }

};

int variable::maxIndex;
std::vector<variable*> variable::registry;

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

    std::unordered_set<uint64_t> seen;       // says weither a vertex has been seen yet (I want controlled order so I can't just have it here)
    BRAVE_DD::NodeHandle parentVertexHandle = qrbdd.getEdge().getNodeHandle();
    uint64_t uniqueVertexName = (static_cast<uint64_t>(0x4) << 48) | parentVertexHandle;
    std::vector<uint64_t> curLevelVerticies; // has all of the verticies on current level
    BRAVE_DD::NodeHandle curVertexHandle;

    //Rename verticies
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

    //initialize variable
    variable::init();
    //instanitiate all possible variables
    //start with belongs to
    variable belongsTo[numverticis][numStates];
    for (int alpha = 0; alpha < numverticis; alpha ++){
        for (int i = 0; i < numStates; i++){
            belongsTo[alpha][i] = variable();
        }
    }
    //then do delta function
    variable delta[numAssignments][numStates][numStates];
    for(int x = 0; x < numAssignments; x++){
        for (int i = 0; i < numStates; i ++){
            for (int j = 0; j < numStates; j ++){
                delta[x][i][j] = variable();
            }
        }
    }


    std::string function;
    int curVertex = 0;
    int numClauses = 0;


    // build sat problem
    // root(v_a) belongs to q0
    belongsTo[0][0].activate();
    function += std::to_string(belongsTo[0][0].getMyIndex()) + " 0\n";
    numClauses++;
    curVertex++;
    // each of the remaning vertecies can be in at least 1 of each state
    for (curVertex; curVertex < numverticis; curVertex++)
    {
        // incrementing the state
        for (int i = 0; i < numStatesG; i++)
        {
            belongsTo[curVertex][i].activate();
            function += std::to_string(belongsTo[curVertex][i].getMyIndex()) + " ";
        }
        function += "0\n";
        numClauses++;
    }
    std::cout << function << std::endl;

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