#include <iostream>
#include <string>
#include <fstream>
#include <vector>

int main(){
  std::fstream fp("rules.txt");
  std::ofstream out_file("new_rules.txt");	
  if(!fp.good()){
    std::cerr << "Can't open file: rules.txt\n";
    return 0;
  }

  if(!out_file.good()){
    std::cerr << "Can't open out file for writing: new_rules.txt\n";
    return 0;
  }

	{
		while(fp.eof() == false){
			std::vector<char> buf;
			buf.reserve(128000);
			std::fill(buf.begin(),buf.end(),'\1');
			fp.getline((char*)&buf[0],128001);
			
 			std::string line((char*)&buf[0]);
			std::cout << line << "\n";
		}
	}

}
