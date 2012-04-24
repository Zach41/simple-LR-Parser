#include "parser.h"

/*******BNFUnit*****************************/
BNFUnit::BNFUnit() { type = 0; }
BNFUnit::BNFUnit(string s) { name = s; type = 0; }
void BNFUnit::setName(string s) { name = s; }
string BNFUnit::getName() { return name; }
int BNFUnit::getType() { return type; }

/*******NonTerminal*****************************/
NonTerminal::NonTerminal() { type = 1; }
NonTerminal::NonTerminal(string s) { name = s; type = 1; }

/*******Terminal*****************************/
Terminal::Terminal() { type = 2; }
Terminal::Terminal(string s) { name = s; type = 2; }

/*******RuleUnit*****************************/
RuleUnit::RuleUnit() {}

void RuleUnit::add(BNFUnit* bnf)
{
    unitContent.push_back(bnf);
}

/*******Rule*****************************/
Rule::Rule(string s) 
{
    name = s;    
}

RuleUnit* Rule::createNewUnit()
{
    RuleUnit* newUnit = new RuleUnit();
    ruleContent.push_back(newUnit);
    return newUnit;
}

string Rule::getRuleString()
{
    string rule = name;
    rule += " -> ";
    for(int c = 0; c < ruleContent[0]->unitContent.size(); c++)
    {
        rule += "<" + ruleContent[0]->unitContent[c]->getName() + ">";
        //if(c < ruleContent[0]->unitContent.size() - 1)
            //rule += " ";
    }
    
    return rule;
}


/*******Grammer*****************************/

Grammar::Grammar()
{
    
    Debug::write("Creating grammar...");
}

void Grammar::loadFromFile(string fileName)
{
    ifstream reader;
    reader.open(fileName.c_str());
    string line;
    
    Debug::write("Reading from " + fileName + "...");
    while(!reader.eof()) 
    {
        getline(reader,line);
        
        if(line[0] == '#') { continue; }
        
        Rule* rule;
        RuleUnit* ruleUnit;
        
        bool readingNonTerminal = false;
        bool readingTerminal = false;
        bool switchNewUnit = false;
        string nameHolder = "";
        
        for(int i = 0; i < line.length(); i++)
        {
            if( i < line.length() - 1 && !readingTerminal && line[i+1] == ':')
            {
                rule = new Rule(nameHolder);
                rules.push_back(rule);
                ruleUnit = rule->createNewUnit();
                readingNonTerminal = false;
                readingTerminal = false;
                switchNewUnit = false;
                nameHolder = "";
                continue;
            }
            
            if(switchNewUnit)
            {
                ruleUnit = rule->createNewUnit();
                switchNewUnit = false;
            }
            
            if(readingNonTerminal)
            {
                if(line[i] == '>')
                {
                    readingNonTerminal = false;
                    readingTerminal = false;
                    Debug::write("\tFound a NonTerminal \"" + nameHolder + "\"" );
                               
                    BNFUnit* nonTerminal = new NonTerminal(nameHolder);
                    ruleUnit->add(nonTerminal);
                    nameHolder = "";
                    continue;
                }else 
                {
                    nameHolder += line.at(i);
                }
            }
            
            if(readingTerminal)
            {
                if(line[i] == '"')
                {
                    readingNonTerminal = false;
                    readingTerminal = false;
                    Debug::write("\tFound a Terminal \"" + nameHolder + "\"" );
                    
                    BNFUnit* terminal = new Terminal(nameHolder);
                    ruleUnit->add(terminal);
                    nameHolder = "";
                    continue;
                }else 
                {
                    nameHolder += line.at(i);
                }
            }
            
            if(line[i] == '<') 
            {
                readingNonTerminal = true;
                readingTerminal = false;
            }
            if(line[i] == '"')
            {
                readingNonTerminal = false;
                readingTerminal = true;
            }
            if(line[i] == '|')
            {
                readingNonTerminal = false;
                readingTerminal = false;
                switchNewUnit = true;
            }
        }
    }
    reader.close();
    createUniqueRules();
    Debug::write("Reading is done!");
    Debug::writeTitle("Constructed BNF");
    Debug::write(getBNF());
}

void Grammar::createUniqueRules()
{
    uniqueRules.clear();
    
    for(int i = 0; i < rules.size(); i++)
    {
        for(int y = 0; y < rules[i]->ruleContent.size(); y++)
        {    
            Rule* dumyRule = new Rule(rules[i]->name);
            RuleUnit* dumyUnit = dumyRule->createNewUnit();
            
            for(int z = 0; z < rules[i]->ruleContent[y]->unitContent.size(); z++)
            {
                string name = rules[i]->ruleContent[y]->unitContent[z]->getName();
                
                if(rules[i]->ruleContent[y]->unitContent[z]->getType() == 1)
                {
                    BNFUnit* nonTerminal = new NonTerminal(name);
                    dumyUnit->add(nonTerminal);
                    
                }else {
                    BNFUnit* terminal = new Terminal(name);
                    dumyUnit->add(terminal);
                }
            }            
            
            uniqueRules.push_back(dumyRule);
        }
    }
}

string Grammar::getBNF()
{
    string bnf = "";
    
    for(int i = 0; i < rules.size(); i++)
    {
        bnf +=  rules[i]->name;
        bnf += " -> "; 
        
        for(int y = 0; y < rules[i]->ruleContent.size(); y++)
        {
            for(int z = 0; z < rules[i]->ruleContent[y]->unitContent.size(); z++)
            {
                string a= rules[i]->ruleContent[y]->unitContent[z]->getName();
                
                if(rules[i]->ruleContent[y]->unitContent[z]->getType() == 2)
                {
                    transform(a.begin(), a.end(), a.begin(), ::tolower);
                }
                
                bnf += a;
                
                if( z < rules[i]->ruleContent[y]->unitContent.size() - 1) 
                {
                    bnf += " ";
                }
            }
            
            if( y < rules[i]->ruleContent.size() - 1)
            {
                bnf += " | "; 
            }
        }
        bnf += ".\n";
    }
    
    return bnf;
}



vector<string> Grammar::getFirstOf(string ruleName)
{
    vector<string> result;
    bool isAcceptEmpty = false;
    bool secondTour = false;
    
    for(int i = 0; i < uniqueRules.size(); i++)
    {
        if(uniqueRules[i]->name == ruleName)
        {
            vector<BNFUnit*> content = uniqueRules[i]->ruleContent[0]->unitContent;
            
            if(content[0]->getType() == 1 && content[0]->getName() == ruleName)
            {
                if(secondTour)
                {
                    int p = 1;
                    while(true)
                    {
                        if(p < content.size())
                        {
                            if(content[p]->getType() == 1)
                            {
                                vector<string> temp = getFirstOf(content[p]->getName());
                                result.insert( result.end(), temp.begin(), temp.end() );
                                
                                // Must be test
                                vector<string>::iterator result2 = find(temp.begin(), temp.end(), "_");
                                if (result2 == temp.end()) 
                                {
                                    break;
                                }
                            }else if(content[p]->getType() == 2)
                            {
                                result.push_back(content[p]->getName());      
                                break;
                            } 
                            
                            p++;
                        }else{
                            break;
                        }   
                    }
                    // end of accepty empty situation
                    continue;
                }else
                {
                    continue;
                }
            }
            if(content[0]->getType() == 1)
            {
                vector<string> temp = getFirstOf(content[0]->getName());
                
                
                vector<string>::iterator finder = find(temp.begin(), temp.end(), "_");
                if (finder != temp.end()) 
                {
                    temp.erase(finder);
                    int p = 1;
                    
                    while(true)
                    {
                        if(p < content.size())
                        {
                            if(content[p]->getType() == 1)
                            {
                                vector<string> temp2 = getFirstOf(content[p]->getName());
                                result.insert( result.end(), temp2.begin(), temp2.end() );
                                
                                finder = find(temp2.begin(), temp2.end(), "_");
                                if (finder == temp2.end()) 
                                {
                                    break;
                                }
                            }else if(content[p]->getType() == 2)
                            {
                                result.push_back(content[p]->getName());      
                                break;
                            } 
                            
                            p++;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                result.insert( result.end(), temp.begin(), temp.end() );
                
            }else if(content[0]->getType() == 2)
            {
                result.push_back(content[0]->getName());
                if(content[0]->getName() == "_" && !secondTour)
                {
                    isAcceptEmpty = true;
                } 
            }
            
            if(isAcceptEmpty)
            {
                i = -1;
                result.clear();
                isAcceptEmpty = false;
                secondTour = true;
            }
        }
    }
    
    return result;
}

vector<string> Grammar::getFollowOf(string ruleName)
{
    vector<string> result;
    
    for(int i = 0; i < uniqueRules.size(); i++)
    {
        vector<BNFUnit*> content = uniqueRules[i]->ruleContent[0]->unitContent;
        
        for(int y = 0; y < content.size(); y++)
        {
            if(content[y]->getName() == ruleName)
            {
                if(y == content.size() - 1)
                {
                    // Follow(y) includes Follow(uniqueRules[i]->name)
                    if(uniqueRules[i]->name != ruleName)
                    {
                        for(int k = 0; k < rules.size(); k++)
                        {
                            if(rules[k]->name == uniqueRules[i]->name){
                                result.insert( result.end(), rules[k]->followSet.begin(), rules[k]->followSet.end() );
                                break;
                            }
                        }
                    }
                    break;
                }
                
                for(int z = y+1; z < content.size(); z++)
                {
                    if(content[z]->getName() == ruleName)
                    {
                        y = z - 1;                
                        break;
                    }else {
                        if(content[z]->getType() == 1)
                        {
                            vector<string> temp = getFirstOf(content[z]->getName());
                            
                            vector<string>::iterator finder = find(temp.begin(), temp.end(), "_");
                            if (finder != temp.end()) 
                            {
                                temp.erase(finder);
                                result.insert( result.end(), temp.begin(), temp.end() );
                                if(z == content.size() - 1)
                                {
                                    for(int k = 0; k < rules.size(); k++)
                                    {
                                        if(rules[k]->name == uniqueRules[i]->name){
                                            result.insert( result.end(), rules[k]->followSet.begin(), rules[k]->followSet.end() );
                                        }
                                    }
                                }
                            }else {
                                result.insert( result.end(), temp.begin(), temp.end() );
                            }
                            
                            if(z == content.size() - 1) {
                                y = z; break;
                            }
                            
                        }else if(content[z]->getType() == 2) {
                            result.push_back(content[z]->getName());
                            y = z;
                            break;
                        }
                    }
                }     
            }
        }
        
    }
    
    return result;
}

/* Implementation of Helpers */
int Grammar::getUnitCount(int ruleNumber)
{
    return uniqueRules[ruleNumber]->ruleContent[0]->unitContent.size();
}

string Grammar::getNextNonTerminal(int ruleNumber, int position)
{
    string name;
    if(position < uniqueRules[ruleNumber]->ruleContent[0]->unitContent.size())
    {
        return uniqueRules[ruleNumber]->ruleContent[0]->unitContent[position]->getName();
    }else{
        return "";
    }
   
    return "";
}

string Grammar::getRuleContent(int ruleNumber, int pointer)
{
    string rule = uniqueRules[ruleNumber]->name + " --> ";
    
    bool specialCase = false;
    if(uniqueRules[ruleNumber]->ruleContent[0]->unitContent.size() == pointer)
    {
        specialCase = true;
    }
    for(int i = 0; i < uniqueRules[ruleNumber]->ruleContent[0]->unitContent.size(); i++)
    {
        if(pointer != -1 && !specialCase && i==pointer)
        {
            rule += ".";
        }
        rule += "<" + uniqueRules[ruleNumber]->ruleContent[0]->unitContent[i]->getName() + ">";
        
        if(specialCase && i+1 == uniqueRules[ruleNumber]->ruleContent[0]->unitContent.size())
        {
            rule += ".";
        }
    }
    
    return rule;
}

/*****************************************************/

void SLRParserGenerator::generate()
{
    Debug::write("Creating First and Follow Sets...");
    findFirstSets();
    findFollowSets();
    Debug::write("First and follow sets are constructed!");
    Debug::write("Converting grammer to augemented version...");
    convertGrammerToAugementedVersion();
    grammar->createUniqueRules();
    Debug::write("Converting is done!");
    Debug::write("Creating Canonical Closure Sets(States)...");
    createStates();
    Debug::write("Canonical Closure Sets(States) Done!");
    printStates();
    Debug::write("Creating parsing table...");
    createParsingTable();
    Debug::write("Parsing Table is done!");
    printParsingTable();
}


void SLRParserGenerator::findFirstSets()
{
    for(int r = 0; r < grammar->rules.size(); r++) 
    {
        vector<string> firsts = grammar->getFirstOf(grammar->rules[r]->name);
        for(int j = 0; j < firsts.size(); j++)
        {
            vector<string>::iterator result = find( grammar->rules[r]->firstSet.begin(), 
                                                     grammar->rules[r]->firstSet.end(), 
                                                    firsts[j]);
            if (result == grammar->rules[r]->firstSet.end()) 
            {
                grammar->rules[r]->firstSet.push_back(firsts[j]);
            }
        }
    }
    
    /* First Set Debug */
    Debug::writeTitle("First Sets :");
    for(int r = 0; r < grammar->rules.size(); r++) {
        string unitContent = "{";
        for(int k = 0; k < grammar->rules[r]->firstSet.size(); k++)
        {
            unitContent += grammar->rules[r]->firstSet[k];
            if(k < grammar->rules[r]->firstSet.size() -1 )unitContent += ",";
        }
        unitContent += "}";
        Debug::write("First of " + grammar->rules[r]->name + " : " + unitContent);                
    }
}

void SLRParserGenerator::findFollowSets()
{
    grammar->rules[0]->followSet.push_back("$");
    
    for(int r = 0; r < grammar->rules.size(); r++) 
    {
        vector<string> follows = grammar->getFollowOf(grammar->rules[r]->name);
        for(int j = 0; j < follows.size(); j++)
        {
            vector<string>::iterator result = find( grammar->rules[r]->followSet.begin(), 
                                                     grammar->rules[r]->followSet.end(), 
                                                    follows[j]);
            if (result == grammar->rules[r]->followSet.end()) 
            {
                grammar->rules[r]->followSet.push_back(follows[j]);
            }
        }
    }
    
    /*
    for(int r = 0; r < grammar->rules.size(); r++)
    {
        string ruleName = grammar->rules[r]->name;
        
        for(int ur = 0; ur < grammar->uniqueRules.size(); ur++)
        {
            vector<BNFUnit*> content = grammar->uniqueRules[ur]->ruleContent[0]->unitContent;
            for(int c = 0; c < content.size(); c++)
            {
                if(content[c]->getName() == ruleName)
                {
                    if( c+1 < content.size() )
                    {
                        if(content[c+1]->getType() == 1) // NonTerminal
                        {
                            for(int rCounter = 0; rCounter < grammar->rules.size(); rCounter++)
                            {
                                if(grammar->rules[rCounter]->name == content[c]->getName())
                                {
                                    //Add First set to the follow set
                                    for(int frsSet = 0; frsSet < grammar->rules[rCounter]->firstSet.size(); frsSet++)
                                    {
                                        vector<string>::iterator result = 
                                            find(   grammar->rules[r]->followSet.begin(), 
                                                    grammar->rules[r]->followSet.end(), 
                                                    grammar->rules[rCounter]->firstSet[frsSet]);
                                        if (result == grammar->rules[r]->followSet.end()) 
                                        {
                                            grammar->rules[r]->followSet.push_back(grammar->rules[rCounter]->firstSet[frsSet]);
                                        }
                                    }    
                                }
                            } 
                        }else if(content[c+1]->getType() == 2) // Terminal
                        {
                            vector<string>::iterator result = 
                                find(   grammar->rules[r]->followSet.begin(), 
                                        grammar->rules[r]->followSet.end(), 
                                        content[c+1]->getName());
                            if (result == grammar->rules[r]->followSet.end()) 
                            {
                                grammar->rules[r]->followSet.push_back(content[c+1]->getName());
                            }
                        }
                        
                    }else if( c == content.size() -1 )
                    {
                        for(int s = 0; s < grammar->rules.size(); s++)
                        {
                            if(grammar->rules[s]->name == grammar->uniqueRules[ur]->name)
                            {
                                //Add Follow set to the follow set
                                for(int flwSet = 0; flwSet < grammar->rules[s]->followSet.size(); flwSet++)
                                {
                                    vector<string>::iterator result = 
                                        find(   grammar->rules[r]->followSet.begin(), 
                                                grammar->rules[r]->followSet.end(), 
                                                grammar->rules[s]->followSet[flwSet]);
                                    if (result == grammar->rules[r]->followSet.end()) 
                                    {
                                        grammar->rules[r]->followSet.push_back(grammar->rules[s]->followSet[flwSet]);
                                    }
                                } 
                            }
                        }
                    }
                }
            }
        }
    }
    */
    /* Follow Set Debug */
    Debug::writeTitle("Follow Sets");
    for(int r = 0; r < grammar->rules.size(); r++) {
        string unitContent = "{";
        for(int k = 0; k < grammar->rules[r]->followSet.size(); k++)
        {
            unitContent += grammar->rules[r]->followSet[k];
            if(k < grammar->rules[r]->followSet.size() -1 )unitContent += ",";
        }
        unitContent += "}";
    
        Debug::write("Follow of " + grammar->rules[r]->name + " : " + unitContent);                
    }
}

void SLRParserGenerator::convertGrammerToAugementedVersion()
{
    string dumyName = "S'";
    
    Rule* dumyRule = new Rule(dumyName);
    RuleUnit* dumyUnit = dumyRule->createNewUnit();
    
    string rootName = grammar->rules[0]->name;
    BNFUnit* nonTerminal = new NonTerminal(rootName);
    dumyUnit->add(nonTerminal);
    
    vector<Rule*> orderedRule;
    orderedRule.push_back(dumyRule);
    for(int r = 0; r < grammar->rules.size(); r++)
    {
        orderedRule.push_back(grammar->rules[r]);
    }
    
    grammar->rules.clear();
    grammar->rules = orderedRule;
    
    Debug::writeTitle("Converted BNF");
    Debug::write(grammar->getBNF());
}

void SLRParserGenerator::createStates()
{
    int stateNameCounter = 1;
    
    State* initialState = new State("s0");
    initialState->items.push_back(new Item(0,0));
    
    states.push_back(initialState);
    //createDerivations(states.size() - 1);
    

    /* Loops through states */
    for(int i = 0; i < states.size(); i++)
    {   
        /* Creates the Derivations */ 
        vector<string> addedRules;
        for(int y = 0; y < states[i]->items.size(); y++)
        {
            string next = grammar->getNextNonTerminal(states[i]->items[y]->ruleNumber,
                                                    states[i]->items[y]->pointPosition
                                                    );
            if(next != "")
            {
                
                vector<string>::iterator result = find(addedRules.begin(), addedRules.end(), next);
                if (result == addedRules.end()) 
                {
                    addedRules.push_back(next);
                    for(int z = 0; z < grammar->uniqueRules.size(); z++)
                    {
                        if(grammar->uniqueRules[z]->name == next)
                        {
                            states[i]->items.push_back(new Item(z, 0));
                        }                  
                    }
                }
            }                                       
        }
        addedRules.clear();
        /* Derivation Ends */
        
        /* Create new states */
        for(int y = 0; y < states[i]->items.size(); y++)
        {
            string next = grammar->getNextNonTerminal(states[i]->items[y]->ruleNumber,
                                                    states[i]->items[y]->pointPosition
                                                    );
            
            if(next != "")
            {
                int r = states[i]->items[y]->ruleNumber;
                int p = states[i]->items[y]->pointPosition + 1;
                
                bool isNewState = true;
                
                for(int z = 0; z < states[i]->links.size(); z++)
                {
                    if(states[i]->links[z]->labelName == next)
                    {
                        isNewState = false;
                        
                        bool alreadyIn = false;
                        for(int l = 0; l < states[i]->links[z]->targetState->items.size(); l++)
                        {
                            if(states[i]->links[z]->targetState->items[l]->ruleNumber == r
                               && states[i]->links[z]->targetState->items[l]->pointPosition == p)
                            {
                                alreadyIn = true;
                            }
                        }
                        if(!alreadyIn)
                        {
                            states[i]->links[z]->targetState->items.push_back(new Item(r, p));
                        }
                        break;
                    }
                }
                
                if(isNewState) {
                    
                    bool isInPreviousStates = false;
                    
                    for(int s = 0; s < states.size(); s++)
                    {
                        for(int t = 0; t < states[s]->items.size(); t++)
                        {
                            if(states[s]->items[t]->ruleNumber == r
                                && states[s]->items[t]->pointPosition == p
                                && !isInPreviousStates)
                            {
                                isInPreviousStates = true;
                                states[i]->links.push_back(new Link(next, states[s]));
                            }    
                        }
                    }
                     
                    if(!isInPreviousStates)
                    {
                        stringstream ss;
                        ss << stateNameCounter;
                        stateNameCounter++;
                        State* tempState = new State("s" + ss.str());    
                        tempState->items.push_back(new Item(r, p));
                        
                        states[i]->links.push_back(new Link(next, tempState));   
                        states.push_back(tempState);
                    }
                    
                }
            }
        }
        /* Create new states end */
    }
}

void SLRParserGenerator::createParsingTable()
{
    
    vector<string> terminals, nonterminals;
    
    /* Populating NonTerminals */
    for(int i = 0; i < grammar->rules.size(); i++)
    {
        nonterminals.push_back(grammar->rules[i]->name);
    }
    
    /* Populating Terminals */
    terminals.push_back("$");
    for(int i = 0; i < grammar->uniqueRules.size() ; i++)
    {
        vector<BNFUnit*> content = grammar->uniqueRules[i]->ruleContent[0]->unitContent;
        for(int y = 0; y < content.size(); y++)
        {
            if(content[y]->getType() == 2)
            {
                vector<string>::iterator result = find(terminals.begin(), terminals.end(), content[y]->getName());
                if (result == terminals.end()) 
                {
                    terminals.push_back(content[y]->getName());
                }
            }
        }
    }
    
    /* Constracting Parsing Table */
    for(int stateNo = 0; stateNo < states.size(); stateNo++)
    {
        map<string, ParsingTableAction*> x;
        
        /* CASE (a) */
        for(int links = 0; links < states[stateNo]->links.size(); links++)
        {
            vector<string>::iterator result = find(terminals.begin(), terminals.end(), states[stateNo]->links[links]->labelName);
            if (result != terminals.end()) 
            {
                ParsingTableAction* action = new ParsingTableAction(1);
                action->shiftTo = states[stateNo]->links[links]->targetState->name;
                x[states[stateNo]->links[links]->labelName] = action;
            }
        } 
        
        /* CASE (b) */
        for(int item = 0; item < states[stateNo]->items.size(); item++)
        {
            int ruleNumber = states[stateNo]->items[item]->ruleNumber;
            int point = states[stateNo]->items[item]->pointPosition;
                    
            vector<BNFUnit*> content = grammar->uniqueRules[ruleNumber]->ruleContent[0]->unitContent;
            
            if(point == content.size())
            {
                for(int rule = 0; rule < grammar->rules.size(); rule++)
                {
                    if(grammar->rules[rule]->name == grammar->uniqueRules[ruleNumber]->name)
                    {
                        for(int flw = 0; flw < grammar->rules[rule]->followSet.size(); flw++)
                        {
                            ParsingTableAction* action = new ParsingTableAction(2);
                            action->reduceRule = grammar->uniqueRules[ruleNumber];
                            x[grammar->rules[rule]->followSet[flw]] = action;
                        }
                    }
                }
            }
        }
        
        /* CASE (c) */
        bool acc = false;
        for(int item = 0; item < states[stateNo]->items.size(); item++)
        {
            int ruleNumber = states[stateNo]->items[item]->ruleNumber;
            int point = states[stateNo]->items[item]->pointPosition;
            if(ruleNumber == 0 && point == 1)
            {
                acc = true;
            }
        }
        if(acc)
        {
            ParsingTableAction* action = new ParsingTableAction(3);
            x["$"] = action;
        }
        
        /* Constructing GOTO's */
        for(int nt = 0; nt < nonterminals.size(); nt++)
        {
            for(int link = 0; link < states[stateNo]->links.size(); link++)
            {
                if(states[stateNo]->links[link]->labelName == nonterminals[nt])
                {
                    ParsingTableAction* action = new ParsingTableAction(4);
                    action->goTo = states[stateNo]->links[link]->targetState->name;
                    x[nonterminals[nt]] = action;
                }
            }   
        }
        
        parsingTable[states[stateNo]->name] = x;
    }
    
}
void SLRParserGenerator::printStates()
{
    Debug::writeTitle("States");
    for(int i = 0; i < states.size(); i++)
    {
        Debug::write("\n----------------------------------------");
        Debug::write("State Name : " + states[i]->name);
        for(int z = 0; z < states[i]->links.size(); z++)
        {
            Debug::write("( goes to " + states[i]->links[z]->targetState->name + " when reads : " + states[i]->links[z]->labelName + ")");
        }
        Debug::write("----------------------------------------");
        
        for(int y = 0; y < states[i]->items.size(); y++)
        {
            Item* temp = states[i]->items[y];
            string content = grammar->getRuleContent(temp->ruleNumber, temp->pointPosition);
            Debug::write(content);    
        }
        
    }
}

void SLRParserGenerator::printParsingTable()
{
    Debug::writeTitle("Parsing Table");
    map <string, map<string, ParsingTableAction*> >::iterator curr;
    map <string, ParsingTableAction*>::iterator curr2;
    
    for( curr = parsingTable.begin(); curr != parsingTable.end();  ++curr)
    {
        Debug::write(curr->first + " -> ");
        for( curr2 = curr->second.begin(); curr2 != curr->second.end(); ++curr2 )
        {
            if(curr2->second->type == 1)
                Debug::write("\t(" + curr2->first + ", shift to " + curr2->second->shiftTo + ")");
                
            if(curr2->second->type == 2)
            {
                Rule* r = curr2->second->reduceRule;
                Debug::write("\t(" + curr2->first + ", reduce " + r->getRuleString() + ")");
            }
            if(curr2->second->type == 3)
                Debug::write("\t(" + curr2->first + ", accept)");
                
            if(curr2->second->type == 4)
                Debug::write("\t(" + curr2->first + ", goto " + curr2->second->goTo + ")");
        }
    }
    
    
}
/*****************************************************/

void Parser::loadInputFromFile(string fileName)
{
    ifstream reader;
    reader.open(fileName.c_str());
    string content = "";
    
    while(!reader.eof()) 
    {
        string line;
        getline(reader,line);
        content += line;
    }
    
    reader.close();
    
    Parser::inputStream = content;
}

string Parser::getNextStream()
{
   
    if(streamPointer >= Parser::inputStream.length()) return "$";
    char temp = Parser::inputStream[streamPointer];
    streamPointer++;
    stringstream ss;
    ss << temp;
    return ss.str();
}

void Parser::parse()
{
    Debug::writeTitle("Parsing Results");
    
    /* Initilazing the Stack */
    stateStack.push(parserGenerator->states[0]->name);
    
    string readedStream = "";   
    string currentStream = getNextStream();
    
    while(true)
    {
        map<string, ParsingTableAction*>::iterator result = parserGenerator->parsingTable[stateStack.top()].find(currentStream);
        if (result == parserGenerator->parsingTable[stateStack.top()].end()) 
        {
            /* Error Recovery Mode */
            readedStream += currentStream;
            int remaining = 8 - readedStream.length();
            for(int i = 0; i < remaining; i++)
            {
                readedStream += getNextStream();
            }
            
            while(!stateStack.empty()) stateStack.pop();
            stateStack.push(parserGenerator->states[0]->name);
            
            currentStream = getNextStream();
            Debug::write(readedStream + "(Makamsiz)");
            if(currentStream == "$") break;
            else readedStream = "";
            continue;
        }
         
        ParsingTableAction* action = parserGenerator->parsingTable[stateStack.top()][currentStream];
        
        if(action->type == 1) // Shifting Action
        {
            readedStream += currentStream;
            currentStream = getNextStream();
            stateStack.push(action->shiftTo);
            
        }else if(action->type == 2) // Reduce Action
        {
            Rule* reduceRule = action->reduceRule;
            int popingCount = reduceRule->ruleContent[0]->unitContent.size();
            
            for(int p = 0; p < popingCount; p++)
            {
                stateStack.pop();
            }
            
            
            ParsingTableAction* action = parserGenerator->parsingTable[stateStack.top()][reduceRule->name];
            stateStack.push(action->goTo);
            
            //Debug::write(readedStream + reduceRule->getRuleString());
            
            if(reduceRule->name == "Huseyni" || 
                reduceRule->name == "Neva" ||
                reduceRule->name == "Muhayyer" ||
                reduceRule->name == "Tahir" ||
                reduceRule->name == "Ussak")
            {
                Debug::write(readedStream + "(" + reduceRule->name + ")");
                readedStream = "";
            }
            
            
        }else if(action->type == 3) // Accept Action
        {
            Debug::write("Accepted");
            break;
        }else
        {
           Debug::write("Not Accepted");
           break; 
        }
    }
    
    
}

/************************************************/


void Debug::write(string content)
{   
    ofstream writer;
      
    if(!Debug::session)
    {
        Debug::session = true;
        writer.open(Debug::debugFile.c_str());  
    }else
    {
        writer.open(Debug::debugFile.c_str(), ios::app);  
    }
           
    writer << content << endl;
    
    writer.close();
}

void Debug::writeTitle(string title)
{
    Debug::write("\n--------------------------------------------------------");
    Debug::write("-------------------------"+title+"-------------------------");
    Debug::write("--------------------------------------------------------");
}
