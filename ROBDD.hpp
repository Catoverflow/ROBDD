#ifndef _ROBDD
#define _ROBDD
#include <unordered_map>
#include <string>
#include <vector>
enum binary_op
{
    OP_AND,
    OP_OR,
    OP_THEN
};
struct BDD_node
{
    // var = 0 and var = 1 reserved
    unsigned int var;
    BDD_node *low, *high;
    bool operator==(const BDD_node &rhs) const { return this->var == rhs.var && this->low == rhs.low && this->high == rhs.high; }
};
struct ROBDD
{
private:
    struct BDD_node_hash
    {
        std::size_t operator()(const BDD_node& T) const
        {
            return std::hash<void*>()(static_cast<void*>(T.low))^std::hash<void*>()(static_cast<void*>(T.high))^std::hash<int>()(T.var);
        }
    };
    struct BDD_pair_hash
    {
        std::size_t operator()(const std::pair<BDD_node*, BDD_node*> &T) const
        {
            return std::hash<void*>()(static_cast<void*>(T.first))^std::hash<void*>()(static_cast<void*>(T.second));
        }
    };
    BDD_node *root;
    // for make_node()
    std::unordered_map<BDD_node, BDD_node *, BDD_node_hash> node_table;
    std::unordered_map<std::string, unsigned int> var_to_ID;
    // for node ID assignment
    unsigned int avail_ID = 2; // 0 & 1 is reserved
    // for apply()
    std::unordered_map<std::pair<BDD_node *, BDD_node *>, BDD_node *, BDD_pair_hash> apply_table[sizeof(binary_op)];
    BDD_node *_apply(binary_op op, BDD_node *low, BDD_node *high);
    BDD_node *calc(binary_op op, BDD_node *left, BDD_node *right);
    void _output(BDD_node *, std::ofstream &);
    // for ROBDD printing
    std::vector<bool> printed;
    std::unordered_map<unsigned int, std::string> ID_to_var;
    // for SAT question
    bool _SAT(BDD_node *, bool all_sat);
    unsigned _SATCOUNT(BDD_node *);

public:
    ROBDD();
    // get id for var
    unsigned int get_ID(std::string var);
    // add or return existed node
    BDD_node *make_node(unsigned int ID, BDD_node *low, BDD_node *high);
    // eliminate binary operators
    BDD_node *apply(binary_op op, BDD_node *low, BDD_node *high);
    void set_root(BDD_node *);
    // delete nodes with references count = 0
    // void trim();
    // print ROBDD to .dot file for graphviz
    void output(std::ofstream &);
    bool SAT(bool all_sat);
    unsigned int SATCOUNT();
    BDD_node *one, *zero;
};
#endif