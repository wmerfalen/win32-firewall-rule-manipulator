#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

#define LINE_BUFFER_SIZE 2048
#define USAGE_STATUS_RETURN 1
#define hr(){ for(int i=0;i < 80;i++){ std::cout << "="; } std::cout << "\n"; }
#define ACTION_COLUMN 4     /* zero based */
#define ENABLED_COLUMN 3 /* zero based */
/* My compiler is g++ (Ming shell) on Windows. Unfortunatly, I don't
 * think it supports c++11, or c++14. So, this code is a bit old-fashioned.
 */
namespace rule {
  std::vector<std::string> columns;
  bool quiet;
  const short LOG = 0;
  const short WARN = 1;
  const short ERR = 2; 
  std::vector<std::string> explode_line(const std::string& line){
    std::string::size_type start = 0,end = 0;
    std::vector<std::string> parts;
    while((end = line.find('\t',start)) != std::string::npos){
      if(!quiet){
        std::cout << "start[" << line[start] << "] end[" << line[end] << "]\n";
      }
      parts.push_back(line.substr(start,end - start));
      start = end + 1;
    }
    return parts;
  }
  void deny_if_disabled(const std::vector<std::string>& cols,std::ios * out){
    std::string action = "";
    std::string enabled = "";
    for(int i=0;i < cols.size();i++){
      /* If ACTION_COLUMN is always greater than ENABLED_COLUMN, this code works.
       * Otherwise, this will most likely break
       */
      if(i == ENABLED_COLUMN){
        if(cols[i].compare("No") == 0){   /* if the rule is not enabled */
          action = "Deny";
          enabled = "Yes";
        }else{
          action = "Allow";
          enabled = "Yes";
        }
        (*out) << enabled.c_str() << "\t";
        continue; 
      }
      if(i == ACTION_COLUMN){
        (*out) << action.c_str() << "\t";
        continue;
      }
      (*out) << cols[i].c_str();
      if((i+1) != cols.size()){
        (*out) << "\t";
      }else{
        (*out) << "\n";
      }
    }
  }
  const char* prefix_level(short level){
    switch(level){
      case LOG: return "[LOG]:";
      case WARN: return "[WARN]:";
      case ERR: return "[ERROR]:";
      default: return "[INFO]:";
    }
  }
  int _report(const char* msg,short level){
    if(quiet && level < WARN){
      return level;
    }
    if(quiet && level == ERR){
      std::cout << prefix_level(level) << msg << "\n";
      return level;
    }
    if(!quiet){
      std::cout << prefix_level(level) << msg << "\n";
    }
    return level;
  }
  int report(std::string str,short level){
    return _report(str.c_str(),level);
  }
  template <typename T>
  std::string to_string(T t){
    std::stringstream ss;
    ss >> t;
    std::string output;
    ss << output;
    return output;
  }
};

std::string rdlne(std::fstream *fp,std::vector<char>& buf,std::size_t size){
  buf.reserve(size);
  std::fill(buf.begin(),buf.end(),'\0');
  fp->getline((char*)&buf[0],size-1);
  std::string line = (char*)&buf[0];
  return line;
}

int usage(const char** options,char* exe_name){
  char* ptr = (char*)options[0];
  std::cout << "Usage: " << exe_name << " ";
  while(*ptr){
    std::cout << ptr << ptr + 1;
    ptr += 2;
  }
  std::cout << "\n";
  return USAGE_STATUS_RETURN;
}

int report_error(const char** options,char* exe_name,const char* message){
  std::cout << "[error]: {" << message << "}\n";
  std::cout << "Use -h for help on usage\n";
  return usage(options,exe_name);
}

int main(int argc,char* argv[]){
  std::fstream *fp = NULL;
  std::fstream *out_file = NULL;	
  std::string in_file_name = "rules.txt",out_file_name = "new_rules.txt";
  rule::quiet = true;
  const char* options[] = { 
    "-v","Verbose mode",
    "-i <file>","Select an input file name",
    "-o <file>","Select an output file name",
    "-h","This help menu",
    NULL
  };

  int arg_ctr = 1;
  while(arg_ctr < argc){
    std::string current_arg = argv[arg_ctr++];
    if(current_arg.compare("-v")){
      rule::quiet = false;
      continue;
    }
    if(current_arg.compare("-i")){
      if(arg_ctr > argc){
        return report_error(options,argv[0],"-i requires a valid file name");
      }else{
        in_file_name = argv[arg_ctr];
      }
      continue;
    }
    if(current_arg.compare("-o")){
      if(arg_ctr > argc){
         return report_error(options,argv[0],"-o requires a valid file name");
      }else{
        out_file_name = argv[arg_ctr];
      }
      continue;
    }
  }//End while
 
  fp = new std::fstream(in_file_name.c_str(),std::ios::in);
  if(!fp->is_open()){
    delete fp;
    return rule::report(std::string("Could not open input file: ") + in_file_name,rule::ERR); 
  }

  out_file = new std::fstream("new_rules.txt",std::ios::out|std::ios::trunc);
  if(!out_file->is_open()){
    if(fp){
      fp->close();
      delete fp;
    }
    delete out_file;
    return rule::report(std::string("Could not open output file: ") + out_file_name,rule::ERR);
  }
  
  rule::report(std::string("Files opened: ") + in_file_name + "," + out_file_name,rule::LOG);

  /* Process the first line of the file (it is going to be the tab-delimited field names) */
  std::vector<char> buf;
  rule::columns = rule::explode_line(rdlne(fp,buf,LINE_BUFFER_SIZE));
  unsigned int line_number = 2;
  while(fp->eof() == false){
    std::vector<std::string> exploded = rule::explode_line(rdlne(fp,buf,LINE_BUFFER_SIZE));
    if(exploded.size() != rule::columns.size()){
      rule::report(std::string("Expected ") + rule::to_string(rule::columns.size()) + std::string(" columns on line ") + rule::to_string(line_number),rule::WARN);
    }
    rule::deny_if_disabled(exploded,out_file);
    line_number++;
    //std::cout << line << "\n";
  }

}
