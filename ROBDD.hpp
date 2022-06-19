#ifndef _ROBDD
    #define _ROBDD
    #include <map>
    #include <string>
    enum binary_op {OP_AND, OP_OR, OP_NOT, OP_THEN};
    struct BDD_node
    {
        // var = 0 and var = 1 reserved
        unsigned int var;
        BDD_node *high, *low;
        unsigned int ref_count{0};
        bool operator<(const BDD_node &rhs) const {return this->var < rhs.var;}
    };

    struct ROBDD
    {
    private:
        BDD_node *root;
        BDD_node *one, *zero;
        std::map<BDD_node, BDD_node*> node_table;
        std::map<std::string, unsigned int> var_to_ID;
        std::map<std::pair<BDD_node*, BDD_node*>, BDD_node*> apply_table[sizeof(binary_op)];
        unsigned int avail_ID = 2; //0 & 1 is reserved
        BDD_node *apply_(binary_op op, BDD_node *high, BDD_node *low);
        BDD_node *calc(binary_op op, BDD_node *high, BDD_node *low);
    public:
        ROBDD();
        // get id for var
        unsigned int get_ID(std::string var);
        // add or return existed node
        BDD_node *make_node(unsigned int ID, BDD_node *high, BDD_node *low);
        // eliminate binary operators
        BDD_node *apply(binary_op op, BDD_node *high, BDD_node *low);
        void set_root(BDD_node * node);
        // delete nodes with references count = 0
        void trim();
    };
#endif