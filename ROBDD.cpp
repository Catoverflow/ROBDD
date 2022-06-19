#include "ROBDD.hpp"
#include <vector>
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
    {
        high = this->one;
        this->one->ref_count += 1;
    }
    if (low == nullptr)
    {
        low = this->zero;
        this->zero->ref_count += 1;
    }
    if (low == high)
        return low;
    auto found = this->node_table.find({ID, high, low});
    if (found != this->node_table.end())
        return found->second;
    else
    {
        BDD_node *new_node = new BDD_node({ID, high, low});
        high->ref_count += 1;
        low->ref_count += 1;
        this->node_table[*new_node] = new_node;
        return new_node;
    }
}

BDD_node *ROBDD::calc(binary_op op, BDD_node *left, BDD_node *right)
{
    switch (op)
    {
    case OP_AND:
        if (left->var == 0 or right->var == 0)
        {
            this->zero->ref_count += 1;
            return this->zero;
        }
        else if (left->var == 1)
        {
            right->ref_count += 1;
            return right;
        }
        else if (right->var == 1)
        {
            left->ref_count += 1;
            return left;
        }
        break;
    case OP_OR:
        if (left->var == 1 or right->var == 1)
        {
            this->one->ref_count += 1;
            return this->one;
        }
        else if (left->var == 0)
        {
            right->ref_count += 1;
            return right;
        }
        else if (right->var == 0)
        {
            left->ref_count += 1;
            return left;
        }
        break;
    case OP_THEN:
        if (left->var == 0 or right->var == 1)
        {
            this->one->ref_count += 1;
            return this->one;
        }
        else if (left->var == 1)
        {
            right->ref_count += 1;
            return right;
        }
        else if (right->var == 0)
        {
            left->ref_count += 1;
            auto tmp = left->low;
            left->low = left->high;
            left->high = tmp;
            return left;
        }
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
    if (found != this->apply_table[op].end())
        return found->second;
    else if (left->var <= 1 or right->var <= 1)
        return this->calc(op, left, right);
    else if (left->var == right->var)
        return this->make_node(left->var, this->apply_(op, left->low, right), this->apply_(op, left->high, right));
    else if (left->var < right->var)
        return this->make_node(left->var, this->apply_(op, left->low, right), this->apply(op, left->high, right));
    else
        return this->make_node(right->var, this->apply_(op, left, right->low), this->apply(op, left, right->high));
}

void ROBDD::set_root(BDD_node *node)
{
    this->root = node;
    node->ref_count += 1;
}

void ROBDD::trim()
{
    std::vector<BDD_node> to_remove;
    for (auto it : this->node_table)
        if (it.second->ref_count == 0)
            to_remove.push_back(it.first);
    for (auto it : to_remove)
    {
        delete this->node_table[it];
        this->node_table.erase(it);
    }
    for (auto it : this->apply_table)
        it.clear();
}

int main()
{
    T = new ROBDD();
    yyparse();
    return 0;
}