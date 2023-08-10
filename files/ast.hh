#include <queue>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>

using namespace std;

class type_class{
public:
    string type;
    string name;
    int deref_level,array_level,isStruct,valid,lvalue,isConst,bracketed=0,returned=0,isZero=0;
    queue<int> numElems;

    type_class(){
        
    }

    type_class(string type,string name,int isStruct, int deref_level,int array_level, int lvalue, int isConst){
           this->type = type;
           this->name = name;
           this->isStruct = isStruct;
           this->deref_level = deref_level;
           this->array_level = array_level;
           this->lvalue = lvalue;
           this->isConst = isConst;
    }


    type_class(string type,string name,int isStruct, int deref_level,int array_level, int lvalue, int isConst, queue<int> numElems){
           this->type = type;
           this->name = name;
           this->isStruct = isStruct;
           this->deref_level = deref_level;
           this->array_level = array_level;
           this->lvalue = lvalue;
           this->isConst = isConst;
           this->numElems = numElems;
    }

    type_class(type_class * copy){
            this->type = copy->type;
           this->name = copy->name;
           this->isStruct = copy->isStruct;
           this->deref_level = copy->deref_level;
           this->array_level = copy->array_level;
           this->lvalue = copy->lvalue;
           this->isConst = copy->isConst;
           this->bracketed = copy->bracketed;
           this->numElems = copy->numElems;
           this->returned = copy->returned;
    }

    string getString(){
        string type_str="";

        if(isStruct){
             type_str = "struct " + type;
        }
        else{
             type_str = type;
        }

        for(int i=0;i<deref_level;i++){
             type_str = type_str+"*";
        }
        queue<int> replacement = numElems;
        if(bracketed){
            type_str = type_str + "(*)";
            array_level--;
        }
        if(array_level>0){               
             while(!replacement.empty()){
                  int val = replacement.front();
                  replacement.pop();
                  type_str = type_str + "[";
                  type_str = type_str + to_string(val);
                  type_str = type_str + "]";
             }
        }
        if(bracketed){
            array_level++;
        }

        return type_str;
    }

};


class Entry
{
public:
    string name;
    string varfun;
    string scope;
    int size;
    int offset;
    string type;

    Entry(string name,string varfun,string scope,int size,int offset,string type){
        this->name = name;  
        this->varfun = varfun;
        this->scope = scope;  
        this->size = size;  
        this->offset = offset;  
        this->type = type;  
    }
};

class EntryLocal: public Entry
{
public:
    type_class* type_c;
    EntryLocal(string name,string varfun,string scope,int size,int offset,string type,type_class* type_c) : Entry(name,varfun,scope,size,offset,type){
        this->type_c = type_c;
    }
};

static bool compareEntry(const EntryLocal &a,const EntryLocal &b){
    return a.name<b.name;
}


class LocalSymbTab 
{
public:
    std::vector<EntryLocal> Entries;


    const EntryLocal* getEntry(string name){
        for (const auto &entry : Entries)
        {
        	if (entry.name == name)
        	return &entry;
        }
        return nullptr;
    }


    vector<type_class*> getParams(){
        vector<type_class*> params;
        for (const auto &entry : Entries)
        {
        	if (entry.scope == "param") {
                params.push_back(entry.type_c);
            };
        }
        return params;
    }

    void setParamSizes(queue<int> param_sizes,int running_offset){
        for (auto &entry : Entries)
        {
        	if (entry.scope == "param"){
                running_offset -= param_sizes.front();
                param_sizes.pop();
            	entry.offset = running_offset;
            }
        }
    }

    void print(){
        sort(Entries.begin(),Entries.end(),compareEntry);
        cout<<"[\n";
        auto itr = Entries.begin();
        if(itr != Entries.end()){
            cout<<"\t";
            cout<<"[\""<<itr->name<<"\", \""<<itr->varfun<<"\", \""<<itr->scope<<"\", "<<itr->size<<", "<<itr->offset<<", \""<<itr->type<<"\"]";
            itr++;
        }
        while(itr!=Entries.end()){
            cout<<",\n";
            cout<<"\t";
            cout<<"[\""<<itr->name<<"\", \""<<itr->varfun<<"\", \""<<itr->scope<<"\", "<<itr->size<<", "<<itr->offset<<", \""<<itr->type<<"\"]";
            itr++;
        }
        cout<<"\n]\n";
    }
};

class EntryGlobal: public Entry
{
public:
    LocalSymbTab* symbtab;
    EntryGlobal(string name,string varfun,string scope,int size,int offset,string type,LocalSymbTab* st) : Entry(name,varfun,scope,size,offset,type){
        symbtab = st;
    }
};

class SymbTab 
{
public:
    std::map<std::string, EntryGlobal> Entries;

    const EntryGlobal* getStructEntry(string name){
        for (const auto &entry : Entries)
        {   
        	if (entry.second.varfun == "struct" && entry.first == name)
        	return &(entry.second);
        }
        return nullptr;
    }

    const EntryGlobal* getFuncEntry(string name){
        for (const auto &entry : Entries)
        {
        	if (entry.second.varfun == "fun" && entry.first == name){
            	return &(entry.second);
            }
        }
        return nullptr;
    }

    void setStructSize(string name, int newsize){
        for (auto &entry : Entries)
        {
        	if (entry.second.varfun == "struct" && entry.first == name){
            	entry.second.size = newsize;
            }
        }
      
    }
    void setSymbolTable(string name, LocalSymbTab * runningST){
        for (auto &entry : Entries)
        {
        	if (entry.first == name){
            	entry.second.symbtab = runningST;
            }
        }
      
    }

    void printgst(){
        cout<<"[\n";
        auto itr = Entries.begin();
        if(itr != Entries.end()){
            cout<<"\t";
            if(itr->second.offset==-1)
                cout<<"[\""<<itr->first<<"\", \""<<itr->second.varfun<<"\", \""<<itr->second.scope<<"\", "<<itr->second.size<<", "<<"\"-\""<<", \""<<itr->second.type<<"\"]";
            else
                cout<<"[\""<<itr->first<<"\", \""<<itr->second.varfun<<"\", \""<<itr->second.scope<<"\", "<<itr->second.size<<", "<<itr->second.offset<<", \""<<itr->second.type<<"\"]";
            itr++;
        }
        while(itr!=Entries.end()){
            cout<<",\n";
            cout<<"\t";
            if(itr->second.offset==-1)
                cout<<"[\""<<itr->first<<"\", \""<<itr->second.varfun<<"\", \""<<itr->second.scope<<"\", "<<itr->second.size<<", "<<"\"-\""<<", \""<<itr->second.type<<"\"]";
            else
                cout<<"[\""<<itr->first<<"\", \""<<itr->second.varfun<<"\", \""<<itr->second.scope<<"\", "<<itr->second.size<<", "<<itr->second.offset<<", \""<<itr->second.type<<"\"]";
            itr++;
        }
        cout<<"\n]\n";
    }   
};

extern SymbTab gststruct, gstfun;
extern stack<string> rstack, tstack;
extern vector<string> code;
extern int lcount;
extern int structlevel;
extern bool check_param;
extern int arraylevel;
extern bool addneeded;

static int getexpSize(type_class* type){
    int size_type, num_elem = 1,total_size;

    if(type->isStruct){
         size_type = gststruct.getStructEntry("struct "+ type->type)->size;
         if(type->deref_level>0){
              size_type = 4;
         }
    }
    else{
         size_type = 4;
    }

    if(type->array_level>0){
         queue<int> replacement = type->numElems;
         while(!replacement.empty()){
              int val = replacement.front();
              replacement.pop();
              num_elem = num_elem*val;
         }
    }
    total_size = size_type*num_elem;
    return total_size;
}

class abstract_astnode
{
public:
int label;
int isMain = 0;
int hasReturn = 0;
int isFunction = 0;
string funcname;
bool isId = false;
bool isArray = false;
bool isMember = false;
// virtual void gencode(LocalSymbTab * symbtab, stack<string> rstack, vector<string> &code, int& lcount)=0;
virtual void gencode(LocalSymbTab* symbtab)=0;
virtual void print(int blanks) = 0;
void printblanks(int blanks){
    for(int i = 0; i < blanks; i++){
        cout << "\t";
    }
}
};

class exp_astnode: public abstract_astnode
{
public:
    string identifier_id;
    int string_value;
    bool isconstant  = false;
    virtual void gencode(LocalSymbTab* symbtab) = 0;
    virtual void print(int blanks) = 0;
    type_class* type;
};

class statement_astnode: public abstract_astnode
{
public:
    virtual void gencode(LocalSymbTab* symbtab) = 0;
    virtual void print(int blanks) = 0;
    // enum typeExp astnode_type;

};

class ref_astnode: public exp_astnode{

};

class intconst_astnode: public exp_astnode{
    public: 
        int int_value;
        intconst_astnode(string value){
            int_value=stoi(value);
        }
        void gencode(LocalSymbTab* symbtab){
            code.push_back("\tmovl \t$"+to_string(int_value)+", %"+rstack.top());
            // cout<<"\tmovl \t$"<<int_value<<", %"<<rstack.top()<<"\n";
        }; 
        void print(int blanks){
            for(int i=0;i<blanks;i++){
                cout<<"\t";
            }
            cout<<"{ \"intconst\": "<< int_value <<" }\n";
        }
        
};

class floatconst_astnode: public exp_astnode{
    public: 
        float float_value;
        floatconst_astnode(string value){
            float_value=stof(value);
        };

        void gencode(LocalSymbTab* symbtab){
            for(int i  =0; i<10; i++)
            {
                int x = 0; 
            }
        }; 
        void print(int blanks){
            for(int i=0;i<blanks;i++){
                cout<<"\t";
            }
            cout<<"{ \"floatconst\": "<< float_value <<" }\n";
        }
        
};

class stringconst_astnode: public exp_astnode{
public: 
        
        stringconst_astnode(int value){
            string_value=value;
            isconstant = true;
        };

        void gencode(LocalSymbTab* symbtab){
            code.back() = code.back() + "$.LC"+to_string(string_value);
            // cout<<"$.LC"<<string_value;
        }; 
        void print(int blanks){
            for(int i=0;i<blanks;i++){
                cout<<"\t";
            }
            cout<<"{ \"stringconst\": "<< string_value <<" }\n";
        }
        
};

class op_binary_astnode: public exp_astnode{
public:
    exp_astnode *left;
    exp_astnode *right;
    string op;

    op_binary_astnode(string op, exp_astnode * left, exp_astnode *right){
        this->left = left;
        this->right = right;
        this->op = op;
    }

    
    void gencode(LocalSymbTab* symbtab){
            int stored = 0;
            while(rstack.size()<3){
                code.push_back("\tpushl \t%" + tstack.top());
                rstack.push(tstack.top());
                tstack.pop();
                stored++;
            }
            if(op=="PLUS_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                if(left->type->deref_level + left->type->array_level > 0){
                    int size;
                    size = 4;
                    if(left->type->isStruct){
                        string type = "struct " + left->type->type;
                        size = gststruct.getStructEntry(type)->size;
                    }
                    code.push_back("\timull\t$"+to_string(size)+", %"+rstack.top());
                }
                code.push_back("\taddl \t%"+rstack.top()+", %"+R);
                // cout<<"\taddl \t%"<<rstack.top()<<", %"<< R <<"\n";
                rstack.push(R);   tstack.pop();
            }
            if(op=="DIV_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);

                code.push_back("\tpushl\t%edx");
                code.push_back("\tpushl\t%eax");
                code.push_back("\tpushl\t%" + rstack.top());

                code.push_back("\tmovl \t%"+R+", %eax");
                code.push_back("\tcltd");
                code.push_back("\tidivl \t(%esp)");

                code.push_back("\tpushl\t%eax");
                code.push_back("\tmovl \t12(%esp), %edx");
                code.push_back("\tmovl \t8(%esp), %eax");
                code.push_back("\tpopl %" + R);
                code.push_back("\taddl \t$12, %esp");
                rstack.push(R);
                tstack.pop();

                // if(R == "eax" && rstack.top()!="edx"){
                //     code.push_back("\tpushl\t%edx");
                //     code.push_back("\tcltd");
                //     code.push_back("\tidiv \t%"+rstack.top());
                //     code.push_back("\tmovl \t(%esp), %edx");
                //     code.push_back("\taddl \t$4, %esp");
                //     rstack.push(R);   tstack.pop();
                // }
                // else if(R=="eax" && rstack.top() =="edx"){
                //     string R2 = rstack.top();   rstack.pop(); tstack.push(R2);
                //     code.push_back("\tmovl \t%edx, %"+ rstack.top());
                //     code.push_back("\tcltd");
                //     code.push_back("\tidiv \t%"+rstack.top());
                //     rstack.push(R2);
                //     rstack.push(R);   tstack.pop(); tstack.pop();
                // }
                // else if(R!="eax" && rstack.top() =="edx"){
                //     code.push_back("\tpushl\t%eax");
                //     code.push_back("\tmovl \t%"+R+", %eax");
                //     code.push_back("\tpushl\t%edx");
                //     code.push_back("\tcltd");
                //     code.push_back("\tidiv \t%esp");
                //     code.push_back("\tmovl \t%eax, %"+R);
                //     code.push_back("\tmovl \t4(%esp), %eax");
                //     rstack.push(R);   tstack.pop();
                //     code.push_back("\taddl \t$8, %esp");
                // }
                // else{
                //     code.push_back("\tpushl\t%eax");
                //     code.push_back("\tmovl \t%"+R+", %eax");
                //     code.push_back("\tpushl\t%edx");
                //     code.push_back("\tcltd");
                //     code.push_back("\tidiv \t%"+rstack.top());
                //     code.push_back("\tmovl \t%eax, %"+R);
                //     code.push_back("\tmovl \t4(%esp), %eax");
                //     code.push_back("\tmovl \t8(%esp), %edx");
                //     rstack.push(R);   tstack.pop();
                //     code.push_back("\taddl \t$8, %esp");
                // }

            }
            if(op=="MULT_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                code.push_back("\timull\t%"+rstack.top()+", %"+R);
                // cout<<"\taddl \t%"<<rstack.top()<<", %"<< R <<"\n";
                rstack.push(R);   tstack.pop();
           }
            if(op=="MINUS_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                if(left->type->deref_level + left->type->array_level > 0 && right->type->deref_level + right->type->array_level > 0){
                    int size;
                    size = 4;
                    if(left->type->isStruct){
                        string type = "struct " + left->type->type;
                        size = gststruct.getStructEntry(type)->size;
                    }

                    code.push_back("\tsubl \t%"+rstack.top()+", %"+R);


                    code.push_back("\tpushl\t%edx");
                    code.push_back("\tpushl\t%eax");
                    code.push_back("\tpushl\t$" + to_string(size));

                    code.push_back("\tmovl \t%"+R+", %eax");
                    code.push_back("\tcltd");
                    code.push_back("\tidivl \t(%esp)");

                    code.push_back("\tpushl\t%eax");
                    code.push_back("\tmovl \t12(%esp), %edx");
                    code.push_back("\tmovl \t8(%esp), %eax");
                    code.push_back("\tpopl \t%" + R);
                    code.push_back("\taddl \t$12, %esp");

                    rstack.push(R);
                    tstack.pop();
                    return;

                }
                if(left->type->deref_level + left->type->array_level > 0){
                    int size;
                    size = 4;
                    if(left->type->isStruct){
                        string type = "struct " + left->type->type;
                        size = gststruct.getStructEntry(type)->size;
                    }
                    code.push_back("\timull\t$"+to_string(size)+", %"+rstack.top());
                }
                code.push_back("\tsubl \t%"+rstack.top()+", %"+R);
                // cout<<"\tsubl \t%"<<rstack.top()<<", %"<< R <<"\n";
                rstack.push(R);   tstack.pop();
            }
            if(op=="OR_OP"){
                left->gencode(symbtab);
                code.push_back("\tcmpl \t$0, %" + rstack.top());
                int true_label = lcount ;
                lcount++;
                code.push_back("\tjne .L"+to_string(true_label));
                right->gencode(symbtab);
                code.push_back("\tcmpl \t$0, %" + rstack.top());
                int false_label = lcount ;
                lcount++;
                code.push_back("\tje .L"+to_string(false_label));                
                code.push_back(".L"+to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + rstack.top());
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L"+to_string(false_label)+":");
                code.push_back("\tmovl \t$0, %" + rstack.top());
                code.push_back(".L"+to_string(next_label)+":");
            }
            if(op=="AND_OP"){
                left->gencode(symbtab);
                code.push_back("\tcmpl \t$0, %" + rstack.top());
                int false_label = lcount ;
                lcount++;
                code.push_back("\tje .L"+to_string(false_label));
                right->gencode(symbtab);
                code.push_back("\tcmpl \t$0, %" + rstack.top());
                int true_label = lcount ;
                lcount++;
                code.push_back("\tjne .L"+to_string(true_label));                
                code.push_back(".L"+to_string(false_label)+":");
                code.push_back("\tmovl \t$0, %" + rstack.top());
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L"+ to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + rstack.top());
                code.push_back(".L"+ to_string(next_label)+":");
            }
            if(op=="EQ_OP_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                code.push_back("\tcmpl \t%" + rstack.top() + ", %" + R );
                int true_label = lcount ;
                lcount++;
                code.push_back("\tje .L"+ to_string(true_label));
                code.push_back("\tmovl \t$0, %" + R);
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L" + to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + R);
                code.push_back(".L"+to_string(next_label)+":");
                rstack.push(R);   tstack.pop();
            }
            if(op=="NE_OP_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                code.push_back("\tcmpl \t%" + rstack.top() + ", %" + R );
                int true_label = lcount ;
                lcount++;
                code.push_back("\tjne .L"+ to_string(true_label));
                code.push_back("\tmovl \t$0, %" + R);
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L" + to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + R);
                code.push_back(".L"+to_string(next_label)+":");
                rstack.push(R);   tstack.pop();
            }
            if(op=="LT_OP_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                code.push_back("\tcmpl \t%" + rstack.top() + ", %" + R );
                int true_label = lcount ;
                lcount++;
                code.push_back("\tjl .L"+ to_string(true_label));
                code.push_back("\tmovl \t$0, %" + R);
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L" + to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + R);
                code.push_back(".L"+to_string(next_label)+":");
                rstack.push(R);   tstack.pop();
            }
            if(op=="LE_OP_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                code.push_back("\tcmpl \t%" + rstack.top() + ", %" + R );
                int true_label = lcount ;
                lcount++;
                code.push_back("\tjle .L"+ to_string(true_label));
                code.push_back("\tmovl \t$0, %" + R);
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L" + to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + R);
                code.push_back(".L"+to_string(next_label)+":");
                rstack.push(R);   tstack.pop();
            }            
            if(op=="GT_OP_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                code.push_back("\tcmpl \t%" + rstack.top() + ", %" + R );
                int true_label = lcount ;
                lcount++;
                code.push_back("\tjg .L"+ to_string(true_label));
                code.push_back("\tmovl \t$0, %" + R);
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L" + to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + R);
                code.push_back(".L"+to_string(next_label)+":");
                rstack.push(R);   tstack.pop();
            }
            if(op=="GE_OP_INT"){
                left->gencode(symbtab);
                string R = rstack.top();   tstack.push(R);
                rstack.pop();
                right->gencode(symbtab);
                code.push_back("\tcmpl \t%" + rstack.top() + ", %" + R );
                int true_label = lcount ;
                lcount++;
                code.push_back("\tjge .L"+ to_string(true_label));
                code.push_back("\tmovl \t$0, %" + R);
                int next_label = lcount ;
                lcount++;                                
                code.push_back("\tjmp .L" + to_string(next_label));
                code.push_back(".L" + to_string(true_label)+":");
                code.push_back("\tmovl \t$1, %" + R);
                code.push_back(".L"+to_string(next_label)+":");
                rstack.push(R);   tstack.pop();
            }
            int i=0;
            while(i<stored){
                code.push_back("\tpopl \t%" + rstack.top());
                tstack.push(rstack.top());
                rstack.pop();
                i++;
            }
            
        }; 
    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"op_binary\": {\n";
        printblanks(blanks+1);
        cout<<"\"op\": \""<< op <<"\"\n";
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"left\":\n";
        left->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"right\":\n";
        right->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class op_unary_astnode: public exp_astnode{
public:
    string op;
    exp_astnode *exp;

    op_unary_astnode(string op, exp_astnode * exp){
        this->exp = exp;
        this->op = op;
    }

    void gencode(LocalSymbTab* symbtab){
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        if(op == "PP"){
            bool temp_addneeded = addneeded; 
            addneeded = true;
            // int loc = symbtab->getEntry(exp->identifier_id)->offset;
            exp->gencode(symbtab);

            addneeded = temp_addneeded; 
            
            code.push_back("\taddl \t$1, (%" + rstack.top()+")");
            code.push_back("\tmovl \t(%"+rstack.top()+"), %"+rstack.top());
            code.push_back("\tsubl \t$1, %"+rstack.top());
        }    
        else if(op=="UMINUS"){
            exp->gencode(symbtab);
            code.push_back("\timull \t$-1, %"+rstack.top());
        }
        else if(op=="NOT"){
            exp->gencode(symbtab);
            code.push_back("\tcmpl \t$0, %"+rstack.top());
            int temp_label=lcount;
            lcount++;
            code.push_back("\tje .L" + to_string(temp_label));
            code.push_back("\tmovl \t$0, %"+rstack.top());
            int next_label=lcount;
            lcount++;
            code.push_back("\tjmp .L" + to_string(next_label));
            code.push_back(".L"+ to_string(temp_label) + ":");
            code.push_back("\tmovl \t$1, %"+rstack.top());
            code.push_back(".L"+ to_string(next_label) + ":");
        }
        else if(op=="ADDRESS"){
            bool temp_addneeded = addneeded;
            addneeded = true;
            if(!exp->isId && exp->type->array_level>0){
               check_param = true;
            }
            exp->gencode(symbtab);
            check_param = false;
            addneeded = temp_addneeded;
        }
        else if(op=="DEREF"){
            bool tempaddneeded = addneeded;
            addneeded = false;
            exp->gencode(symbtab);
            addneeded = tempaddneeded;
            if(addneeded){
                return;
            }
            else{
                code.push_back("\tmovl \t(%"+rstack.top()+"), %"+rstack.top());
            }
        }
    }; 

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"op_unary\": {\n";
        printblanks(blanks+1);
        cout<<"\"op\": \""<< op <<"\"\n";
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"child\":\n";
        exp->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }

};

class arrayref_astnode: public ref_astnode{
public:
    exp_astnode *left;
    exp_astnode *right;

    arrayref_astnode(exp_astnode * left, exp_astnode *right){
        this->left = left;
        this->right = right;
        this->isArray = true;
    }
    
    void gencode(LocalSymbTab* symbtab){
        
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }

        arraylevel++;
        int temp_level = arraylevel;
        int temp_addneeded = addneeded;
        int temp_checkparam = check_param;
        check_param = true;
        addneeded = true;
        left->gencode(symbtab);
        check_param = temp_checkparam;
        addneeded = temp_addneeded;
        arraylevel = temp_level;

        int arrsize = getexpSize(type);
        
        tstack.push(rstack.top());
        string R = rstack.top();
        rstack.pop();
        int temp_level_2 = arraylevel;
        int temp_addneeded_2 = addneeded;
        addneeded = false;
        arraylevel = 0;
        right->gencode(symbtab);
        arraylevel = temp_level_2;
        addneeded = temp_addneeded;

        code.push_back("\timull \t$"+to_string(arrsize)+", %" + rstack.top());
        code.push_back("\taddl \t%"+rstack.top()+", %"+R);
        rstack.push(R);
        tstack.pop();

        arraylevel--;
        
        if(arraylevel==0){
            const EntryLocal* local_entry = symbtab->getEntry(left->type->name);    
            if(!addneeded){
                code.push_back("\tmovl \t(%"+rstack.top()+"), %"+rstack.top());
           }
        }

        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
    };
    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"arrayref\": {\n";
        printblanks(blanks+1);
        cout<<"\"array\": \n";
        left->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"index\": \n";
        right->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};


class identifier_astnode: public ref_astnode{
    public: 
       
        identifier_astnode(string id){
            identifier_id=id;
            isId = true;
        };

        void gencode(LocalSymbTab* symbtab){
            int stored=0;
            while(rstack.size()<1){
                code.push_back("\tpushl \t%" + tstack.top());
                rstack.push(tstack.top());
                tstack.pop();
                stored++;
            }
            if(addneeded){
                const EntryLocal* local_entry = symbtab->getEntry(identifier_id);
                int loc = local_entry->offset;
                code.push_back("\tmovl \t$"+to_string(loc)+", %"+rstack.top());
                code.push_back("\taddl \t%ebp, %"+rstack.top());
                if(local_entry->scope=="param" && check_param){
                    code.push_back("\tmovl \t(%"+rstack.top()+"), %"+rstack.top());                
                }
                return;
            }
            int loc = symbtab->getEntry(identifier_id)->offset;
            code.push_back("\tmovl \t"+to_string(loc)+"(%ebp), %"+rstack.top());

            int i=0;
            while(i<stored){
                code.push_back("\tpopl \t%" + rstack.top());
                tstack.push(rstack.top());
                rstack.pop();
                i++;
            }
        }; 
        void print(int blanks){
            for(int i=0;i<blanks;i++){
                cout<<"\t";
            }
            cout<<"{ \"identifier\": \""<< identifier_id <<"\" }\n";
        }
        
};

class funcall_astnode: public exp_astnode{
    public:
    vector <exp_astnode*> arguments;

    funcall_astnode(exp_astnode * exp){
        arguments.push_back(exp);
    }

    void insert(exp_astnode * exp){
        arguments.push_back(exp);
    }

    void insert_front(exp_astnode * exp){
        arguments.insert(arguments.begin(),exp);
    }

    void gencode(LocalSymbTab* symbtab){
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        if(arguments.back()->identifier_id=="printf"){
            int pushcount = 0;
            for(int i = arguments.size()-2; i>=1;i--){
                pushcount+=1;
                arguments[i]->gencode(symbtab);
                code.push_back("\tpushl\t%"+rstack.top());
            }
            code.push_back("\tpushl\t");
            arguments[0]->gencode(symbtab);
            code.push_back("\tcall \tprintf");
            code.push_back("\taddl\t$"+to_string(pushcount*4)+", %esp");
        }
        else{         
                 
            /*saving caller saved registers*/
            int num_mem_regs = tstack.size(), i=0; 
            while(i<num_mem_regs){
                code.push_back("\tpushl\t%"+tstack.top());
                rstack.push(tstack.top());
                tstack.pop();
                i++;
            }
            /*making space for return value*/
            const EntryGlobal* receivedEntry = gstfun.getFuncEntry(arguments.back()->identifier_id);
            // int return_size = receivedEntry->size;
            // void,struct not taken care of
            int return_size; 
            if(receivedEntry->type=="void"){
                return_size = 0;
            }
            else{
                return_size = 4;
            }
            code.push_back("\tsubl\t$"+to_string(return_size)+", %esp");
            /*pushing parameters*/
            int sizepop = 0;
            for(int i = 0; i<arguments.size()-1;i++){
                if(arguments[i]->type->isStruct  && (arguments[i]->type->array_level+arguments[i]->type->deref_level)==0){
                    bool temp_addneeded = addneeded; 
                    addneeded = true;
                    arguments[i]->gencode(symbtab);
                    addneeded = temp_addneeded;
                    string type = "struct " + arguments[i]->type->type;
                    int size = gststruct.getStructEntry(type)->size;
                    code.push_back("\taddl \t$"+to_string(size)+", %"+rstack.top());
                    for(int i = 0; i<size/4; i++){
                        code.push_back("\tsubl \t$4, %"+rstack.top());
                        code.push_back("\tpushl\t(%"+rstack.top()+")");
                    }
                    sizepop += size;
                    continue;
                }
                else{
                    sizepop += 4;
                }
                bool temp_addneeded = addneeded; 
                if(arguments[i]->type->array_level > 0){
                    addneeded = true;
                }
                arguments[i]->gencode(symbtab);
                addneeded = temp_addneeded;
                code.push_back("\tpushl\t%"+rstack.top());
            }

            code.push_back("\tcall \t"+arguments.back()->identifier_id);
            code.push_back("\taddl\t$"+to_string(sizepop)+", %esp");
            
            i=0;
            while(i<num_mem_regs){
                tstack.push(rstack.top());
                rstack.pop();
                i++;
            }
            if(return_size>0){
                code.push_back("\tpopl\t%"+rstack.top());
            }
            i=0;
            while(i<num_mem_regs){
                rstack.push(tstack.top());
                tstack.pop();
                i++;
            }
            i=0;
            while(i<num_mem_regs){
                code.push_back("\tpopl\t%"+rstack.top());
                tstack.push(rstack.top());
                rstack.pop();
                i++;
            }

        }
        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }

        
    }

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"funcall\": {\n";
        printblanks(blanks+1);
        cout<<"\"fname\": \n";
        arguments.back()->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"params\": [\n";
        if(arguments.size()>1){
            if(arguments.size()>0){
                arguments[0]->print(blanks+2);
            }
        }
        for(int i = 1; i < arguments.size()-1;i++){
            printblanks(blanks+2);
            cout<<",\n";
            arguments[i]->print(blanks+2);
        }
        printblanks(blanks+1);
        cout<<"]\n";
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }


};


class assignE_astnode: public exp_astnode{
public:
    exp_astnode *left;
    exp_astnode *right;

    assignE_astnode(exp_astnode * left, exp_astnode *right){
        this->left = left;
        this->right = right;
    }

    void gencode(LocalSymbTab* symbtab){
        int stored =0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        if(left->type->isStruct && left->type->deref_level == 0){
            bool temp_addneeded = addneeded;             
            addneeded = true;
            left->gencode(symbtab);
            string R = rstack.top();
            tstack.push(R);
            rstack.pop();
            right->gencode(symbtab);
            string R2 = rstack.top();
            rstack.pop();
            tstack.push(R2);
            addneeded = temp_addneeded;

            string type = "struct " + left->type->type;
            
            int size = gststruct.getStructEntry(type)->size;
            

            for(int i = 0; i < size/4-1; i++){
                code.push_back("\tmovl \t(%"+R2+"), %"+rstack.top());
                code.push_back("\tmovl \t%"+rstack.top()+", (%"+R+")");
                code.push_back("\taddl \t$4, %"+R);
                code.push_back("\taddl \t$4, %"+R2);
            }
            code.push_back("\tmovl \t(%"+R2+"), %"+rstack.top());
            code.push_back("\tmovl \t%"+rstack.top()+", (%"+R+")");
            rstack.push(R);
            tstack.pop();
            rstack.push(R2);
            tstack.pop();
        }
        else{
            int loc;
            if(left->isId){
                loc = symbtab->getEntry(left->identifier_id)->offset;
                right->gencode(symbtab);
                code.push_back("\tmovl \t%"+rstack.top()+", "+to_string(loc)+"(%ebp)");
            }
            else{
                bool temp_addneeded = addneeded; 
                addneeded = true;
                left->gencode(symbtab);
                addneeded = temp_addneeded;
                string R = rstack.top();
                rstack.pop();
                tstack.push(R);
                right->gencode(symbtab);
                
                code.push_back("\tmovl \t%"+rstack.top()+", "+"(%"+R+")");
                rstack.push(R);
                tstack.pop();
            }
        }
        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
        // if(right->isconstant){
        //     cout<<"\tmovl \t";
        //     right->gencode(gstfun, gststruct, symbtab);
        //     cout<<", "<<loc<<"(%ebp)\n";
        // }
        // else{
        //     right->gencode(gstfun, gststruct, symbtab);
        //     cout<<"\tmovl \t%eax"<<", "<<loc<<"(%ebp)\n";
        // }
        
        // cout<<"\tmovl \t%"<<rstack.top()<<", "<<loc<<"(%ebp)\n";
        }; 

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"assignE\": {\n";
        printblanks(blanks+1);
        cout<<"\"left\":\n";
        left->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"right\":\n";
        right->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class seq_astnode: public statement_astnode{
    public:
    vector<statement_astnode *> statementlist;

    seq_astnode(){

    }

    seq_astnode(statement_astnode * init){
        statementlist.push_back(init);
    }

    void insert(statement_astnode * newentry){
        statementlist.push_back(newentry);
    }

    void gencode(LocalSymbTab* symbtab){
        for(int i = 0; i < statementlist.size();i++){
            statementlist[i]->gencode(symbtab);
        }
    }

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"seq\": [\n";
        if(statementlist.size()>0){
            statementlist[0]->print(blanks+1);
        }
        for(int i = 1; i < statementlist.size();i++){
            printblanks(blanks+1);
            cout<<",\n";
            statementlist[i]->print(blanks+1);
        }
        printblanks(blanks);
        cout<<"]\n";
        printblanks(blanks);
        cout<<"}\n";
    }

};

class member_astnode: public ref_astnode{
    public:
    exp_astnode * exp;
    identifier_astnode * id;

    member_astnode(exp_astnode* exp, identifier_astnode*id){
        this->exp=exp;
        this->id = id;
        isMember = true;
    }

    //galat hai abhi
    void gencode(LocalSymbTab* symbtab){
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }

        int loc;
        
        if(exp->isMember){
            structlevel++;
            
            exp->gencode(symbtab);

           
        }
        else{
           
            
            int temp_addneeded = addneeded;
            addneeded = true;
            exp->gencode(symbtab);
            addneeded = temp_addneeded;
            
            // string type = symbtab->getEntry(exp->identifier_id)->type;
            
            // loc = symbtab->getEntry(exp->identifier_id)->offset;
            
            // code.push_back("\tmovl \t$"+to_string(loc)+", %"+rstack.top());
            structlevel++;
            
            
        }
        string structleft = "struct "+exp->type->type;
        int shift = gststruct.getStructEntry(structleft)->symbtab->getEntry(id->identifier_id)->offset;
        

        code.push_back("\taddl \t$"+to_string(shift)+", %"+rstack.top());
        structlevel--;

      
        
        if(structlevel==0){
            if(!addneeded){
                code.push_back("\tmovl \t(%"+rstack.top()+"), %"+rstack.top());
           }
        }


        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
        
        }; 
    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"member\": {\n";
        printblanks(blanks+1);
        cout<<"\"struct\": \n";
        exp->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"field\": \n";
        id->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class arrow_astnode: public ref_astnode{
    public:
    exp_astnode * exp;
    identifier_astnode * id;

    arrow_astnode(exp_astnode* exp, identifier_astnode*id){
        this->exp=exp;
        this->id = id;
    }

    void gencode(LocalSymbTab* symbtab){
        
        int addneedold = addneeded;
        addneeded = false;
            exp->gencode(symbtab);
            addneeded = addneedold;
            string type = "struct " + exp->type->type;
            
            int shift = gststruct.getStructEntry(type)->symbtab->getEntry(id->identifier_id)->offset;

            code.push_back("\taddl \t$"+to_string(shift)+", %"+rstack.top());

            if(!addneeded){
                code.push_back("\tmovl \t(%"+rstack.top()+"), %"+rstack.top());
            }
        }; 

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"arrow\": {\n";
        printblanks(blanks+1);
        cout<<"\"pointer\": \n";
        exp->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"field\": \n";
        id->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class for_astnode: public statement_astnode{
    public:
    exp_astnode * init;
    exp_astnode * gaurd;
    exp_astnode * step;
    statement_astnode * body;

    for_astnode(exp_astnode * init, exp_astnode * gaurd, exp_astnode * step, statement_astnode * body){
        this->init=init;
        this->gaurd = gaurd;
        this->step = step;
        this->body = body;
    }

    void gencode(LocalSymbTab* symbtab){
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        init->gencode(symbtab);
        int top_label = lcount ;
        lcount++;
        code.push_back(".L"+to_string(top_label)+":");
        gaurd->gencode(symbtab);
        code.push_back("\tcmpl \t$0, %" + rstack.top());
        int exit_label = lcount ;
        lcount++;
        code.push_back("\tje .L"+to_string(exit_label));
        body->gencode(symbtab);
        step->gencode(symbtab);
        code.push_back("\tjmp .L" + to_string(top_label));
        code.push_back(".L"+to_string(exit_label)+":");
        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
    }; 
    
    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"for\": {\n";
        printblanks(blanks+1);
        cout<<"\"init\": \n";
        init->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"guard\": \n";
        gaurd->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"step\": \n";
        step->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"body\": \n";
        body->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class while_astnode: public statement_astnode{
    public: 
    exp_astnode * cond;
    statement_astnode* body;

    while_astnode(exp_astnode * cond, statement_astnode * body){
        this->cond = cond;
        this->body = body;
    }

    void gencode(LocalSymbTab* symbtab){
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        int top_label = lcount ;
        lcount++;
        code.push_back(".L"+to_string(top_label)+":");
        cond->gencode(symbtab);
        code.push_back("\tcmpl \t$0, %" + rstack.top());
        int exit_label = lcount ;
        lcount++;
        code.push_back("\tje .L"+to_string(exit_label));
        body->gencode(symbtab);
        code.push_back("\tjmp .L" + to_string(top_label));
        code.push_back(".L"+to_string(exit_label)+":");
        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
        }; 

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"while\": {\n";
        printblanks(blanks+1);
        cout<<"\"cond\": \n";
        cond->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"stmt\": \n";
        body->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class if_astnode: public statement_astnode{
    public:
    exp_astnode * cond;
    statement_astnode * thenstat;
    statement_astnode * elsestat;

    if_astnode(exp_astnode * cond, statement_astnode* thenstat, statement_astnode * elsestat){
        this->cond = cond;
        this->thenstat = thenstat;
        this->elsestat = elsestat;
    }

    void gencode(LocalSymbTab* symbtab){
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        cond->gencode(symbtab);
        code.push_back("\tcmpl \t$0, %" + rstack.top());
        int false_label = lcount ;
        lcount++;
        code.push_back("\tje .L"+to_string(false_label));
        thenstat->gencode(symbtab);
        int next_label = lcount ;
        lcount++;                                
        code.push_back("\tjmp .L" + to_string(next_label));
        code.push_back(".L"+to_string(false_label)+":");
        elsestat->gencode(symbtab);
        code.push_back(".L"+to_string(next_label)+":");
        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
    } 
        
    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"if\": {\n";
        printblanks(blanks+1);
        cout<<"\"cond\": \n";
        cond->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"then\": \n";
        thenstat->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"else\": \n";
        elsestat->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class empty_astnode: public statement_astnode{
    public:
    empty_astnode(){};
    //galat hai abhi
    void gencode(LocalSymbTab* symbtab){
            for(int i  =0; i<10; i++)
            {
                int x = 0; 
            }
        }; 
    void print(int blanks){
        printblanks(blanks);
        cout<<"\"empty\"\n";
    }
};

class return_astnode: public statement_astnode{
    public:
    exp_astnode * return_node;

    return_astnode(exp_astnode* ret){
        this->return_node = ret;
    }
    
    void gencode(LocalSymbTab* symbtab){
            int stored = 0;
            while(rstack.size()<2){
                code.push_back("\tpushl \t%" + tstack.top());
                rstack.push(tstack.top());
                tstack.pop();
                stored++;
            }
            int localsize = 0;
            int paramsize = 0;
            for(auto itr : symbtab->Entries){
		        if(itr.scope=="local")localsize += itr.size;
		        if(itr.scope=="param")paramsize += itr.size;
	        }
            return_node->gencode(symbtab);
        	if(localsize > 0){
		        code.push_back("\taddl\t$"+to_string(localsize)+", %esp\n");
	        }
            int loc = 8 + paramsize;
            code.push_back("\tmovl \t%"+rstack.top()+", "+to_string(loc)+"(%ebp)");        
	        code.push_back("\tleave\n\tret");
            int i=0;
            while(i<stored){
                code.push_back("\tpopl \t%" + rstack.top());
                tstack.push(rstack.top());
                rstack.pop();
                i++;
            }
        };

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"return\": \n";
   
        return_node->print(blanks+1);
        
        printblanks(blanks);
        cout<<"}\n";
    }

};


class assignS_astnode: public statement_astnode{
public:
    exp_astnode *left;
    exp_astnode *right;

    assignS_astnode(exp_astnode * left, exp_astnode *right){
        this->left = left;
        this->right = right;
    }

    void gencode(LocalSymbTab* symbtab){
       
        int stored =0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        if(left->type->isStruct && left->type->deref_level == 0){
            bool temp_addneeded = addneeded;             
            addneeded = true;
            left->gencode(symbtab);
            string R = rstack.top();
            tstack.push(R);
            rstack.pop();
            right->gencode(symbtab);
            string R2 = rstack.top();
            rstack.pop();
            tstack.push(R2);
            addneeded = temp_addneeded;

            string type = "struct " + left->type->type;
            
            int size = gststruct.getStructEntry(type)->size;
            

            for(int i = 0; i < size/4-1; i++){
                code.push_back("\tmovl \t(%"+R2+"), %"+rstack.top());
                code.push_back("\tmovl \t%"+rstack.top()+", (%"+R+")");
                code.push_back("\taddl \t$4, %"+R);
                code.push_back("\taddl \t$4, %"+R2);
            }
            code.push_back("\tmovl \t(%"+R2+"), %"+rstack.top());
            code.push_back("\tmovl \t%"+rstack.top()+", (%"+R+")");
            rstack.push(R);
            tstack.pop();
            rstack.push(R2);
            tstack.pop();
        }
        else{
            int loc;
            if(left->isId){
                loc = symbtab->getEntry(left->identifier_id)->offset;
                right->gencode(symbtab);
                code.push_back("\tmovl \t%"+rstack.top()+", "+to_string(loc)+"(%ebp)");
            }
            else{
                bool temp_addneeded = addneeded; 
                addneeded = true;
                left->gencode(symbtab);
                addneeded = temp_addneeded;
                string R = rstack.top();
                rstack.pop();
                tstack.push(R);
                right->gencode(symbtab);
                
                code.push_back("\tmovl \t%"+rstack.top()+", "+"(%"+R+")");
                rstack.push(R);
                tstack.pop();
            }
        }
        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
        // if(right->isconstant){
        //     cout<<"\tmovl \t";
        //     right->gencode(gstfun, gststruct, symbtab);
        //     cout<<", "<<loc<<"(%ebp)\n";
        // }
        // else{
        //     right->gencode(gstfun, gststruct, symbtab);
        //     cout<<"\tmovl \t%eax"<<", "<<loc<<"(%ebp)\n";
        // }
        
        // cout<<"\tmovl \t%"<<rstack.top()<<", "<<loc<<"(%ebp)\n";
    }; 

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"assignS\": {\n";
        printblanks(blanks+1);
        cout<<"\"left\":\n";
        left->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"right\":\n";
        right->print(blanks+1);
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};

class proccall_astnode: public statement_astnode{
    public:
    vector <exp_astnode*> arguments;

    proccall_astnode(funcall_astnode * exp){
        arguments = exp->arguments;            
    }

    void gencode(LocalSymbTab* symbtab){
        int stored = 0;
        while(rstack.size()<2){
            code.push_back("\tpushl \t%" + tstack.top());
            rstack.push(tstack.top());
            tstack.pop();
            stored++;
        }
        if(arguments.back()->identifier_id=="printf"){
            int pushcount = 1;
            for(int i = arguments.size()-2; i>=1;i--){
                pushcount+=1;
                // if(arguments[i]->isconstant){
                //     cout<<"\tpushl\t";
                //     arguments[i]->gencode(symbtab, rstack, code, lcount);
                //     cout<<"\n";
                // }
                // else{
                //     arguments[i]->gencode(symbtab, rstack, code, lcount);
                //     cout<<"\tpushl\t%eax\n";
                // }
                arguments[i]->gencode(symbtab);
                // arguments[i]->gencode(symbtab);
                code.push_back("\tpushl\t%"+rstack.top());
            }
            code.push_back("\tpushl\t");
            arguments[0]->gencode(symbtab);
            code.push_back("\tcall \tprintf");
            code.push_back("\taddl\t$"+to_string(pushcount*4)+", %esp");
        }
        else{         
            /*saving caller saved registers*/
            int num_mem_regs = tstack.size(), i=0; 
            while(i<num_mem_regs){
                code.push_back("\tpushl\t%"+tstack.top());
                rstack.push(tstack.top());
                tstack.pop();
                i++;
            }
            /*making space for return value*/
            const EntryGlobal* receivedEntry = gstfun.getFuncEntry(arguments.back()->identifier_id);
            // int return_size = receivedEntry->size;
            // void,struct not taken care of
            int return_size; 
            if(receivedEntry->type=="void"){
                return_size = 0;
            }
            else{
                return_size = 4;
            }
            code.push_back("\tsubl\t$"+to_string(return_size)+", %esp");
            /*pushing parameters*/
            int sizepop = 0;
            for(int i = 0; i<arguments.size()-1;i++){
                if(arguments[i]->type->isStruct  && (arguments[i]->type->array_level+arguments[i]->type->deref_level)==0){
                    bool temp_addneeded = addneeded; 
                    addneeded = true;
                    arguments[i]->gencode(symbtab);
                    addneeded = temp_addneeded;
                    string type = "struct " + arguments[i]->type->type;
                    int size = gststruct.getStructEntry(type)->size;
                    code.push_back("\taddl \t$"+to_string(size)+", %"+rstack.top());
                    for(int i = 0; i<size/4; i++){
                        code.push_back("\tsubl \t$4, %"+rstack.top());
                        code.push_back("\tpushl\t(%"+rstack.top()+")");
                    }
                    sizepop += size;
                    continue;
                }
                else{
                    sizepop += 4;
                }
                bool temp_addneeded = addneeded; 
                if(arguments[i]->type->array_level > 0){
                    addneeded = true;
                }
                arguments[i]->gencode(symbtab);
                addneeded = temp_addneeded;
                code.push_back("\tpushl\t%"+rstack.top());
            }

            code.push_back("\tcall \t"+arguments.back()->identifier_id);
            code.push_back("\taddl\t$"+to_string(sizepop)+", %esp");
            
            i=0;
            while(i<num_mem_regs){
                tstack.push(rstack.top());
                rstack.pop();
                i++;
            }
            if(return_size>0){
                code.push_back("\tpopl\t%"+rstack.top());
            }
            i=0;
            while(i<num_mem_regs){
                rstack.push(tstack.top());
                tstack.pop();
                i++;
            }
            i=0;
            while(i<num_mem_regs){
                code.push_back("\tpopl\t%"+rstack.top());
                tstack.push(rstack.top());
                rstack.pop();
                i++;
            }
        }
        int i=0;
        while(i<stored){
            code.push_back("\tpopl \t%" + rstack.top());
            tstack.push(rstack.top());
            rstack.pop();
            i++;
        }
    }

    void print(int blanks){
        printblanks(blanks);
        cout<<"{ \"proccall\": {\n";
        printblanks(blanks+1);
        cout<<"\"fname\": \n";
        arguments.back()->print(blanks+1);
        printblanks(blanks+1);
        cout<<",\n";
        printblanks(blanks+1);
        cout<<"\"params\": [\n";
        if(arguments.size()>1){
            if(arguments.size()>0){
                arguments[0]->print(blanks+2);
            }
        }
        for(int i = 1; i < arguments.size()-1;i++){
            printblanks(blanks+2);
            cout<<",\n";
            arguments[i]->print(blanks+2);
        }
        printblanks(blanks+1);
        cout<<"]\n";
        printblanks(blanks);
        cout<<"}\n";
        printblanks(blanks);
        cout<<"}\n";
    }
};