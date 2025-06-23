#include <iomanip>
#include "brave_dd.h"
#include "timer.h"

using namespace BRAVE_DD;

bool isReportToFile = 0;
bool isMT = 0;

std::string bddType;    // Terminal-encoding BDD forest, or Edge-valued BDD
std::string inpath;
std::string base;

long nodeNC = 0;
long nodeRes = 0;
long nodeOsm = 0;
long nodeTsm = 0;

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
    std::cout << "\t[Option]: " << "-m: Multi-terminal for terminal encoding BDDs" << std::endl;
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
            if (strcmp("-m", argv[i])==0) {
                isMT = 1;
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

    /* report nodes */
    out << "=========================| Node |=========================" << std::endl;
    out << std::left << std::setw(align) << "No Concretizing:" << nodeNC << std::endl;
    out << std::left << std::setw(align) << "Restrict:" << nodeRes << std::endl;
    out << std::left << std::setw(align) << "One-sided match:" << nodeOsm << std::endl;
    out << std::left << std::setw(align) << "Two-sided match:" << nodeTsm << std::endl;

    /* the end */
    out << "**********************************************************" << std::endl;
}

int main(int argc, const char** argv)
{
    /* Process arguments and initialize BDD forests */
    if (!processArgs(argc, argv)) return usage(argv[0]);
    /* Parser to read */
    FileReader FR(inpath.c_str());
    ParserPla parser(&FR);
    parser.readHeader();
    /* Initial forest */
    ForestSetting setting(bddType, parser.getInBits());
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

    DotMaker dot(forest, "EGT_BDD");
    dot.buildGraph(ans);
    dot.runDot("pdf");

    if (!evaluateEGT(ans, EGT)) {
        std::cout << "Test failed\n";
    } else {
        std::cout << "Test pass\n";
    }

    /* Concretizing */

    /* Report */
    if (!isReportToFile) {
        report(std::cout);
    } else {
        std::cout << "**********************************************************" << std::endl;
        std::string fileName = base;
        fileName += "_";
        if (isMT) fileName += "MT";
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
    return 0;
}