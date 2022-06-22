#include "ROBDD.hpp"
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
int yyparse();
ROBDD *T;
bool frontend_err = false;
int main(int argc, char **argv)
{
    // for command line args parsing
    bool all_sat_flag = 0, any_sat_flag = 0, sat_count_flag = 0;
    char *output_filename = nullptr;
    char flag;
    opterr = 0;

    // ref: https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
    while ((flag = getopt(argc, argv, "Ssco:")) != -1)
    {
        switch (flag)
        {
        case 's':
            any_sat_flag = 1;
            break;
        case 'S':
            all_sat_flag = 1;
            break;
        case 'c':
            sat_count_flag = 1;
            break;
        case 'o':
            output_filename = optarg;
            break;
        case '?':
            if (optopt == 'o')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            std::cerr << "Usage: ROBDD [-Ssc] [-o filename]" << std::endl;
            std::cerr << " -s\t\t- Check if the proposition is All-SAT" << std::endl;
            std::cerr << " -S\t\t- Check if the proposition is Any-SAT" << std::endl;
            std::cerr << " -c\t\t- Return SAT count for the proposition" << std::endl;
            std::cerr << " -o filename\t- Print ROBDD to filename.svg" << std::endl;
            return 1;
        default:
            abort();
        }
    }

    T = new ROBDD();
    yyparse();
    if (T->empty() or frontend_err)
    {
        std::cerr << "ROBDD construction failed" << std::endl;
        exit(-1);
    }

    if (all_sat_flag)
    {
        std::cout << "ALL SAT: ";
        if (T->SAT(all_sat_flag = true))
            std::cout << "True" << std::endl;
        else
            std::cout << "False" << std::endl;
    }
    if (any_sat_flag)
    {
        std::cout << "ANY SAT: ";
        if (T->SAT(all_sat_flag = false))
            std::cout << "True" << std::endl;
        else
            std::cout << "False" << std::endl;
    }
    if (sat_count_flag)
        std::cout << "SAT count: " << T->SATCOUNT() << std::endl;
    if (output_filename)
    {
        std::string filename = output_filename;
        std::ofstream out(filename + ".dot");
        T->output(out);
        out.close();
        if (!system(NULL))
        {
            std::cerr << "This OS does not support system() call, please convert output to image manually" << std::endl;
            std::cerr << ".DOT file written to " << filename << std::endl;
        }
        else
        {
            system(("dot " + filename + ".dot" + " -Tsvg -o " + filename + ".svg").c_str());
            system(("rm -f " + filename + ".dot").c_str());
            std::cout << "output written to " << filename << '.' << std::endl;
        }
    }
    return 0;
}