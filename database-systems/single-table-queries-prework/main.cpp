#include <tomekdb_tuple.h>
#include <algorithm>
#include <iterator>
#include <tomekdb_scaniterator.h>
#include <tomekdb_limititerator.h>

int main(int argc, char *argv[])
{
    std::list<tomekdb::Tuple> records = {
        {{"company", "Merck & Co. Inc."}, {"ticker", "mrk"}, {"price", 127.79}},
        {{"company", "Walmart Inc."}, {"ticker", "wmt"}, {"price", 107.36}},
        {{"company", "Dow Inc."}, {"ticker", "dow"}, {"price", 55.48}},
        {{"company", "International Business Machines Corp."}, {"ticker", "ibm"}, {"price", 187.64}},
        {{"company", "Home Depot Inc."}, {"ticker", "hd"}, {"price", 362.35}},
        {{"company", "Procter & Gamble Co."}, {"ticker", "pg"}, {"price", 157.51}},
        {{"company", "UnitedHealth Group Inc."}, {"ticker", "unh"}, {"price", 521.55}},
        {{"company", "Chevron Corp."}, {"ticker", "cvx"}, {"price", 154.63}},
        {{"company", "McDonald's Corp."}, {"ticker", "mcd"}, {"price", 292.02}},
        {{"company", "American Express Co."}, {"ticker", "axp"}, {"price", 212.56}},
        {{"company", "Coca-Cola Co."}, {"ticker", "ko"}, {"price", 59.39}},
        {{"company", "Johnson & Johnson"}, {"ticker", "jnj"}, {"price", 156.55}},
        {{"company", "Verizon Communications Inc."}, {"ticker", "vz"}, {"price", 40.49}},
        {{"company", "Goldman Sachs Group Inc."}, {"ticker", "gs"}, {"price", 384.44}},
        {{"company", "Walgreens Boots Alliance Inc."}, {"ticker", "wba"}, {"price", 21.98}},
        {{"company", "Caterpillar Inc."}, {"ticker", "cat"}, {"price", 321.91}},
        {{"company", "Honeywell International Inc."}, {"ticker", "hon"}, {"price", 197.17}},
        {{"company", "JPMorgan Chase & Co."}, {"ticker", "jpm"}, {"price", 179.03}},
        {{"company", "Microsoft Corp."}, {"ticker", "msft"}, {"price", 404.06}},
        {{"company", "Travelers Co. Inc."}, {"ticker", "trv"}, {"price", 217.53}},
        {{"company", "Boeing Co."}, {"ticker", "ba"}, {"price", 203.89}},
        {{"company", "Walt Disney Co."}, {"ticker", "dis"}, {"price", 111.6}},
        {{"company", "Salesforce Inc."}, {"ticker", "crm"}, {"price", 289.72}},
        {{"company", "Apple Inc."}, {"ticker", "aapl"}, {"price", 182.31}},
        {{"company", "Visa Inc. CI A"}, {"ticker", "v"}, {"price", 278.56}},
        {{"company", "3M Co."}, {"ticker", "mmm"}, {"price", 91.25}},
        {{"company", "Intel Corp."}, {"ticker", "intc"}, {"price", 43.51}}};

    tomekdb::ScanIterator scanNode{records};
    size_t limit{1};
    tomekdb::LimitIterator limitNode{limit, &scanNode};
    while (const tomekdb::Tuple *tuple = limitNode.next())
    {
        std::cout << *tuple;
    }

    return 0;
}
