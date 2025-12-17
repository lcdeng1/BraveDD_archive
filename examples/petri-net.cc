#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;
timer watch;
PredefForest setType, relType;

Forest *forest1; // Set
Forest *forest2; // Relation

std::string f_name;

uint16_t total;

std::string optype = "fix";

std::string trim(const std::string &str)
{
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
    {
        ++start;
    }

    if (start == str.size())
        return "";

    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end])))
    {
        --end;
    }

    return str.substr(start, end - start + 1);
}

std::unordered_map<uint16_t, Edge> cache;
Edge buildEdge(uint16_t lvl, std::vector<uint16_t> ins, std::vector<uint16_t> outs) {
    EdgeLabel label = 0;
    packRule(label, RULE_I0);
    bool in = std::find(ins.begin(), ins.end(), lvl) != ins.end();
    bool out = std::find(outs.begin(), outs.end(), lvl) != outs.end();
    if (cache.find(lvl) != cache.end()) {
        // std::cerr << "cache hit" << std::endl;
        return cache[lvl];
    }
    std::vector<Edge> child(4);
    // std::cerr << "current level: " << lvl << std::endl;
    if (lvl == 1) {
        if (in && out) {
            child[0].setEdgeHandle(makeTerminal(INT, 1));
            child[3].setEdgeHandle(makeTerminal(INT, 1));
            child[1].setEdgeHandle(makeTerminal(INT, 0));
            child[2].setEdgeHandle(makeTerminal(INT, 0));
        } else if (out) {
            child[2].setEdgeHandle(makeTerminal(INT, 1));
            child[0].setEdgeHandle(makeTerminal(INT, 0));
            child[1].setEdgeHandle(makeTerminal(INT, 0));
            child[3].setEdgeHandle(makeTerminal(INT, 0));  
        } else if (in) {
            child[1].setEdgeHandle(makeTerminal(INT, 1));
            child[0].setEdgeHandle(makeTerminal(INT, 0));
            child[2].setEdgeHandle(makeTerminal(INT, 0));
            child[3].setEdgeHandle(makeTerminal(INT, 0));
        } else {
            child[0].setEdgeHandle(makeTerminal(INT, 1));
            child[3].setEdgeHandle(makeTerminal(INT, 1));
            child[1].setEdgeHandle(makeTerminal(INT, 0));
            child[2].setEdgeHandle(makeTerminal(INT, 0));
        }
        child[0].setRule(RULE_X);
        child[1].setRule(RULE_X);
        child[2].setRule(RULE_X);
        child[3].setRule(RULE_X);
        Edge result = forest2->reduceEdge(lvl, label, lvl, child);
        cache[lvl] = result;
        return result;
    }
    if (in && out) {
        child[0] = buildEdge(lvl - 1, ins, outs);
        child[3] = buildEdge(lvl - 1, ins, outs);
        child[1].setEdgeHandle(makeTerminal(INT, 0));
        child[1].setRule(RULE_X);
        child[2].setEdgeHandle(makeTerminal(INT, 0));
        child[2].setRule(RULE_X);
    } else if (in) {
        child[2] = buildEdge(lvl - 1, ins, outs);
        child[0].setEdgeHandle(makeTerminal(INT, 0));
        child[0].setRule(RULE_X);
        child[1].setEdgeHandle(makeTerminal(INT, 0));
        child[1].setRule(RULE_X);
        child[3].setEdgeHandle(makeTerminal(INT, 0));  
        child[3].setRule(RULE_X);
    } else if (out) {
        child[1] = buildEdge(lvl - 1, ins, outs);
        child[0].setEdgeHandle(makeTerminal(INT, 0));
        child[0].setRule(RULE_X);
        child[2].setEdgeHandle(makeTerminal(INT, 0));
        child[2].setRule(RULE_X);
        child[3].setEdgeHandle(makeTerminal(INT, 0));
        child[3].setRule(RULE_X);
    } else {
        child[0] = buildEdge(lvl - 1, ins, outs);
        child[3] = buildEdge(lvl - 1, ins, outs);
        child[1].setEdgeHandle(makeTerminal(INT, 0));
        child[1].setRule(RULE_X);
        child[2].setEdgeHandle(makeTerminal(INT, 0));
        child[2].setRule(RULE_X);
    }
    Edge result = forest2->reduceEdge(lvl, label, lvl, child);
    cache[lvl] = result;
    return result;
}

Func buildTransition(uint16_t lvl, std::vector<uint16_t> ins, std::vector<uint16_t> outs) {
    Func res(forest2);
    Edge root = buildEdge(lvl, ins, outs);
    res.setEdge(root);
    return res;
}

bool SSG_Fixpoint(const Func& init, const std::vector<Func>& relations) {
    // DotMaker dot1(forest1, "fixpoint-init");
    // dot1.buildGraph(init);
    // dot1.runDot("pdf");
    std::cerr << "SSG Fixpoint" << std::endl;

    // S \gets S0
    Func s = init;
    // S' \gets \emptyset
    Func sp(forest1);
    sp.constant(0);
    uint64_t iter = 0;
    while (s.getEdge() != sp.getEdge()) {
        std::cout << "iteration: " << iter++ << std::endl;
        // S' \gets S
        sp = s;

        Func n(forest1);
        n.constant(0);
        for (Func relation: relations) {
            Func temp(forest1);
            apply(POST_IMAGE, s, relation, temp);
            n |= temp;
        }
        s |= n;
    }

    // std::cout << f_name << " , " << forest1->getNodeManPeak() << " , " << forest1->getNodeManUsed(curr) << std::endl;
    std::cerr << std::endl;
    std::cerr << "Peak memory usage: " << forest1->getNodeManPeak() << std::endl;
    forest1->markNodes(s);
    forest1->markSweep();
    std::cerr << "Final memory usage: " << forest1->getNodeManUsed(s) << std::endl;

    ForestSetting setting3(PredefForest::REXBDD, forest1->getSetting().getNumVars());
    Forest* forest3 = new Forest(setting3);

    Func comp(forest3);
    apply(COPY, s, comp);
    std::cerr << "Final Nodes: " << forest3->getNodeManUsed(comp) << std::endl;

    // DotMaker dot(forest1, "fixpoint");
    // dot.buildGraph(s);
    // dot.runDot("pdf");

    return 1;
}

void compute_saturation(const Func& target, const std::vector<Func>& relations)
{
    // DotMaker dot1(forest1, "saturation-init");
    // dot1.buildGraph(target);
    // dot1.runDot("pdf");
    Func states_Sat(forest1);
    // Timer start
    std::cerr << "Begin saturation" << std::endl;
    apply(SATURATE, target, relations, states_Sat);

    std::cout << forest1->getNodeManPeak() << " , " << forest1->getNodeManUsed(states_Sat) << std::endl;
    std::cerr << std::endl;
    std::cerr << "Saturation" << std::endl;
    std::cerr << "Peak Nodes: " << forest1->getNodeManPeak() << std::endl;
    forest1->markNodes(states_Sat);
    forest1->markSweep();
    std::cerr << "Final Nodes: " << forest1->getNodeManUsed(states_Sat) << std::endl;

    ForestSetting setting3(PredefForest::REXBDD, forest1->getSetting().getNumVars());
    Forest* forest3 = new Forest(setting3);

    Func comp(forest3);
    apply(COPY, states_Sat, comp);
    std::cerr << "Final Nodes: " << forest3->getNodeManUsed(comp) << std::endl;

    // DotMaker dot(forest1, "saturation");
    // dot.buildGraph(states_Sat);
    // dot.runDot("pdf");

}

bool SSG_Frontier(const Func& init, const std::vector<Func>& relations)
{
    // DotMaker dot1(forest1, "frontier-init");
    // dot1.buildGraph(init);
    // dot1.runDot("pdf");

    Func curr = init;
    Func front = init;
    int n = 0;
    std::cerr << "SSG Frontier" << std::endl;

    // std:: cout << "Is condition met? " << front.getEdge().isConstantZero() << std::endl;
    uint64_t iter = 0;
    while (!front.getEdge().isConstantZero()) {
        std::cout << "iteration: " << iter++ << std::endl;
        Func next(forest1);
        Func g(forest1);
        g.constant(0);
        for (Func relation : relations) {
            Func temp(forest1);
            apply(POST_IMAGE, front, relation, temp);
            g |= temp;
        }
        front = g & !curr;
        curr |= front;
        n++;
    }

    std::cerr << std::endl;
    std::cerr << "Peak nodes: " << forest1->getNodeManPeak() << std::endl;
    forest1->markNodes(curr);
    forest1->markSweep();
    std::cerr << "Final nodes: " << forest1->getNodeManUsed(curr) << std::endl;
    // DotMaker dot(forest1, "frontier");
    // dot.buildGraph(curr);
    // dot.runDot("pdf");

    ForestSetting setting3(PredefForest::REXBDD, forest1->getSetting().getNumVars());
    Forest* forest3 = new Forest(setting3);

    Func comp(forest3);
    apply(COPY, curr, comp);
    std::cerr << "Final Nodes: " << forest3->getNodeManUsed(comp) << std::endl;
    return 1;
}

bool SSG_Chaining(const Func& init, const std::vector<Func>& relations) {
    // DotMaker dot1(forest1, "chaining-init");
    // dot1.buildGraph(init);
    // dot1.runDot("pdf");

    Func s = init;
    Func sp(forest1);
    sp.constant(0);
    std::cerr << "SSG Chaining" << std::endl;
    uint64_t iter = 0;
    while (s.getEdge() != sp.getEdge()) {
        std::cout << "iteration: " << iter++ << std::endl;
        sp = s;
        for (Func relation: relations) {
            Func temp(forest1);
            apply(POST_IMAGE, s, relation, temp);
            s |= temp;
        }
    }
    std::cerr << std::endl;
    std::cerr << "Peak nodes: " << forest1->getNodeManPeak() << std::endl;
    forest1->markNodes(s);
    forest1->markSweep();
    std::cerr << "Final nodes: " << forest1->getNodeManUsed(s) << std::endl;

    ForestSetting setting3(PredefForest::REXBDD, forest1->getSetting().getNumVars());
    Forest* forest3 = new Forest(setting3);

    Func comp(forest3);
    apply(COPY, s, comp);
    std::cerr << "Final Nodes: " << forest3->getNodeManUsed(comp) << std::endl;

    // DotMaker dot(forest1, "chaining");
    // dot.buildGraph(s);
    // dot.runDot("pdf");
}

int main(int argc, const char **argv)
{
    setType = PredefForest::REXBDD;
    relType = PredefForest::IBMXD;


    if (strcmp(argv[1], "r") == 0)
    {
        setType = PredefForest::REXBDD;
    }
    else if (strcmp(argv[1], "u") == 0)
    {
        setType = PredefForest::UBDD;
    }
    else if (strcmp(argv[1], "f") == 0)
    {
        setType = PredefForest::FBDD;
    }
    else if (strcmp(argv[1], "z") == 0)
    {
        setType = PredefForest::ZBDD;
    }
    else if (strcmp(argv[1], "esr")== 0)
    {
        setType = PredefForest::ESRBDD;
    }
    else if (strcmp(argv[1], "cesr") == 0) 
    {
        setType = PredefForest::CESRBDD;
    }
    else if (strcmp(argv[1], "q") == 0) 
    {
        setType = PredefForest::QBDD;
    }
    else
    {
        std::cerr << "u for UBDD r for Rex" << std::endl;
        exit(0);
    }

    optype = std::string(argv[2]);


    std::ifstream file("../../petri-net.txt");
    if (!file)
    {
        std::cerr << "File not found bruh :(" << std::endl;
        exit(0);
    }

    std::string line;
    std::getline(file, line);

    // Getting the number of variable
    std::getline(file, line);
    // std::cerr << "line 67: " << line.substr(line.find(':') + 1) << std::endl;
    uint16_t levels = std::stoi(line.substr(line.find(':') + 1));
    total = levels;
    ForestSetting setting1(setType, levels);

    ForestSetting setting2(relType, levels);

    setting1.output(std::cerr);
    // setting2.output(std::cerr);

    forest1 = new Forest(setting1);
    forest2 = new Forest(setting2);

    // Now we will be reading in the initial marking
    Func res(forest1);
    res.constant(1);

    std::getline(file, line); // initialMarking
    std::getline(file, line); // this should be lvl
    while (line != "BDD-DONE")
    {
        uint16_t lvl = std::stoi(trim(line));
        Func temp(forest1);
        temp.variable(lvl);
        res &= temp;
        std::getline(file, line);
    }

    std::vector<Func> relations;

    std::getline(file, line); // should be RELATIONS
    std::getline(file, line); // This should be RELATION

    size_t counter = 0;
    while (trim(line) != "RELATIONS-DONE") {
        // if (counter % 10 == 0) std::cerr << "Building transition: " << counter++ << std::endl;
        counter++;
        // std::cerr << "Building transition: " << counter << std::endl;

        // std::cout << "The line is: " << line << std::endl; 
        std::getline(file, line); // This should be INPUTS
        // std::cerr << "line: " << line << std::endl;
        Func relation(forest2);
        std::vector<uint16_t> ins;
        std::vector<uint16_t> outs;

        std::getline(file, line); // This should be first lvl
        // std::cerr << "line: " << line << std::endl;

        while (trim(line) != "INPUTS-DONE") {
            // std::cout << "inputs line is: " << line << std::endl; 
            uint16_t lvl = std::stoi(trim(line));
            ins.push_back(lvl);
            std::getline(file, line);
            // std::cerr << "line: " << line << std::endl;
        }

        std::getline(file, line); // This should be outputs
        // std::cerr << "line: " << line << std::endl;
        std::getline(file, line); // This should be first lvl
        // std::cerr << "line: " << line << std::endl;

        while (trim(line) != "OUTPUTS-DONE") {
            // std::cout << "outputs line is: " << line << std::endl; 
            uint16_t lvl = std::stoi(trim(line));
            outs.push_back(lvl);
            std::getline(file, line); 
            // std::cerr << "line: " << line << std::endl;
        }

        // std::cerr << "NOW building relations" << std::endl;
        relation = buildTransition(forest2->getSetting().getNumVars(), ins, outs);
        // std::cerr << "DONE building relations" << std::endl;
        cache.clear();
        relations.push_back(relation);

        std::getline(file, line); // This should be RELATION-DONE
        // std::cerr << "line: " << line << std::endl;
        std::getline(file, line); // This should be RELATIONS-DONE
        // std::cerr << "line: " << line << std::endl;
        // std::cerr << "current line: " << line << std::endl;

    }
    std::cerr << "Done building relations" << std::endl;
    std::cerr << "Number of relations: " << relations.size() << std::endl;

    if (optype == "sat") {
        compute_saturation(res, relations);
    }

    if (optype == "fix") {
        SSG_Fixpoint(res, relations);
    }

    if (optype == "front") {
        SSG_Frontier(res, relations);
    }

    if (optype == "chain") {
        SSG_Chaining(res, relations);
    }

    std::cerr << "Done with" << f_name << std::endl;
    return 0;
}
