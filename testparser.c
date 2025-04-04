#include "parser.h"

void main(){
    
    ParserResult* p = parse("test.txt");
    afficher_parser_result(&p);

    return;
}