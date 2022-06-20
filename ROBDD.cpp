#include "ROBDD.hpp"
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <stdio.h>
int yyparse();
ROBDD *T;
ROBDD::ROBDD()
{
    this->zero = new BDD_node({0, nullptr, nullptr});
    this->one = new BDD_node({1, nullptr, nullptr});
    this->node_table[*(this->zero)] = this->zero;
    this->node_table[*(this->one)] = this->one;
}

unsigned int ROBDD::get_ID(std::string var)
{
    // new var
    if (this->var_to_ID.find(var) == this->var_to_ID.end())
        this->var_to_ID[var] = avail_ID++;
    return this->var_to_ID[var];
}

BDD_node *ROBDD::make_node(unsigned int ID, BDD_node *low, BDD_node *high)
{
    if (low == high)
        return low;
    auto found = this->node_table.find(BDD_node{ID, low, high});
    if (found != this->node_table.end())
        return found->second;
    else
    {
        BDD_node *new_node = new BDD_node({ID, low, high});
        this->node_table[*new_node] = new_node;
        return new_node;
    }
}

BDD_node *ROBDD::calc(binary_op op, BDD_node *left, BDD_node *right)
{
    switch (op)
    {
    case OP_AND:
        if (left->var == 1 and right->var == 1)
            return this->one;
        else
            return this->zero;
        break;
    case OP_OR:
        if (left->var == 1 or right->var == 1)
            return this->one;
        else
            return this->zero;
        break;
    case OP_THEN:
        if (left->var == 0 or right->var == 1)
            return this->one;
        else
            return this->zero;
        break;
    }
    return nullptr;
}

BDD_node *ROBDD::apply(binary_op op, BDD_node *left, BDD_node *right)
{
    return this->_apply(op, left, right);
    this->apply_table[op].clear();
}

BDD_node *ROBDD::_apply(binary_op op, BDD_node *left, BDD_node *right)
{
    auto found = this->apply_table[op].find(std::pair<BDD_node *, BDD_node *>(left, right));
    BDD_node *res;
    if (found != this->apply_table[op].end())
        res = found->second;
    else if (left->var <= 1 and right->var <= 1)
        res = this->calc(op, left, right);
    else if (left->var == right->var) // promote the shared var
    {
        auto l = this->_apply(op, left->low, right->low);
        auto r = this->_apply(op, left->high, right->high);
        res = this->make_node(left->var, l, r);
    }
    else if ((left->var < right->var and left->var > 1) or right->var <= 1) // unfold left BDD tree
    {
        auto l = this->_apply(op, left->low, right);
        auto r = this->_apply(op, left->high, right);
        res = this->make_node(left->var, l, r);
    }
    else if ((left->var > right->var and right->var > 1) or left->var <= 1) // unfold right BDD tree
    {
        auto l = this->_apply(op, left, right->low);
        auto r = this->_apply(op, left, right->high);
        res = this->make_node(right->var, l, r);
    }
    this->apply_table[op][std::pair<BDD_node *, BDD_node *>(left, right)] = res;
    return res;
}

void ROBDD::set_root(BDD_node *node)
{
    this->root = node;
}

bool ROBDD::_SAT(BDD_node *node, bool all_sat = false)
{
    if (node->var == 1)
        return true;
    else if (node->var == 0)
        return false;
    if (all_sat)
        return this->_SAT(node->high, true) & this->_SAT(node->low, true);
    else
        return this->_SAT(node->high) | this->_SAT(node->low);
}

bool ROBDD::SAT(bool all_sat = false)
{
    return this->_SAT(this->root, all_sat);
}

unsigned int ROBDD::_SATCOUNT(BDD_node *node)
{
    if (node->var == 1)
        return 1;
    else if (node->var == 0)
        return 0;
    return this->_SATCOUNT(node->high) | this->_SATCOUNT(node->low);
}

unsigned int ROBDD::SATCOUNT()
{
    return this->_SATCOUNT(this->root);
}

void ROBDD::output(std::ofstream &out)
{
    this->printed.clear();
    for (auto it : this->var_to_ID)
        this->ID_to_var[it.second] = it.first;
    out << "digraph ROBDD {\n"
        << "fontname=\"Helvetica,Arial,sans-serif\"\n"
        << "node [fontname=\"Helvetica,Arial,sans-serif\"]\n"
        << "edge [fontname=\"Helvetica,Arial,sans-serif\"]\n"
        << "node [shape=circle];\n";
    this->_output(this->root, out);
    out << "}";
    this->printed.clear();
}

void ROBDD::_output(BDD_node *node, std::ofstream &out)
{
    out << "\"";
    if (node->var > 1)
        out << this->ID_to_var[node->var];
    else
        out << node->var;
    out << "\"" << std::endl;
    this->printed.insert(*node);
    if (node->var <= 1)
        return;
    else
    {
        out << "\"";
        out << this->ID_to_var[node->var];
        out << "\"";
        out << " -> ";
        out << "\"";
        if (node->low->var > 1)
            out << this->ID_to_var[node->low->var];
        else
            out << node->low->var;
        out << "\""
            << "[style = dotted]" << std::endl;
        out << "\"";
        out << this->ID_to_var[node->var];
        out << "\"";
        out << " -> ";
        out << "\"";
        if (node->high->var > 1)
            out << this->ID_to_var[node->high->var];
        else
            out << node->high->var;
        out << "\"" << std::endl;
        if (this->printed.find(*(node->low)) == this->printed.end())
            _output(node->low, out);
        if (this->printed.find(*(node->high)) == this->printed.end())
            _output(node->high, out);
    }
}

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
            std::cerr << "Usage: ROBDD [-s] [-S] [-c] [-o filename]" << std::endl;
            std::cerr << " -s\t" << "- Print any SAT result" << std::endl;
            std::cerr << " -S\t" << "- Print all SAT result" << std::endl;
            std::cerr << " -c\t" << "- Print SAT count" << std::endl;
            std::cerr << " -o filename\t" << "- Print ROBDD to image" << std::endl;
            return 1;
        default:
            abort();
        }
    }

    T = new ROBDD();
    yyparse();

    if (all_sat_flag)
        std::cout << "ALL SAT: " << T->SAT(true) << std::endl;
    if (any_sat_flag)
        std::cout << "ANY SAT: " << T->SAT(false) << std::endl;
    if (sat_count_flag)
        std::cout << "SAT count: " << T->SATCOUNT() << std::endl;
    if (output_filename)
    {
        std::string filename = output_filename;
        std::ofstream out(filename + ".tmp");
        T->output(out);
        out.close();
        FILE *fp = popen(("dot " + filename + ".tmp" + " -Tsvg -o " + filename).c_str(), "r");
        fclose(fp);
        FILE *rfp = popen(("rm -f " + filename).c_str(), "r");
        fclose(rfp);
        std::cout << "output written to " << filename << '.' << std::endl;
    }
    return 0;
}