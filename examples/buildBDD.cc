#include "/home/dara/Git/brave_dd/src/brave_dd.h"
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace BRAVE_DD;

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
    int getVarType()
    {
        return varTypeG;
    }
    int getMaxIndex()
    {
        return maxIndex;
    }
    std::vector<variable *> getRegistry()
    {
        return registry;
    }
};
int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments, std::string name);
std::unordered_map<uint64_t, int> renameVertices(const std::vector<uint64_t> &vertexNames);
std::string variableName(variable var);
variable getVariableByValue(int index);
std::string intToBoolVariable(variable var);
std::string intToAlphaString(int number);

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

    qRBDDToBoolForDFA(result, 4, 2, "Func2");

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
int qRBDDToBoolForDFA(BRAVE_DD::Func qrbdd, int numStates, int numAssignments, std::string name)
{
    numStatesG = numStates;

    std::unordered_set<uint64_t> seen; // says weither a vertex has been seen yet (I want controlled order so I can't just have it here)
    BRAVE_DD::NodeHandle parentVertexHandle = qrbdd.getEdge().getNodeHandle();
    uint64_t uniqueVertexName = (static_cast<uint64_t>(0x4) << 48) | parentVertexHandle;
    std::vector<uint64_t> curLevelVerticies; // has all of the verticies on current level
    BRAVE_DD::NodeHandle curVertexHandle;
    curLevelVerticies.push_back(uniqueVertexName);
    seen.insert(uniqueVertexName);
    variable tempVar;

    // Rename verticies
    std::vector<uint64_t> allVerticies;
    allVerticies.push_back(uniqueVertexName);
    int verticiesOnLvl = curLevelVerticies.size();
    for (int lvl = maxLvl - 1; lvl >= 0; lvl--)
    {
        for (verticiesOnLvl; verticiesOnLvl > 0; verticiesOnLvl--)
        {
            // std::cout << std::hex << "Result: 0x" << curLevelVerticies.front() << std::endl;
            parentVertexHandle = curLevelVerticies.front();
            for (int x = 0; x < numAssignments; x++)
            {
                // get handle of the vertecy
                curVertexHandle = qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x);
                // create a unique name from handle and level
                uniqueVertexName = static_cast<uint64_t>(lvl) << 48 | curVertexHandle;
                // std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
                //  check if seen befor and add it to seen if not
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
            belongsTo[alpha][i] = variable(v_AlphBelongsToq_i, alpha, i);
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
                deltaFun[i][j][x] = variable(deltaq_iXIsq_j, i, j, x);
                // std::cout << variableName(deltaFun[i][j][x]) << std::endl;
                //  delta[i][j][x].activate();
            }
        }
    }

    std::string function;
    std::string tempFunction;
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
    // std::cout << std::hex << "Result: 0x" << uniqueVertexName << std::endl;
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
                // std::cout << variableName(deltaFun[j][i][x]) << std::endl;
                //  printf("%d\n",delta[x][i][j].getMyIndex());
                function += std::to_string(deltaFun[j][i][x].getMyIndex()) + " ";
            }
            function += "0\n";
            numClauses++;
        }
        // std::cout << function << std::endl;
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
                    // std::cout << variableName(deltaFun[i][jPrime][x]) << std::endl;
                    function += "-" + std::to_string(deltaFun[i][jPrime][x].getMyIndex()) + " ";
                    deltaFun[i][jSub][x].activate();
                    // std::cout << variableName(deltaFun[i][jSub][x]) << std::endl;
                    function += "-" + std::to_string(deltaFun[i][jSub][x].getMyIndex()) + " ";
                    function += "0\n";
                    // std::cout << function << std::endl;
                    numClauses++;
                }
            }
            // std::cout << function << std::endl;
        }
    }

    /*// notes for using renamed
    for (const auto& [name, id] : renamed) {
        std::cout << "Vertex " << name << " -> ID " << id << "\n";
        std::cout << "TestID Retrival " << name << " -> ID " << renamed.at(name) << "\n";
    }*/

    // make sure the DFA agrees with BDD
    // set up base
    // get children
    seen.clear();
    curLevelVerticies.clear();
    parentVertexHandle = qrbdd.getEdge().getNodeHandle();
    uniqueVertexName = (static_cast<uint64_t>(0x4) << 48) | parentVertexHandle;
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
                // check if seen befor and add it to seen if not
                if (seen.insert(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x)).second)
                {
                    // add to the stack
                    curLevelVerticies.push_back(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x));
                }
            }

            auto parent = renamed.at(curLevelVerticies.front());
            tempFunction = "-" + std::to_string(belongsTo[parent][0].getMyIndex()) + " ";
            curLevelVerticies.erase(curLevelVerticies.begin());
            for (int i = 0; i < numStates; i++)
            {
                for (int x = 0; x < numAssignments; x++)
                {
                    for (int j = 0; j < numStates; j++)
                    {
                        belongsTo[parent][i].activate();
                        tempFunction = "-" + std::to_string(belongsTo[parent][i].getMyIndex()) + " ";
                        function += tempFunction;
                        std::cout << variableName(deltaFun[i][j][x]) << std::endl;
                        function += "-" + std::to_string(deltaFun[i][j][x].getMyIndex()) + " ";
                        // std::cout << (static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x)) << std::endl;
                        std::cout << variableName(belongsTo[renamed.at(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x))][j]) << "\n"
                                  << std::endl;
                        function += std::to_string(belongsTo[renamed.at(static_cast<uint64_t>(lvl) << 48 | qrbdd.getForest()->getChildNodeHandle(lvl + 1, parentVertexHandle, x))][j].getMyIndex()) + " 0\n";
                        numClauses++;
                    }
                    // std::cout << function << std::endl;
                }
                // std::cout << function << std::endl;
            }
        }
        verticiesOnLvl = curLevelVerticies.size();
    }

    // each state is either final or non final.

    function = "p cnf " + std::to_string(deltaFun[0][0][0].getMaxIndex() - 1) + " " + std::to_string(numClauses) + "\n" + function;

    // add p cnf <num variables> <num clauses> to tope of file
    std::ofstream outFile("satFunctionForQRBDDtoDFA" + name + ".txt");
    if (!outFile)
    {
        std::cerr << "Error opening file: " << "satFunctionForQRBDDtoDFA" + name + ".txt" << std::endl; // O(1)
        return -1;
    }

    outFile << function; // O(n) — writes each character of the string
    outFile.close();     // O(1) — flushes and closes the file

    std::ifstream inFile("satFunctionForQRBDDtoDFA" + name + ".txt");
    if (!inFile)
    {
        std::cerr << "Error opening file: " << "satFunctionForQRBDDtoDFA" + name + ".txt" << std::endl; // O(1)
        return -1;
    }

    std::string line;
    // Loop over each line — O(m), where m is the number of lines
    std::getline(inFile, line);
    std::string latexFunction = "";
    while (std::getline(inFile, line))
    {
        std::istringstream lineStream(line); // O(1)
        std::string word;
        bool isFirst = true;

        // Loop over each word in the line — O(k), where k is the number of words in the line
        while (lineStream >> word && word != "0")
        {
            if (!isFirst)
            {
                latexFunction += "\\vee ";
                std::cout << "\\vee ";
            }
            std::cout << word << std::endl;
            if (std::stoi(word) < 0)
            {
                latexFunction += "\\neg " + variableName(getVariableByValue(abs(std::stoi(word)))) + " ";
                std::cout << "\\neg " + variableName(getVariableByValue(abs(std::stoi(word)))) << " ";
            }
            else
            {
                latexFunction += variableName(getVariableByValue(abs(std::stoi(word)))) + " ";
                std::cout << variableName(getVariableByValue(abs(std::stoi(word)))) << " ";
            }
            // O(1)
            isFirst = false;
        }
        latexFunction += "\\wedge \\\\ \n";
        std::cout << "\\wedge \\\\" << std::endl;
    }

    inFile.close(); // O(1)*/

    std::ofstream outFileLatx("satFunctionForQRBDDtoDFALatex" + name + ".txt");
    if (!outFileLatx)
    {
        std::cerr << "Error opening file: " << "satFunctionForQRBDDtoDFALatex" + name + ".txt" << std::endl; // O(1)
        return -1;
    }

    outFileLatx << latexFunction; // O(n) — writes each character of the string
    outFileLatx.close();          // O(1) — flushes and closes the file

    std::string command = "/home/dara/Git/kissat/build/kissat ";
    std::string outputFile = "kissat_output" + name + ".txt";
    command += "/home/dara/Git/brave_dd/build/examples/satFunctionForQRBDDtoDFA" + name + ".txt > " + outputFile;
    std::cout << "Executing: " << command << std::endl;

    int result = std::system(command.c_str());

    std::ifstream fileKissat("/home/dara/Git/brave_dd/build/examples/kissat_output" + name + ".txt");

    // Check if the file was successfully opened
    if (!fileKissat.is_open())
    {
        std::cerr << "Error: Could not open file '/home/dara/Git/brave_dd/build/examples/kissat_output" + name + ".txt'\n";
        return -1;
    }

    bool foundSatisfiable = false;
    bool stop = false;

    // prep DFA format
    std::string DFAFormat = "DFA" + name + "\n";
    DFAFormat += "STATES " + std::to_string(numStates) + "\n";
    DFAFormat += "ALPHABET a";
    for (int i = 1; i < numAssignments; i++)
    {
        DFAFormat += "," + intToAlphaString(i);
    }
    DFAFormat += "\n";
    DFAFormat += "INITIAL 0\n";
    DFAFormat += "FINAL ";

    bool isFirstFinalDFAState = true;
    bool isFirstDelta = true;

    // Read the file line by line
    while (std::getline(fileKissat, line))
    {
        if (stop)
            break;
        if (!foundSatisfiable)
        {
            // Look for the line containing "SATISFIABLE"
            if (!line.empty() && line[0] == 's')
            {
                foundSatisfiable = true;
            }
        }
        else
        {
            // Process lines after "SATISFIABLE"
            std::istringstream iss(line);
            std::string num;
            while (iss >> num)
            {
                if (num == "v")
                {
                }
                else if (std::stoi(num) == 0)
                {
                    stop = true;
                    break;
                }
                else if (std::stoi(num) > 0)
                {
                    std::cout << num << std::endl;
                    std::cout << variableName(getVariableByValue(std::stoi(num))) << std::endl;
                    tempVar = getVariableByValue(std::stoi(num));
                    switch (tempVar.getVarType())
                    {
                    case v_AlphBelongsToq_i:
                        if (tempVar.alpha == numverticis)
                        {
                            if (isFirstFinalDFAState == true)
                            {
                                isFirstFinalDFAState = false;
                                DFAFormat += std::to_string(tempVar.i);
                            }
                            else
                            {
                                DFAFormat += ", " + std::to_string(tempVar.i);
                            }
                        }
                        break;
                    case deltaq_iXIsq_j:
                        if (isFirstDelta == true)
                        {
                            DFAFormat += "\n";
                            isFirstFinalDFAState = false;
                            DFAFormat += std::to_string(tempVar.i);
                        }
                        else
                        {
                            DFAFormat += std::to_string(tempVar.i);
                        }
                        break;
                    }
                    // std::cout << registry.at(std::stoi(num)) << std::endl;
                }
            }
        }
    }

    std::cout << std::endl;

    fileKissat.close();

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

std::string variableName(variable var)
{
    switch (var.getVarType())
    {
    case v_AlphBelongsToq_i:
        return "v_" + intToAlphaString(var.alpha) + " \\relSym\\q_" + std::to_string(var.i);
        break;
    case deltaq_iXIsq_j:
        return "\\delta (q_" + std::to_string(var.i) + " ," + std::to_string(var.x) + ") = q_" + std::to_string(var.j);
        break;
    }
}

/**
 * @brief Retrieves a copy of the variable object at a given index.
 * @param index Index in the registry vector.
 * @return A copy of the variable object.
 * @note Time Complexity: O(1) - Direct access via std::vector::at
 */
variable getVariableByValue(int index)
{
    variable *ptr = variable::getvariableIndex(index); // O(1)
    if (ptr == nullptr)
    {
        throw std::runtime_error("No variable at this index."); // O(1)
    }
    return *ptr; // O(1)
}