
#include "scanner.hh"
#include "parser.tab.hh"
#include <fstream>
using namespace std;

SymbTab gst, gstfun, gststruct; 
map<string,map<string,int>> LocalStrings;
stack<string> rstack, tstack;
string filename;
vector<string> code;
bool addneeded = false;
int structlevel = 0, arraylevel = 0;
int lcount = 0;
bool check_param = 0;

extern std::map<string,abstract_astnode*> ast;
std::map<std::string, std::string> predefined {
            {"printf", "void"},
            {"scanf", "void"}
        };
int main(int argc, char **argv)
{
	using namespace std;
	fstream in_file, out_file;
	

	in_file.open(argv[1], ios::in);

	IPL::Scanner scanner(in_file);

	IPL::Parser parser(scanner);

#ifdef YYDEBUG
	parser.set_debug_level(1);
#endif
parser.parse();
// create gstfun with function entries only

for (const auto &entry : gst.Entries)
{
	if (entry.second.varfun == "fun")
	gstfun.Entries.insert({entry.first, entry.second});
}
// create gststruct with struct entries only

for (const auto &entry : gst.Entries)
{
	if (entry.second.varfun == "struct")
	gststruct.Entries.insert({entry.first, entry.second});
}

// start the JSON printing

// cout << "{\"globalST\": " << endl;
// gst.printgst();
// cout << "," << endl;

rstack.push("edi");
rstack.push("esi");
rstack.push("edx");
rstack.push("ecx");
rstack.push("ebx");
rstack.push("eax");

cout<<"\t.file"<<"\t\""<<argv[1]<<"\"\n";
cout<<"\t.text\n";


for (auto it = gstfun.Entries.begin(); it != gstfun.Entries.end(); ++it)
{	
	
	code.push_back("\t.section\t.rodata");
	for (auto i: LocalStrings[it->first]){
		code.push_back(".LC"+to_string(i.second)+":\n\t.string "+i.first);
	}
	code.push_back("\t.text");
	code.push_back("\t.globl\t"+it->first);
	code.push_back("\t.type\t"+it->first+", @function");
	code.push_back(it->first+": ");
	code.push_back("\tpushl\t%ebp");
	code.push_back("\tmovl\t%esp, %ebp");

	int localsize = 0;
	for(auto itr : it->second.symbtab->Entries){
		if(itr.scope=="local")localsize += itr.size;
	}

	if(localsize > 0){
		code.push_back("\tsubl\t$"+to_string(localsize)+", %esp");
	}

	ast[it->first]->gencode(it->second.symbtab);

	if(ast[it->first]->isMain==1 && ast[it->first]->hasReturn==0){
	        code.push_back("\tmovl \t$0,%eax");
	        code.push_back("\tleave\n\tret");
	}
	if(ast[it->first]->isMain!=1 && ast[it->first]->hasReturn==0){
	        code.push_back("\tleave\n\tret");
	}

	code.push_back("\t.size\t" + it->first + ", .-" + it->first);
}
	for(auto itr: code){
		cout<<itr<<endl;
	}
}
