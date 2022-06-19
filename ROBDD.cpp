#include "ROBDD.hpp"
#include <fstream>
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

BDD_node *ROBDD::make_node(unsigned int ID, BDD_node *high, BDD_node *low)
{
    if (high == nullptr)
        high = this->one;
    if (low == nullptr)
        low = this->zero;
    if (low == high)
        return low;
    auto found = this->node_table.find({ID, high, low});
    if (found != this->node_table.end())
        return found->second;
    else
    {
        BDD_node *new_node = new BDD_node({ID, high, low});
        this->node_table[*new_node] = new_node;
        return new_node;
    }
}

BDD_node *ROBDD::calc(binary_op op, BDD_node *left, BDD_node *right)
{
    switch (op)
    {
    case OP_AND:
        if (left->var && right->var)
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
    this->apply_table[op].clear();
    return this->apply_(op, left, right);
}

BDD_node *ROBDD::apply_(binary_op op, BDD_node *left, BDD_node *right)
{
    auto found = this->apply_table[op].find(std::pair<BDD_node *, BDD_node *>(left, right));
    BDD_node *res;
    if (found != this->apply_table[op].end())
        res = found->second;
    else if (left->var <= 1 and right->var <= 1)
        res = this->calc(op, left, right);
    else if (left->var == right->var) // promote the shared var
        res = this->make_node(left->var, this->apply_(op, left->low, right->low), this->apply_(op, left->high, right->high));
    else if (left->var < right->var or right->var <= 1) // unfold left BDD tree
        res = this->make_node(left->var, this->apply_(op, left->low, right), this->apply(op, left->high, right));
    else if (left->var > right->var or left->var <= 1) // unfold right BDD tree
        res = this->make_node(right->var, this->apply_(op, left, right->low), this->apply(op, left, right->high));
    return res;
}

void ROBDD::set_root(BDD_node *node)
{
    this->root = node;
}

void ROBDD::trim()
{
    /*
    std::vector<BDD_node> to_remove;
    for (auto it : this->node_table)
        if (it.second->ref_count == 0)
            to_remove.push_back(it.first);
    for (auto it : to_remove)
    {
        delete this->node_table[it];
        this->node_table.erase(it);
    }
    */
    for (auto it : this->apply_table)
        it.clear();
}

void ROBDD::output(std::ofstream &out)
{
    this->printed.resize(this->avail_ID - 1);
    for (int i = 0; i < avail_ID - 1; i++)
        this->printed[i] = false;
    for (auto it : this->var_to_ID)
        this->ID_to_var[it.second] = it.first;
    out << "digraph ROBDD {\n\
    fontname=\"Helvetica,Arial,sans-serif\"\n\
    node [fontname=\"Helvetica,Arial,sans-serif\"]\n\
    edge [fontname=\"Helvetica,Arial,sans-serif\"]\n\
    node [shape=box];\n";
    this->_output(this->root, out);
    out << "}";
}

void ROBDD::_output(BDD_node *node, std::ofstream &out)
{
    if (!this->printed[node->var])
    {
        out << "\"";
        if (node->var > 1)
            out << this->ID_to_var[node->var];
        else
            out << node->var;
        out << "\"" << "[shape = circle]" << std::endl;
        this->printed[node->var] = true;
    }
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
        out << "\"" << "[style = dotted]" << std::endl;
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
        _output(node->low, out);
        _output(node->high, out);
    }
}

int main()
{
    T = new ROBDD();
    yyparse();
    std::ofstream out("res.dot");
    T->output(out);
    return 0;
}