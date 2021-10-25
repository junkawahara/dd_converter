
#include "tdzdd_converter.hpp"

#include "tdzdd/spec/SizeConstraint.hpp"

using namespace tdzdd;
using namespace dd_converter;

int main() {

    IntRange range(2, 2); // size just 2
    SizeConstraint sc(3, range);
    DdStructure<2> dd1(sc);

    //output_as_graphillion_text(std::cout, dd1);

    DdStructure<2> dd2 = graphillionToTdZdd(std::cin);

    //std::cerr << dd2.size();
    outputAsGraphillionText(std::cout, dd2);

    return 0;
}
