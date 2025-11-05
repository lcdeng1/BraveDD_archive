#include "out_bddx.h"
#include "../forest.h"
#include <unordered_map>

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
        std::cout << "[BRAVE_DD] Error!\t BddxMaker(): Could not open or create the file: "<< fName << std::endl;
        exit(1);
    }
    parent = f;
}
BddxMaker::~BddxMaker()
{
    //
}

void BddxMaker::makeHeader()
{
    outfile << "FOREST {\n";
    outfile << "\tTYPE " << parent->getSetting().getName() << "\n";
    outfile << "\tREDUCED true\n";
    outfile << "\tLVLS " << parent->getSetting().getNumVars() << " * " << (parent->getSetting().isRelation() ? 2 : 1) << "\n";
    outfile << "\tRANGE " << rangeType2String(parent->getSetting().getRangeType()) << "\n";
}

void BddxMaker::buildBddx(const Func& func)
{
    // header
    makeHeader();
    outfile << "\tNNUM " << parent->getNodeManUsed(func) << "\n";
    outfile << "\tRNUM 1\n";
    outfile << "}\n";

    // nodes map for global index
    struct pairHash
    {
        std::size_t operator()(const std::pair<Level, NodeHandle>& p) const noexcept {
            return (static_cast<std::size_t>(p.first) << 32) ^ p.second;
        }
    };
    std::unordered_map<std::pair<Level, NodeHandle>, uint64_t, pairHash> nodeMap(parent->getNodeManUsed(func));
    // nodes
    bool isMxd = parent->getSetting().isRelation();
    bool hasLevelInfo = parent->getSetting().getReductionSize() > 0;
    int numChild = (isMxd) ? 4 : 2;
    outfile << "NODES {\n";
    parent->markNodes(func);
    uint64_t nodeIndex = 1;
    for (Level l=1; l<=parent->getSetting().getNumVars(); l++) {
        for (uint32_t i=1; i<parent->nodeMan->chunks[l-1].firstUnalloc; i++) {
            Node node = parent->nodeMan->chunks[l-1].nodes[i];
            if (node.isMarked()) {
                // node header
                outfile << "\tN"<< nodeIndex << " L " << l << ": ";
                // save map
                nodeMap[{l, i}] = nodeIndex;
                nodeIndex++;
                // print child edges
                for (int c=0; c<numChild; c++) {
                    outfile << c << ":<" << rule2String(node.edgeRule(c, isMxd))
                            << "," << (int)node.edgeComp(c, isMxd)
                            << "," << (int)node.edgeSwap(c, 0, isMxd);
                    Level childLvl = (hasLevelInfo) ? node.childNodeLevel(c, isMxd) : l-1;
                    outfile << "," << ((childLvl == 0) ? "T" : "")
                            << ((childLvl == 0) ? node.childNodeHandle(c, isMxd) : nodeMap[{childLvl, node.childNodeHandle(c, isMxd)}])
                            << ">";
                    if (c < numChild-1) {
                        outfile << ",";
                    } else {
                        outfile << "\n";
                    }
                }
            }
        } // end for subnodeman
    } // end for level
    outfile << "}\n";

    // roots
    outfile << "ROOTS {\n";
    outfile << "\tr1 <" << rule2String(func.getEdge().getRule()) 
            << "," << (int)func.getEdge().getComp()
            << "," << (int)func.getEdge().getSwap(0)
            << "," << ((func.getEdge().getNodeLevel() == 0) ? "T" : "")
            << ((func.getEdge().getNodeLevel() == 0) ? func.getEdge().getNodeHandle() : nodeMap[{func.getEdge().getNodeLevel(), func.getEdge().getNodeHandle()}])
            << ">\n";
    outfile << "}\n";
    outfile.close();
}

void BddxMaker::buildBddx(const std::vector<Func>& func)
{
    // header
    makeHeader();
    outfile << "\tNNUM " << parent->getNodeManUsed(func) << "\n";
    outfile << "\tRNUM 1\n";
    outfile << "}\n";

    // nodes map for global index
    struct pairHash
    {
        std::size_t operator()(const std::pair<Level, NodeHandle>& p) const noexcept {
            return (static_cast<std::size_t>(p.first) << 32) ^ p.second;
        }
    };
    std::unordered_map<std::pair<Level, NodeHandle>, uint64_t, pairHash> nodeMap(parent->getNodeManUsed(func));
    // nodes
    bool isMxd = parent->getSetting().isRelation();
    bool hasLevelInfo = parent->getSetting().getReductionSize() > 0;
    int numChild = (isMxd) ? 4 : 2;
    outfile << "NODES {\n";
    parent->markNodes(func);
    uint64_t nodeIndex = 1;
    for (Level l=1; l<=parent->getSetting().getNumVars(); l++) {
        for (uint32_t i=1; i<parent->nodeMan->chunks[l-1].firstUnalloc; i++) {
            Node node = parent->nodeMan->chunks[l-1].nodes[i];
            if (node.isMarked()) {
                // node header
                outfile << "\tN"<< nodeIndex << " L " << l << ": ";
                // save map
                nodeMap[{l, i}] = nodeIndex;
                nodeIndex++;
                // print child edges
                for (int c=0; c<numChild; c++) {
                    outfile << c << ":<" << rule2String(node.edgeRule(c, isMxd))
                            << "," << (int)node.edgeComp(c, isMxd)
                            << "," << (int)node.edgeSwap(c, 0, isMxd);
                    Level childLvl = (hasLevelInfo) ? node.childNodeLevel(c, isMxd) : l-1;

                    outfile << "," << ((childLvl == 0) ? "T" : "")
                            << ((childLvl == 0) ? node.childNodeHandle(c, isMxd) : nodeMap[{childLvl, node.childNodeHandle(c, isMxd)}])
                            << ">";
                    if (c < numChild-1) {
                        outfile << ",";
                    } else {
                        outfile << "\n";
                    }
                }
            }
        } // end for subnodeman
    } // end for level
    outfile << "}\n";

    // roots
    outfile << "ROOTS {\n";
    for (size_t i=0; i<func.size(); i++) {
        outfile << "\tr" << i+1 << " <" << rule2String(func[i].getEdge().getRule()) 
            << "," << (int)func[i].getEdge().getComp()
            << "," << (int)func[i].getEdge().getSwap(0)
            << "," << ((func[i].getEdge().getNodeLevel() == 0) ? "T" : "")
            << ((func[i].getEdge().getNodeLevel() == 0) ? func[i].getEdge().getNodeHandle() : nodeMap[{func[i].getEdge().getNodeLevel(), func[i].getEdge().getNodeHandle()}])
            << ">\n";
    }
    outfile << "}\n";
    outfile.close();
}
