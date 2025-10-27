/*
    BRAVE_DD: Binary Reduction and Value on Edge Decision Diagram Library

    Copyright 2025, Lichuan Deng, Andrew S. Miner, Gianfranco Ciardo

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef BRAVE_DD_H
#define BRAVE_DD_H

#include "info.h"
#include "setting.h"
#include "function.h"
#include "forest.h"
#include "operators.h"
#include "operations/apply.h"
#include "IO/out_dot.h"
#include "IO/out_bddx.h"
#include "IO/parser.h"

namespace BRAVE_DD {
    inline const char* getLibInfo(int what)
    {
        static char* title = 0;
        switch (what) {
        case 0:
            if (!title) {
            title = new char[80];
            snprintf(title, 80,
    #ifdef DEVELOPMENT_CODE
                "%s version %s.dev",
    #else
                "%s version %s",
    #endif
                PROJECT_NAME, PROJECT_VERSION
            );
            }
            return title;

        case 1:
            return PROJECT_COPYRIGHT;

        case 2:
            return PROJECT_LICENSE;

        case 3:
            return PROJECT_URL;

        case 4:
            return "Data Structures and operations available:\n\
                    (1) BDDs: Union, Intersection, Difference.\n\
                    (2) Binary Matrix Diagrams (BMxDs): Union, Intersection, Difference.\n\
                    (3) Multi-Terminal BDDs (MTBDDs) with integer or real terminals:\n\
                        Arithmetic: Plus, Minus, Multiply, Divide, Min, Max.\n\
                        Logical: <, <=, >, >=, ==, !=.\n\
                        Conversion to and from BDDs.\n\
                    (4) Multi-Terminal BMxDs (MTBMxDs) with integer or real terminals:\n\
                        Arithmetic: Plus, Minus, Multiply, Divide, Min, Max.\n\
                        Logical: <, <=, >, >=, ==, !=.\n\
                        Conversion to and from BMxDs.\n\
                ";

        case 5:
            return PROJECT_DATE;
        }
        return 0;
    }
    inline void printInfo()
    {
        std::cout << "**************************************[" << PROJECT_NAME << "]**************************************" << std::endl;
        std::cout << "Version: \t" << PROJECT_VERSION << std::endl;
        std::cout << "Homepage: \t" << PROJECT_URL << std::endl;
        std::cout << "Copyright: \t" << PROJECT_COPYRIGHT << std::endl;
        std::cout << "License: \t" << PROJECT_LICENSE << std::endl;
        std::cout << "Last Update: \t" << PROJECT_DATE << std::endl;
        std::cout << "*************************************************************************************" << std::endl;
    }
}

#endif