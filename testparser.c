#include "parser.h"

int main(int argc, char* argv[]) {
    
    ParserResult* p = parse("test.txt");
    afficher_parser_result(p);
    free_parser_result(p);

    return 0;
}