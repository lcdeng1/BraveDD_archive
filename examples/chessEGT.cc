#include <iomanip>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;

bool isReportToFile = 0;
bool isMT = 0;
bool isMR = 0;

std::string bddType;    // Terminal-encoding BDD forest, or Edge-valued BDD
std::string inpath;
std::string base;

long nodeNC = 0;
long nodeRes = 0;
long nodeOsm = 0;
long nodeTsm = 0;
long nodeDC = 0;
unsigned long numFun = 0;

// for MR
std::vector<Func> table;
std::vector<int> outcomes;
std::vector<long> distribution_state;
std::vector<long> distribution_node;

int usage(const char* who)
{
    /* Strip leading directory, if any: */
    const char* name = who;
    for (const char* ptr=who; *ptr; ptr++) {
        if ('/' == *ptr) name = ptr+1;
    }
    std::cout << "Usage: " << name << " -i [EGT_INPUT] -b [BDD_TYPE] [Option]\n" << std::endl;
    std::cout << std::left << std::setw(15);
    std::cout << "\t[EGT_INPUT]: " << "the EGT file" << std::endl;
    std::cout << std::left << std::setw(15);
    std::cout << "\t[BDD_TYPE]: " << "the type of BDD to encode" << std::endl;
    std::cout << std::left << std::setw(15);
    std::cout << "\t[Option]: " << "-mt: Multi-terminal for terminal encoding BDDs" << std::endl;
    std::cout << std::left << std::setw(15);
    std::cout << "\t " << "-mr: Multi-root" << std::endl;
    std::cout << std::left << std::setw(15);
    std::cout << "\t " << "-f: Report to files" << std::endl;

    return 1;
}

bool processArgs(int argc, const char** argv)
{
    // default types
    bddType = "FBDD";

    bool setType = 0, setInput = 0;
    for (int i=1; i<argc; i++) {
        if ('-' == argv[i][0]) {
            // options other than size
            if (strcmp("-b", argv[i])==0) {
                bddType = argv[i+1];
                setType = 1;
                i++;
                continue;
            }
            if (strcmp("-i", argv[i])==0) {
                inpath = argv[i+1];
                setInput = 1;
                i++;
                continue;
            }
            // option
            if (strcmp("-mt", argv[i])==0) {
                isMT = 1;
                continue;
            }
            if (strcmp("-mr", argv[i])==0) {
                isMR = 1;
                continue;
            }
            if (strcmp("-f", argv[i])==0) {
                isReportToFile = 1;
                continue;
            }
        }
    }
    if ((!setType) || (!setInput)) return 0;
    size_t lastSlash = inpath.find_last_of("/\\");
    std::string fileName = inpath.substr(lastSlash+1);
    size_t pos = fileName.find('.');
    base = (pos != std::string::npos) ? fileName.substr(0, pos) : fileName;
    return 1;
}

bool evaluateEGT(Func& bdd, ExplictFunc& EGT)
{
    bool isPass = 1;

    std::vector<bool> assignment;
    Value bddVal;

    for (size_t i=0; i<EGT.size(); i++) {
        assignment = EGT.getAssignment(i);
        assignment.insert(assignment.begin(), 0);
        bddVal = bdd.evaluate(assignment);
        if (bddVal != EGT.getOutcome(i)) {
            for (size_t n=1; n<assignment.size(); n++) {
                std::cout << assignment[n] << " ";
            }
            std::cout << std::endl;
            int val;
            bddVal.getValueTo(&val, INT);
            std::cout << "bdd value: " << val << std::endl;
            EGT.getOutcome(i).getValueTo(&val, INT);
            std::cout << "outcome value: " << val << std::endl;
            isPass = 0;
            break;
        }
    }

    return isPass;
}

void report(std::ostream& out)
{
    int align = 30;
    /* report balance */
    out << "=========================| File |=========================" << std::endl;
    out << std::left << std::setw(align);
    out << "Balance:" << base << std::endl;
    out << std::left << std::setw(align);
    out << "#States:" << numFun << std::endl;
    out << std::left << std::setw(align);
    out << "Outcomes:";
    out << "[";
    for (size_t i=0; i<outcomes.size(); i++) {
        out << outcomes[i];
        if (i == (outcomes.size()-1)) {
            out << "]";
        } else {
            out << " ";
        }
    }
    out << std::endl;
    out << std::left << std::setw(align);
    out << "BDD:" << bddType << std::endl;
    /* report nodes */
    out << "=========================| Node |=========================" << std::endl;
    out << std::left << std::setw(align) << "No Concretizing:" << nodeNC << std::endl;
    out << std::left << std::setw(align) << "Restrict:" << nodeRes << std::endl;
    out << std::left << std::setw(align) << "One-sided match:" << nodeOsm << std::endl;
    out << std::left << std::setw(align) << "Two-sided match:" << nodeTsm << std::endl;
    // out << std::left << std::setw(align) << "Don't-care:" << nodeDC << std::endl;
    if (isMR) {
        out << "====================| Distribution |======================" << std::endl;
        out << std::left << std::setw(15) << "Outcome" << std::left << std::setw(15) << "#state" << std::left << std::setw(15) << "#node" << std::endl;
        for (size_t i=0; i<outcomes.size(); i++) {
            out << std::left << std::setw(15) << outcomes[i]
            << std::left << std::setw(15) << distribution_state[i] 
            << std::left << std::setw(15) << distribution_node[i] << std::endl;
        }
        out << "----------------------------------------------------------" << std::endl;
        out << std::left << std::setw(15) << "Don't-care"
            << std::left << std::setw(15) << distribution_state.back() 
            << std::left << std::setw(15) << distribution_node.back() << std::endl;
    }

    /* the end */
    out << "**********************************************************" << std::endl;
}

void build()
{
    /* Parser to read */
    ParserPla parser(inpath);
    parser.readHeader();
    numFun = parser.getNum();
    /* Initial forest */
    ForestSetting setting(bddType, parser.getInBits());
    bddType = setting.getName();
    if (isMT) bddType += "_MT";
    if (setting.getEncodeMechanism() != TERMINAL) {
        setting.setMaxRange(5);
    }
    setting.setValType(INT);
    setting.output(std::cout);
    Forest* forest = new Forest(setting);

    /* ExplictFunc to store */
    ExplictFunc EGT;
    std::vector<bool> assignment(parser.getInBits());
    Value outcome;
    int maxOC = 0;
    for (;;) {
        char oc;
        if (!parser.readAssignment(assignment, oc)) break;
        // register outcome value
        if (std::find(outcomes.begin(), outcomes.end(), (int)(oc-'0')) == outcomes.end()) {
            outcomes.push_back((int)(oc-'0'));
        }
        if ((int)(oc-'0') > maxOC) maxOC = (int)(oc-'0');
        // value mapping, TBD
        outcome.setValue((int)(oc-'0'), INT);
        EGT.addAssignment(assignment, outcome);
    }
    std::cout << "Max outcome: " << maxOC << std::endl;


    /* Build BDD */
    EGT.setDefaultValue(Value(SpecialValue::POS_INF));  // set default value
    // EGT.setDefaultValue(Value(0));
    std::cout<<"build function\n";
    Func ans = EGT.buildFunc(forest);
    // apply(MINIMUM, ans, EGT, ans);
    nodeNC = forest->getNodeManUsed(ans);

    // DotMaker dot(forest, "EGT_BDD");
    // dot.buildGraph(ans);
    // dot.runDot("pdf");

    if (!evaluateEGT(ans, EGT)) {
        std::cout << "Test failed\n";
    } else {
        std::cout << "Test pass\n";
    }

    // build don't-care set
    // Func all(forest);
    // all.constant(1);
    // Func DC(forest);
    // DC = all ^ ans;
    // nodeDC = forest->getNodeManUsed(DC);

    /* Concretizing */
    // TBD

    /* Report */
    if (!isReportToFile) {
        report(std::cout);
    } else {
        std::cout << "**********************************************************" << std::endl;
        std::string fileName = base;
        fileName += "_";
        if (isMT) fileName += "MT";
        else if (isMR) fileName += "MR";
        fileName += forest->getSetting().getName();
        fileName += ".txt";
        std::ofstream file(fileName, std::ios::app);
        if (!file) {
            std::cerr << "Failed to open file " << fileName << std::endl;
        } else {
            report(file);
            file.close();
        }
        std::cout << "Done!" << std::endl;
    }
    delete forest;
}

void buildMR()
{
    /* Parser to read */
    ParserPla parser(inpath);
    parser.readHeader();
    numFun = parser.getNum();
    /* Initial forest */
    ForestSetting setting(bddType, parser.getInBits());
    bddType = setting.getName();
    bddType += "_MR";
    if (setting.getEncodeMechanism() != TERMINAL) {
        setting.setMaxRange(5);
    }
    setting.setValType(INT);
    setting.output(std::cout);
    Forest* forest = new Forest(setting);

    /* ExplictFunc to store */
    std::vector<ExplictFunc> EGTs;
    ExplictFunc EGT;
    std::vector<bool> assignment(parser.getInBits());
    Value outcome(1);
    int maxOC = 0;
    for (;;) {
        char oc;
        if (!parser.readAssignment(assignment, oc)) break;
        // register outcome value
        if (std::find(outcomes.begin(), outcomes.end(), (int)(oc-'0')) == outcomes.end()) {
            outcomes.push_back((int)(oc-'0'));
            EGTs.push_back(EGT);
        }

        if ((int)(oc-'0') > maxOC) maxOC = (int)(oc-'0');
        // value mapping, TBD
        // outcome.setValue((int)(oc-'0'), INT);
        // which EGT to add
        auto it = std::find(outcomes.begin(), outcomes.end(), (int)(oc-'0'));
        EGTs[std::distance(outcomes.begin(), it)].addAssignment(assignment, outcome);
    }
    std::cout << "Max outcome: " << maxOC << std::endl;


    /* Build MR BDD */
    long num_state = 0;
    for (size_t i=0; i<EGTs.size(); i++) {
        // EGT.setDefaultValue(Value(SpecialValue::POS_INF));  // set default value
        EGTs[i].setDefaultValue(Value(0));
        std::cout<<"build function " << i << ", outcome: " << outcomes[i] << std::endl;
        table.push_back(EGTs[i].buildFunc(forest));
        // evaluation, optional
        if (!evaluateEGT(table[i], EGTs[i])) {
            std::cout << "Test failed\n";
            break;
        } else {
            std::cout << "Test pass\n";
        }
        apply(CARDINALITY, table[i], num_state);
        distribution_state.push_back(num_state);
        distribution_node.push_back(forest->getNodeManUsed(table[i]));
    }
    nodeNC = forest->getNodeManUsed(table);

    // DotMaker dot(forest, "EGT_BDD_MR");
    // dot.buildGraph(table);
    // dot.runDot("pdf");

    // build Don't-care set
    Func DC(forest);
    DC.constant(1);
    for (size_t i=0; i<EGTs.size(); i++) {
        DC = DC & !table[i];
    }
    apply(CARDINALITY, DC, num_state);
    distribution_state.push_back(num_state);
    distribution_node.push_back(forest->getNodeManUsed(DC));

    /* Concretizing */
    //TBD

    /* Report */
    if (!isReportToFile) {
        report(std::cout);
    } else {
        std::cout << "**********************************************************" << std::endl;
        std::string fileName = base;
        fileName += "_";
        if (isMT) fileName += "MT";
        else if (isMR) fileName += "MR";
        fileName += forest->getSetting().getName();
        fileName += ".txt";
        std::ofstream file(fileName, std::ios::app);
        if (!file) {
            std::cerr << "Failed to open file " << fileName << std::endl;
        } else {
            report(file);
            file.close();
        }
        std::cout << "Done!" << std::endl;
    }


    delete forest;
}

int main(int argc, const char** argv)
{
    /* Process arguments and initialize BDD forests */
    if (!processArgs(argc, argv)) return usage(argv[0]);
    
    if (!isMR) build();
    else buildMR();
    return 0;
}