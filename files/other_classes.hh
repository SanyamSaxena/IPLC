#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

static vector<exp_astnode*> checkComparison(exp_astnode* lhs, exp_astnode* rhs, int checktype){
         vector<exp_astnode*> result;
        
         result.push_back(lhs);
         result.push_back(rhs);

         if(result[0]->type->getString() == "int" && result[1]->type->getString() == "int"){
              result[1]->type->valid=1;
         }         
         else if(result[0]->type->getString() == "float" && result[1]->type->getString() == "float"){
            result[1]->type->valid=2;
         }
         else if(result[0]->type->getString() == "int" && result[1]->type->getString() == "float"){
              result[0] = new op_unary_astnode("TO_FLOAT", result[0]);
              result[1]->type = new type_class("float","",0,0,0,0,0); 
            result[1]->type->valid=2;
         }
         else if(result[0]->type->getString() == "float" && result[1]->type->getString() == "int"){
              result[1] = new op_unary_astnode("TO_FLOAT", result[1]);
              result[1]->type = new type_class("float","",0,0,0,0,0); 
            result[1]->type->valid=2;
         }
         else if((result[1]->type->deref_level +result[1]->type->array_level) >0 && (result[0]->type->getString() == result[1]->type->getString())){
            result[1]->type->valid=1;
         }
         else if(result[1]->type->getString() == "void*" && (result[0]->type->deref_level + result[0]->type->array_level) >= 1 && checktype){
             result[1]->type->valid=1;
             return result;
        }
        else if(result[0]->type->getString() == "void*" && (result[1]->type->deref_level + result[1]->type->array_level) >= 1 && checktype){
             result[1]->type->valid=1;
             return result;
        }
         else if(result[0]->type->array_level == 0 && result[0]->type->deref_level>0 && result[1]->type->array_level == 1 && result[1]->type->deref_level == result[0]->type->deref_level-1){
        
             result[1]->type->valid=1;
                return result;
         }
         else if(result[1]->type->array_level == 0 && result[1]->type->deref_level>0 && result[0]->type->array_level == 1 && result[0]->type->deref_level == result[1]->type->deref_level-1){
             result[1]->type->valid=1;
                return result;
         }
         else if ((result[1]->type->deref_level +result[1]->type->array_level == result[0]->type->deref_level +result[0]->type->array_level)){
             if(result[0]->type->bracketed){
                 queue<int> lhselems = result[0]->type->numElems;
                 queue<int> rhselems = result[1]->type->numElems;
                 if(result[1]->type->array_level==0){
                     result[1]->type->valid=0;
                     return result;
                 }

                 rhselems.pop();
                 while(!rhselems.empty()){
                     if(!(rhselems.front() == lhselems.front())){
                         result[1]->type->valid=0;
                         return result;
                     }
                     rhselems.pop();
                     lhselems.pop();
                 }
                 result[1]->type->valid=1;
                     return result;
             }
             else if(result[1]->type->bracketed){
                 queue<int> lhselems = result[0]->type->numElems;
                 queue<int> rhselems = result[1]->type->numElems;
                 if(result[0]->type->array_level==0){
                     result[1]->type->valid=0;
                     return result;
                 }

                 lhselems.pop();
                 while(!rhselems.empty()){
                     if(!(rhselems.front() == lhselems.front())){
                         result[1]->type->valid=0;
                         return result;
                     }
                     rhselems.pop();
                     lhselems.pop();
                 }
                 result[1]->type->valid=1;
                     return result;
             }
             else {
                 result[1]->type->valid=0;
                 return result;
             }
         }
         else if((result[0]->type->deref_level+result[0]->type->array_level>0) && result[1]->type->isZero==1){
             result[1]->type->valid=1;
                     return result;
         }
         
         else{
            result[1]->type->valid=0;
         }
        return result;
}

static exp_astnode* checkAssign(exp_astnode* lhs,exp_astnode*rhs, int proccall){
    if (lhs->type->lvalue==1){
        if(lhs->type->isConst==1){
             rhs->type->valid=0;
             return rhs;
        }
        else if(lhs->type->getString() == "int" && rhs->type->getString() == "float"){
             if(proccall!=1) rhs = new op_unary_astnode("TO_INT", rhs);
             rhs->type = new type_class("int","",0,0,0,0,0); 
             rhs->type->valid=1;
             return rhs;
        }
        else if(lhs->type->getString() == "float" && rhs->type->getString() == "int"){
             if(proccall!=1) rhs = new op_unary_astnode("TO_FLOAT", rhs);
             rhs->type = new type_class("float","",0,0,0,0,0); 
             rhs->type->valid=1;
             return rhs;
        }
        else if((rhs->type->deref_level + rhs->type->array_level == 1) && (lhs->type->deref_level +lhs->type->array_level == 1)){
            rhs->type->valid=1;
            return rhs;
        }
        else if((lhs->type->isStruct == rhs->type->isStruct) && (lhs->type->type == rhs->type->type) && (lhs->type->deref_level == rhs->type->deref_level) && (lhs->type->array_level == rhs->type->array_level)){
             rhs->type->valid=1;
             return rhs;
        }
        else if(rhs->type->getString() == "void*" && (lhs->type->deref_level + lhs->type->array_level) >= 1){
             rhs->type->valid=1;
             return rhs;
        }
        else if(lhs->type->getString() == "void*" && (rhs->type->deref_level + rhs->type->array_level) >= 1){
             rhs->type->valid=1;
             return rhs;
        }
        else if((lhs->type->isStruct == rhs->type->isStruct) && (lhs->type->type == rhs->type->type) && (lhs->type->deref_level == 1) && (lhs->type->array_level == 0) && (rhs->type->deref_level == 0) && (rhs->type->array_level == 1)){
             rhs->type->valid=1;
             return rhs;
        }
        else if((lhs->type->deref_level + lhs->type->array_level>0) && rhs->type->isZero==1){
             rhs->type->valid=1;
             return rhs;
        }
        else if(lhs->type->array_level == 0 && lhs->type->deref_level>0 && rhs->type->array_level == 1 && rhs->type->deref_level == lhs->type->deref_level-1){
             rhs->type->valid=1;
                return rhs;
         }
        else if(rhs->type->array_level == 0 && rhs->type->deref_level>0 && lhs->type->array_level == 1 && lhs->type->deref_level == rhs->type->deref_level-1){
             rhs->type->valid=1;
                return rhs;
         }
        else{
             rhs->type->valid=0;
             return rhs;
        }
    }
    else{
         rhs->type->valid=-1;
         return rhs;
    }
}



class type_specifier_class
{
public:
    string type;
    int isStruct;
    type_specifier_class(string name, int isStruct){
        this->type = name;
        this->isStruct = isStruct;
    }
};

class declarator_class
{
public:
    int deref_level, array_level;
    queue<int> numElems;
    string name;

    declarator_class(string name){
        this->name = name;
        this->deref_level = 0;
        this->array_level = 0;
    }

    void inc_arr(string num){
        array_level++;
        numElems.push(stoi(num));
    }
};


class declarator_list_class
{
public:
    vector<declarator_class*> declarators;
    declarator_list_class(declarator_class* declarator){
        this->declarators.push_back(declarator);
    }

    void insert(declarator_class * newentry){
        this->declarators.push_back(newentry);
    }
};

class declaration_class
{
public:
    type_specifier_class* type_spec;
    declarator_list_class* declarator_list;
    declaration_class(type_specifier_class* name, declarator_list_class* dec_list){
        this->type_spec = name;
        declarator_list = dec_list;
    }
};

class declaration_list_class
{
public:
    vector<declaration_class*> declarators;
    declaration_list_class(declaration_class* declarator){
        this->declarators.push_back(declarator);
    }

    void insert(declaration_class * newentry){
        this->declarators.push_back(newentry);
    }
};

class parameter_declaration_class
{
public:
    type_specifier_class* type_spec;
    declarator_class* declarator;
    parameter_declaration_class(type_specifier_class* name,declarator_class* dec){
        this->type_spec = name;
        declarator = dec;
    }
};


class parameter_list_class
{
public:
    vector<parameter_declaration_class*> parameters;
    parameter_list_class(parameter_declaration_class* parameter){
        this->parameters.push_back(parameter);
    }

    void insert(parameter_declaration_class * newentry){
        this->parameters.push_back(newentry);
    }
};

class fun_declarator_class
{
public:
    parameter_list_class* param_list;
    string name;
    int has_param;

    fun_declarator_class(string name){
        this->name = name;
        has_param = 0;
    }
    
    fun_declarator_class(string name, parameter_list_class* par_list){
        this->name = name;
        param_list = param_list;
        has_param = 1;
    }

};
