#include "out_bddx.h"
#include "../forest.h"

using namespace BRAVE_DD;
// ******************************************************************
// *                                                                *
// *                      BddxMaker methods                         *
// *                                                                *
// ******************************************************************

BddxMaker::BddxMaker(const Forest* f, const std::string bn) 
{
    basename = bn;
    std::string fName = basename + ".bddx";
    outfile.open(fName);
    if (!outfile) {
        std::cout << "[BRAVE_DD] Error!\t Could not open or create the file: "<< fName << std::endl;
        exit(1);
    }
    parent = f;
}
BddxMaker::~BddxMaker()
{
    //
}

void BddxMaker::buildBddx(const Func& func)
{
    //
}

void BddxMaker::buildBddx(const std::vector<Func>& func)
{
    // TODO: change this for MxDD
    Level numVars = parent->getSetting().getNumVars();
    char dim = parent->getSetting().isRelation() ? 2 : 1;
    std::string name = parent->getSetting().getName();
    std::string range;
    // TODO: discuss the names for ev diagrams
    if (parent->getSetting().getRangeType() == BOOLEAN) range = "BOOL";    
    std::vector<Edge> roots;
    for (Func fun: func) {
        roots.push_back(fun.getEdge());
    }
    char numChild = (parent->getSetting().isRelation()) ? 4 : 2;
    int numRules = parent->getSetting().getReductionSize();
    CompSet cs = parent->getSetting().getCompType();
    SwapSet ss = parent->getSetting().getSwapType();

    parent->unmark();
    /* start */
    outfile << "FOREST {\n";
    outfile << "\tTYPE " << name << "\n";
    // the forests are always reduced
    outfile << "\tREDUCED true\n"; 
    outfile << "\tLVLS " << numVars << " x " << static_cast<int>(dim) << "\n";
    outfile << "\tRANGE " << range << "\n";
    // TODO
    outfile << "\tNNUM " << parent->getNodeManUsed() << "\n";
    // TODO
    outfile << "\tRNUM " << roots.size() << "\n";
    outfile << "}\n";

    outfile << "NODES {\n";
    
    // Loop through and find all the nodes
    std::queue<Edge> frontier;
    std::vector<std::unordered_map<NodeHandle, bool>> visited(numVars+1);
    std::vector<std::vector<Edge>> edgeByLevel(numVars+1);
    for (Edge root:roots) {
        if (!visited[root.getNodeLevel()][root.getNodeHandle()]) {
            edgeByLevel[root.getNodeLevel()].push_back(root);
            visited[root.getNodeLevel()][root.getNodeHandle()] = true;
            if (root.getNodeLevel() != 0) {
                frontier.push(root);
            }
        }
    }
    while (frontier.size()) {
        Edge curr = frontier.front();
        frontier.pop();
        for (char j=0;j<numChild;j++) {
            Edge child = parent->getChildEdge(curr.getNodeLevel(), curr.getNodeHandle(), j);
            if (!visited[child.getNodeLevel()][child.getNodeHandle()]) {
                edgeByLevel[child.getNodeLevel()].push_back(child);
                visited[child.getNodeLevel()][child.getNodeHandle()] = true;
                if (child.getNodeLevel() != 0) {
                    frontier.push(child);
                }
            }
        }
    }
    for (Level i=1;i<=numVars;i++) {
        for (Edge curr: edgeByLevel[i]) {
            outfile << "\tN" << curr.getNodeHandle() << " L " << i << ": ";
            for (char j=0;j<numChild;j++) {
                Edge child = parent->getChildEdge(curr.getNodeLevel(), curr.getNodeHandle(), j);
                outfile << static_cast<int>(j) <<":";

                std::string label = "";
                label += "<";
                if (numRules > 0) {
                    ReductionRule rule = child.getRule();
                    label += ((rule == RULE_X) && !parent->getSetting().hasReductionRule(rule)) ? "N" : rule2String(rule);
                }
                if (cs != NO_COMP) {
                    label += (child.getComp()) ? ", c" : ", _";
                }
                if (ss != NO_SWAP) {
                    if (ss == ONE) {
                        label += (child.getSwap(0)) ? ", s_o" : ", _";
                    } else if (ss == ALL) {
                        label += (child.getSwap(0)) ? ", s_a" : ", _";
                    } else if (ss == FROM || ss == FROM_TO) {
                        label += (child.getSwap(0)) ? ", s_f" : ", _";
                    } else if (ss == TO) {
                        label += (child.getSwap(1)) ? ", s_t" : ", _";
                    }
                    if (ss == FROM_TO) {
                        label += (child.getSwap(1)) ? ", s_t" : ", _";
                    }
                }
                label += ",";
                if (child.getNodeLevel() == 0) label += "T" + unpackTermiValue(child.getEdgeHandle());
                else label += std::to_string(child.getNodeHandle());
                label += ">";

                outfile << label;
                if (j < numChild-1) outfile << ",";
                else outfile << "\n";
            }
        }
    }
    outfile << "}\n";
    outfile << "ROOTS {\n";
    for (Edge root:roots) {
        outfile << "\tr" << root.getNodeHandle() << " ";
        std::string label = "";
        label += "<";
        if (numRules > 0) {
            ReductionRule rule = root.getRule();
            label += ((rule == RULE_X) && !parent->getSetting().hasReductionRule(rule)) ? "N" : rule2String(rule);
        }
        if (cs != NO_COMP) {
            label += (root.getComp()) ? ", c" : ", _";
        }
        if (ss != NO_SWAP) {
            if (ss == ONE) {
                label += (root.getSwap(0)) ? ", s_o" : ", _";
            } else if (ss == ALL) {
                label += (root.getSwap(0)) ? ", s_a" : ", _";
            } else if (ss == FROM || ss == FROM_TO) {
                label += (root.getSwap(0)) ? ", s_f" : ", _";
            } else if (ss == TO) {
                label += (root.getSwap(1)) ? ", s_t" : ", _";
            }
            if (ss == FROM_TO) {
                label += (root.getSwap(1)) ? ", s_t" : ", _";
            }
        }
        label += ",";
        if (root.getNodeLevel() == 0) label += "T" + unpackTermiValue(root.getEdgeHandle());
        else label += std::to_string(root.getNodeHandle());
        label += ">";
        outfile << label << "\n";
    }
    outfile << "}\n";
}
