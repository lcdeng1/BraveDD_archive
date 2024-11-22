#include "node.h"

#include <random>
#include <iostream>
#include <cstdint>

const unsigned TESTS=1000000;
const unsigned SEED=123456;


std::mt19937 gen(SEED);     // Seed the Mersenne Twister engine with the fixed seed
std::uniform_int_distribution<> distrRule(0, 10);
std::uniform_int_distribution<> distrBool(0, 1);
std::uniform_int_distribution<> distrMarks(0, 3);
std::uniform_int_distribution<uint16_t> distr16(0, UINT16_MAX);
std::uniform_int_distribution<uint32_t> distr32(0, UINT32_MAX);
std::uniform_int_distribution<uint64_t> distr64(0, UINT64_MAX);

using namespace BRAVE_DD;

int main()
{
    std::cout << "Node test.\n\n";
    std::cout << "Size of Node: " << sizeof(Node) << " bytes\n";
    ForestSetting setting(10);

    Node node(setting);
    bool isMxd = setting.isRelation();
    
    for (unsigned i=0; i<TESTS; i++) {
        // next
        NodeHandle nxt = (NodeHandle)distr32(gen);
        node.setNext(nxt);
        if (node.getNext() != nxt) {
            std::cout << "[Next] error at i:" << i << std::endl;
            std::cout << "set Next: " << nxt << "; get Next: " << node.getNext() << std::endl;
            exit(1);
        }
        // mark
        const uint32_t mark = distrBool(gen);
        if (mark) node.mark();
        if (node.isMarked() != mark) {
            std::cout << "[Marks] error at i:" << i << std::endl;
            std::cout << "set Marks: " << mark << "; get Marks: " << node.isMarked() << std::endl;
            exit(1);
        }
        node.unmark();
        // rule
        ReductionRule rule = (ReductionRule)distrRule(gen);
        node.setEdgeRule(i%2, rule, isMxd);
        if (node.edgeRule(i%2, isMxd) != rule) {
            std::cout << "[Rule] error at i:" << i << std::endl;
            std::cout << "set rule: " << rule << "; get rule: " << node.edgeRule(i%2, isMxd) << std::endl;
            exit(1);
        }
        // handle
        NodeHandle handle = (NodeHandle)distr32(gen);
        node.setChildNodeHandle(i%2, handle, isMxd);
        if (node.childNodeHandle(i%2, isMxd) != handle) {
            std::cout << "[Handle] error at i:" << i << std::endl;
            std::cout << "set handle: " << handle << "; get handle: " << node.childNodeHandle(i%2, isMxd) << std::endl;
            exit(1);
        }
        // level
        uint16_t lvl = (uint16_t)distr16(gen);
        node.setChildNodeLevel(i%2, lvl, isMxd);
        if (node.childNodeLevel(i%2, isMxd) != lvl) {
            std::cout << "[Level] error at i:" << i << std::endl;
            std::cout << "set level: " << lvl << "; get lvl: " << node.childNodeLevel(i%2, isMxd) << std::endl;
            exit(1);
        }
        // complement
        bool comp = (bool)distrBool(gen);
        node.setEdgeComp(1, comp, isMxd);
        if (node.edgeComp(1, isMxd) != comp) {
            std::cout << "[Complement] error at i:" << i << std::endl;
            std::cout << "set comp: " << comp << "; get comp: " << node.edgeComp(1, isMxd) << std::endl;
            exit(1);
        }
        // swap
        bool swap = (bool)distrBool(gen);
        node.setEdgeSwap(i%2, 0, swap, isMxd);
        if (node.edgeSwap(i%2, 0, isMxd) != swap) {
            std::cout << "[Swap] error at i:" << i << std::endl;
            std::cout << "set swap: " << swap << "; get swap: " << node.edgeSwap(i%2, 0, isMxd) << std::endl;
            exit(1);
        }
    }


    std::cout << TESTS << " tests passed!" << std::endl;
    return 0;
}