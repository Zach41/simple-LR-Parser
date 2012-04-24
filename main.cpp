#include "parser.cpp"

int main(){
     
    /* Grammar */
    Grammar* g = new Grammar();
    g->loadFromFile("grammar.txt");
    
    /* Parser Generator */
    SLRParserGenerator* parserGenerator = new SLRParserGenerator(g);
    parserGenerator->generate();
    
    /* LR Parser */
    Parser* parser = new Parser(parserGenerator);
    parser->loadInputFromFile("input.txt");
    parser->parse();
   
   
    return 0;
}




