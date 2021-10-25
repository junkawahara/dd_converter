
#include "tdzdd_converter.hpp"

#include "tdzdd/spec/SizeConstraint.hpp"

using namespace tdzdd;
using namespace dd_converter;

int main() {

    IntRange range(2, 2); // size just 2
    SizeConstraint sc(3, range);
    DdStructure<2> dd1(sc); // dd representing "3 choose 2"

    // Output a ZDD in the graphillion format to std::cout
    outputAsGraphillionText(std::cout, dd1);

    // Construct a ZDD from std::cin
    DdStructure<2> dd2 = graphillionToTdZdd(std::cin);

    // Output a ZDD in the graphillion format to std::cout
    outputAsGraphillionText(std::cout, dd2);

    return 0;
}
