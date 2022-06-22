#include "ROBDD.hpp"
#include <fstream>
bool BDD_node::operator==(const BDD_node &rhs) const
{
    return this->var == rhs.var && this->low == rhs.low && this->high == rhs.high;
}
ROBDD::ROBDD()
{
    this->zero = new BDD_node({0, nullptr, nullptr});
    this->one = new BDD_node({1, nullptr, nullptr});
    this->node_table[*(this->zero)] = this->zero;
    this->node_table[*(this->one)] = this->one;
}

BDD_node *ROBDD::get_one() const
{
    return this->one;
}
BDD_node *ROBDD::get_zero() const
{
    return this->zero;
}
bool ROBDD::empty()
{
    return this->root == nullptr;
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
    case OP_XNOR:
        if (left->var == right->var)
            return this->one;
        else
            return this->zero;
        break;
    case OP_XOR:
        if (left->var != right->var)
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
    return this->_SATCOUNT(node->high) + this->_SATCOUNT(node->low);
}

// buggy
unsigned int ROBDD::SATCOUNT()
{
    return this->_SATCOUNT(this->root);
}

void ROBDD::output(std::ofstream &out)
{
    this->printed.clear();
    this->printed[this->zero] = 0;
    this->printed[this->one] = 1;
    this->uuid = 2;
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
    // print node itself
    if (this->printed.find(node) == this->printed.end())
    {
        this->printed[node] = uuid++;
        out << "\"";
        out << this->printed[node]; // use uuid to identify nodes
        out << "\" [label=\"";
        if (node->var > 1)
            out << this->ID_to_var[node->var];
        else
            out << node->var;
        out << "\"]" << std::endl; // use label as node name
    }
    // no subnodes
    if (node->var <= 1)
        return;
    // print links to subnodes
    else
    {
        // left
        if (this->printed.find(node->low) == this->printed.end())
            _output(node->low, out);
        out << "\"";
        out << this->printed[node];
        out << "\" -> \"";
        out << this->printed[node->low];
        out << "\" [style = dotted]" << std::endl;
        // right
        if (this->printed.find(node->high) == this->printed.end())
            _output(node->high, out);
        out << "\"";
        out << this->printed[node];
        out << "\" -> \"";
        out << this->printed[node->high];
        out << "\"" << std::endl;
    }
}