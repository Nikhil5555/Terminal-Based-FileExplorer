#include<iostream>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string>
#include<vector>
#include<termios.h>
#include<ctype.h>
#include<dirent.h>
#include<sys/stat.h>
#include<algorithm>
#include<sys/ioctl.h>
#include<stack>
#include<pwd.h>
#include<grp.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<cmath>
#include<string.h>
#include<ctime>
#include<unistd.h>
using namespace std;
string cur_path;             // Global Current Path
bool quit=true;              //quit flag
bool flag=true;              //
bool normalFlag=true;        //Normal mode  flag
bool commandFlag=false;      //command mode flag by default system will start from normal mode.
stack<string> back;           //back stack for previous visited directories.
stack<string> forwardd;       //forward stack for coming back to the latest visited directories.

string startpath(){          //getcwd will give absolute address of the current working directory.
    char destination[1000];
    char* p=getcwd(destination,1000);
    if(p==NULL)
      perror("erro");
    string cwd= string(destination);
    return cwd;
}

void gotoxy(int x,int y){     //will clear and move the cursor to the given x,y cordinates.
	printf("%c[%d;%dH",27,x,y);
}

char* user(string filename)    //getting name of user(owner) of the file or folder
{
  struct stat information;
  stat(filename.data(), &information);  
  struct passwd *pw=getpwuid(information.st_uid);
  if(pw!=NULL)
  return pw->pw_name;
  else 
  return strerror(errno);
  char* c;
  return c;
}

char* grp(string filename)      //getting name of group of the file or folder
{
  struct stat information;
  stat(filename.data(),&information);
  struct group  *gr=getgrgid(information.st_gid);
  if(gr!=NULL)
  return gr->gr_name;
  else
  return strerror(errno);
  char* c;
  return c;
}

string modtime(string filename)  //getting modification time
{
struct stat result;
stat(filename.c_str(), &result);
string s= string(ctime(&result.st_mtime));
s.pop_back();
return s;
}

string filesize(std::string filename)  //getting filesize in B and coverting into KB
{
    struct stat stat_buffer;
    int rc=stat(filename.c_str(), &stat_buffer);
    long long size=stat_buffer.st_size;
    if(size<1024)
    return to_string(size)+"B";
    else
    return to_string(size/1024)+"KB";
}

string permissions(const char *file){        //getting permissions of a file or a folder
    struct stat st;
    string modeval;
    
    if(stat(file,&st) == 0)
    {
        mode_t p = st.st_mode;
        if(S_ISDIR(st.st_mode)==1)
        modeval.push_back('d');
        else
        modeval.push_back('-');
        if((p & S_IRUSR)!=0)
        modeval.push_back('r');
        else
        modeval.push_back('-');
        if((p & S_IWUSR)!=0)
        modeval.push_back('w');
        else
        modeval.push_back('-');
        if((p & S_IXUSR)!=0)
        modeval.push_back('x');
        else
        modeval.push_back('-');
        if((p & S_IRGRP)!=0)
        modeval.push_back('r');
        else
        modeval.push_back('-');
        if((p & S_IWGRP)!=0)
        modeval.push_back('w');
        else
        modeval.push_back('-');
        if((p & S_IXGRP)!=0)
        modeval.push_back('x');
        else
        modeval.push_back('-');
        if((p & S_IROTH)!=0)
        modeval.push_back('r');
        else
        modeval.push_back('-');
        if((p & S_IWOTH)!=0)
        modeval.push_back('w');
        else
        modeval.push_back('-');
        if((p & S_IXOTH)!=0)
        modeval.push_back('x');
        else
        modeval.push_back('-'); 
        return modeval;     
    }
    else
        return strerror(errno);
}

struct termios origin_termios;
void endrawmode()                              //For Disabling non-Canonical mode
{tcsetattr(STDIN_FILENO,TCSAFLUSH,&origin_termios);}

void enableRawMode() {                          //For Enabling Canonical mode
  tcgetattr(STDIN_FILENO,&origin_termios);
  atexit(endrawmode);
  struct termios raw = origin_termios;
  raw.c_lflag &= ~(ECHO|ICANON);
  tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw);
}

static bool cmp(vector<string> v1,vector<string> v2)   //Sorting by FileName
{
  return v1[5]<v2[5];
}

void Enterkey(int counter,vector<vector<string>> v);//declaration

void statusbarpos()                                 //Statusbar Position
{
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  int cols=ws.ws_col;
  int rows= ws.ws_row;
  gotoxy(rows-3,4);
}

void winsize()                                     //Commandmode Position
{
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  int cols=ws.ws_col;
  int rows= ws.ws_row;
  gotoxy(rows-1,4);
}

string gethomedir()                          //Getting Home Directory of the working user
{                                            //eg /home/nikhil
    char *homedir = getenv("HOME");
    uid_t uid = getuid();
    struct passwd *p = getpwuid(uid);
    return p->pw_dir;
}

void normalmode(string path,int count)       
{
    printf("\033c");                        //Screen Clear


    DIR* directory=opendir(path.c_str()); //opening directory
    if(directory == NULL)
   {
    
    cout<<strerror(errno);                 // Error Handling
    return;
   }

    struct dirent* entity;
    entity=readdir(directory);               //reading directory entry
    vector<vector<string>> v;
    int counter=count;
    for(;entity!=NULL;entity=readdir(directory))        //reading directory entries
   {
      vector<string> temp;
      string filePath = path + "/" + entity->d_name;
      temp.push_back(" ");
      temp.push_back(permissions(filePath.c_str()));
      temp.push_back((filesize(filePath.c_str())));
      temp.push_back(user(filePath.c_str()));
      temp.push_back(grp(filePath.c_str()));
      temp.push_back(entity->d_name);
      temp.push_back(modtime(filePath));

      v.push_back(temp); 
   }
   
    sort(v.begin(),v.end(),cmp);        //sorting
    v[counter][0]=">>>";
    for(int i=0;i<v.size();i++)         //printing
    {
        for(int j=0;j<v[i].size();j++)
        {
           if(j!=5)
              cout<<v[i][j]<<"\t";
           else 
              printf(" %-30s\t ",v[i][j].c_str());
        }
        cout<<endl;
    }
    statusbarpos();                    //getting Status bar position
    cout<<"Status Bar:"<<cur_path;
    char c;
    while(flag) 
    {
      c=getchar();
      if(c=='\n')                        //pressing Enterkey
     {
        if(counter==0)                   //Selecting "." file 
        {
          normalmode(cur_path,0);
        }
        else if(counter==1)                //Selecting ".." file 
        {
          if(cur_path!="/home")
            {
              back.push(cur_path);
              int i=cur_path.size()-1;
              while(cur_path[i]!='/')
             {
                 cur_path.pop_back();
                 i--;
             }
              cur_path.pop_back();
              normalmode(cur_path,0);
            }
        }
        else 
         Enterkey(counter,v);
    }
    
    else if(c==127)                       //pressing Backspace
    {
        string h="/home";
        if(cur_path!=h)
        {
           back.push(cur_path);
           while(!(forwardd.empty()))
           { 
            forwardd.pop();
            }
           int i=cur_path.size()-1;
           while(cur_path[i]!='/')
           {
            cur_path.pop_back();
            i--;
           }
        cur_path.pop_back();
        normalmode(cur_path,0);
        }
    }
    else if(c ==':')   //Switching to Command Mode
    {
      commandFlag=true;
      normalFlag=false;
      flag=false;
    }
   else if(c=='q')   //Quit and should be back to Canonical mode 
   {
    flag=false;
    quit=false;
    commandFlag=false;
    normalFlag=false;
   }
   else if(c=='h')     //Should get you to the home/user directory
   {
        string home=gethomedir();
        if(cur_path!=home)
        {
        back.push(cur_path);
        while(!forwardd.empty())
        {
           forwardd.pop();
        }
        cur_path=home;

        normalmode(cur_path,0);
        }
   }
    else{
     if (c==27)                //ASCII value of ESC
			{
				c=getchar();
        c=getchar();
				                         //If UP-arrow Key press
				if(c=='A')              //up key ASCII value is "ESC[A" which is of three characters.
				{   
          if(0==counter)
          {
            continue;
          }
          else 
          {
            v[counter][0]=" ";
            counter--;
            v[counter][0]=">>>";
            normalmode(path,counter);
          }
                    
        }
        else if(c=='B')  //DOWN key ASCII value is "ESC[B" which is of three characters.
        {
          if(v.size()-1==counter)
            {
              continue;          
            }
          else
            {
              v[counter][0]=" ";
              counter++;
              v[counter][0]=">>>";
              normalmode(path,counter);
            }
                  
        }
                else if(c=='C') //RIGHT key ASCII value is "ESC[C" which is of three characters.
                {
                   if(!forwardd.empty())
                   {
                      back.push(cur_path);
                      cur_path=forwardd.top();
                      forwardd.pop();
                      normalmode(cur_path,0);
                   }
                   
                }             
               else if(c=='D')   //LEFT key ASCII value is "ESC[D" which is of three characters.
               {   
                 if(!back.empty())
                {
                   forwardd.push(cur_path);
                   cur_path=back.top();
                   back.pop();
                   normalmode(cur_path,0);
                }
               }
               
            
            }
    }
  }
   
   closedir(directory);
   
}
void Enterkey(int counter,vector<vector<string>> v)  //Person presses Enter key
{
        struct stat s;
        
        string tempPath;
        tempPath=cur_path+'/'+v[counter][5];
        stat(tempPath.c_str(), &s);
        if(S_ISDIR(s.st_mode))
        {
            back.push(cur_path);
            cur_path=tempPath;
            normalmode(cur_path,0);
        }
        else{
            if(fork()==0) 
            {   string command;
                string file_name=cur_path+"/"+v[counter][5];
                string type=file_name.substr(file_name.length()-3);
                if((type=="jpg")||(type=="png")||(type=="pdf")||(type=="mp4")||(type=="mp3"))
                 command="xdg-open";
                else 
                 command="vi";
                char *p[3]={command.data(),file_name.data(),NULL};
				        if(execvp(command.data(),p)!=0)
        	         perror("exec");
            }
            else
				wait(0);
        }
}
/*---------------------------------------------------------------------------------------------*/
/*------------------------COMMAND MODE---------------------------------------------------------*/



int commandsearch(string file,string currentdir)   //Search File from a folder
{
    struct stat st;
    int x=chdir(currentdir.data());
    if(x==-1)
  {
		return 0;
	}
   DIR *temporary = opendir(currentdir.data());
   struct dirent* pointer = readdir(temporary);
   for(;pointer;pointer=readdir(temporary))
   {
      if((string(pointer->d_name)!=".") && (string(pointer->d_name)!=".."))
       {
          if(pointer->d_name==file)
				    return 1;
          stat((currentdir+'/'+pointer->d_name).data(),&st);
          if(S_ISDIR(st.st_mode))
            {
              string lol=currentdir+ "/" + pointer->d_name;
              if(commandsearch(file,lol))
					      return 1;
            } 
       }
   }
  return 0;
}
void clear()
{
  struct winsize ws;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  int cols=ws.ws_col;
  int rows= ws.ws_row;
  gotoxy(rows-1,4);
	
	printf("%c[2K", 27);             //words of the line in which the cursor is located will be cleaned but not overwritten
  printf("%c[%d;%dH",27,rows-1,1); //overwritting the letters
}
void commandcreatefile(string file_name,string path) //creating file
{
    int p=creat((path+'/'+file_name).data(),S_IRWXG|O_CREAT|S_IRWXO|S_IRWXU);
    if(p ==-1) 
      perror("Program");                 
}
void commandcreatedir(string pathdir,string dir_name) //creating dir
{
    dir_name=pathdir+'/'+dir_name;
    int status = mkdir((dir_name).data(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH|O_CREAT);
}
string resolve(string path)    //Resoving Path
{ 
char* p;
string h;
if(path[0]=='~')
{
   h=gethomedir();
   return h+path.substr(1);
}
else if (path[0]=='/')
{
    p=realpath(path.data(),NULL);
    if(p)
     return string(p);
    else
     winsize();
    cout<<cur_path<<">>>>>"<<"Invalid path";
     
    return "";
}
else 
{
    p=realpath((cur_path+"/"+path).data(),NULL);
    if(p)
     return string(p);
    else
     cout<<"Invalid path";
    return "";
}
}
void gotocommand(string path)   //goto directory  
{
  string p=resolve(path);
  if(p==""||p==cur_path)
  return;
  DIR* directory=opendir(p.c_str()); //opening directory
    if(directory==NULL)
    {
      return;
    }
    back.push(cur_path);
    cur_path=p;
    normalmode(p,0);
}
void copy_contents(string file_path,string file); //Declaration
void  copydirectory(string path,string folder_name,string org_path)  //copying directory(recursive)
  {
    string p=org_path+"/"+folder_name;
    commandcreatedir(path,folder_name);
    string cur=path+"/"+folder_name;
    //struct stat st;
    int x=chdir(p.c_str());
		//return 0;
    if(x==-1)
    {
       cout<<strerror(errno);
        return;
    }
     DIR *temporary = opendir(p.data());
     struct dirent* pointer = readdir(temporary);
     for(;pointer;pointer=readdir(temporary))
     {
        string l=p+'/'+pointer->d_name;
        if((string(pointer->d_name)!=".") && (string(pointer->d_name)!=".."))
        {
         struct stat st;
         stat(l.data(),&st);
          if(!(S_ISDIR(st.st_mode)))
          {
            commandcreatefile(pointer->d_name,cur);
            copy_contents(cur+'/'+pointer->d_name,p+'/'+pointer->d_name);
			    }
          else 
          {
            copydirectory(cur,pointer->d_name,p);
          }
         }
     }
     return;
  }

void renamecommand(string old_filename,string new_filename)//Renaming a file or folder
{
    int result=rename(old_filename.data(),new_filename.data());
    if(result==-1)
    cout<<strerror(errno);
}

void copy_contents(string file_path,string originalfile_path)  //copying contents of the file 
{
  char buffer;
	int p1,p2;
	p1=open(originalfile_path.data(),O_RDONLY);
  p2=open(file_path.data(),O_WRONLY|O_CREAT,S_IRWXU|S_IRGRP|S_IROTH);
	
	if(p1==-1)
	{
		printf("error opening original file\n");
		close(p1);
		return;
	}
	for(;read(p1,&buffer,1);)
		write(p2,&buffer,1);
	close(p2);
	close(p1);
}

void deletedir(string path)         //deleting empty directory
{
  int x=rmdir(path.data());
  if(x==-1)
  {
    perror("error");
  }
}

void deletefile(string path)      //deleting file
{
   int x= remove(path.data());
  if(x==-1)
    perror("error");
}

void deletedirectory(string path)   //deleting non-empty directory using recursion
{
  int x=chdir(path.c_str());
		//return 0;
    if(x==-1)
      return;

    DIR *temporary = opendir(path.data());
    struct dirent* pointer = readdir(temporary);
    for(;pointer;pointer=readdir(temporary))
     {
        string l=path+'/'+pointer->d_name;
         if((string(pointer->d_name)!=".") && (string(pointer->d_name)!=".."))
       {
           struct stat st;
           stat(l.data(),&st);
          if(!(S_ISDIR(st.st_mode))){
              deletefile(l);           //delete
          }
          else{
            deletedirectory(l);
              deletedir(l);        //delete
          }
       }
       else {
        continue;
       }

     }
     return;
}
void startcommandmode() //starting command mode
{  char ch;
     string input,str;
     vector<string> str_tokenss;
     while(1)
     {
        
        winsize();      //  cursor will move to gotoxy(rows-1,4)
        cout<<cur_path<<">>>>>";
		for(;(ch = getchar())!=27 && ch!=10;)
		{
			if(ch==127)        //Backspace
			{   clear();
          if(input.length()<=1)
				  {
					  input="";
				  }
				  else{
					  input.pop_back();
				  }
          winsize();
          cout<<cur_path<<">>>>>";
				  printf("%s",input.data());  //Important
				
			}
			else{
				   input.push_back(ch);
				   printf("%c",ch);
			}
            
        }
        if(ch==27)   //colon //Return to Normal mode
        {
            normalFlag=true;
            flag=true;
            commandFlag=false;
            
            return;
        }
        else if(ch==10)   //Enter
        {
            clear();
        for(int i=0;i<input.size();i++)
        {
            
            if(input[i]==' ')
            {
                str_tokenss.push_back(str);
                str.clear();
            }
            else {
                str+=input[i];
            }
        }
        input.clear();
        if(str!=" ")
       {
       str_tokenss.push_back(str);
       str.clear();
       }
     if(str_tokenss[0]=="search"||str_tokenss[0]=="Search"||str_tokenss[0]=="SEARCH") //searching recursivly
     {
      if(str_tokenss.size()==2)
       {
        winsize();
        if(commandsearch(str_tokenss[1],cur_path))
        cout<<cur_path<<">>>>>"<<"True";
        else
        cout<<cur_path<<">>>>>"<<"False";
       }
      
     }
     else if(str_tokenss[0]=="create_file")   
     {
        if(str_tokenss.size()==3)
        {
            string p=resolve(str_tokenss[2]);
            commandcreatefile(str_tokenss[1],p);
        }
        
     }
        else if(str_tokenss[0]=="create_dir")  
        {
              if(str_tokenss.size()==3)
              {
                string p=resolve(str_tokenss[2]);
                commandcreatedir(p,str_tokenss[1]);
              }
              
              
        }
       else  if(str_tokenss[0]=="goto")
        {
          if(str_tokenss.size()==2)
            gotocommand(str_tokenss[1]);
          
        }
      else if(str_tokenss[0]=="rename")
      {
         if(str_tokenss.size()==3)
        renamecommand(cur_path+'/'+str_tokenss[1],cur_path+'/'+str_tokenss[2]);
         
      }      
     else if(str_tokenss[0]=="copy")
      {
        int i=1;
        int n=str_tokenss.size();
        string p=resolve(str_tokenss[n-1]);
        str_tokenss.pop_back();
        while(i<=n-2)
        {
          struct stat st;
          stat((cur_path+'/'+str_tokenss[i]).data(),&st);
          if(!(S_ISDIR(st.st_mode)))
         { 
           
                commandcreatefile(str_tokenss[i],p);
                string original=cur_path+'/'+str_tokenss[i];
                copy_contents(p+"/"+str_tokenss[i],original);
         }
         else{
                copydirectory(p,str_tokenss[i],cur_path); 
         }
         i++;
          }
      }
      else if(str_tokenss[0]=="delete_file")
      {     if(str_tokenss.size()==2)
            {
            string path=resolve(str_tokenss[1]);
            deletefile(path);
            }
         
      }
      else if(str_tokenss[0]=="delete_dir")
      {
        if(str_tokenss.size()==2)
        {
            string g=resolve(str_tokenss[1]);
            deletedirectory(g);
            deletedir(g);
        }
      }
      else if(str_tokenss[0]=="move")
      {
        int i=1;
        int n=str_tokenss.size();
        string p=resolve(str_tokenss[n-1]);
        str_tokenss.pop_back();
         while(i<=n-2)
        {
          struct stat st;
          stat((cur_path+'/'+str_tokenss[i]).data(),&st);
          if(!(S_ISDIR(st.st_mode)))
         {
            commandcreatefile(str_tokenss[i],p);
            string original=cur_path+'/'+str_tokenss[i];
            copy_contents(p+"/"+str_tokenss[i],original);
            deletefile(original);
            
         }
         else
         {
            copydirectory(p,str_tokenss[i],cur_path);
            string original=cur_path+'/'+str_tokenss[i];
            deletedirectory(original);
            deletedir(original);     
         }
          i++;
        }
         
      }
      else if(str_tokenss[0]=="quit")
      {
         normalFlag=false;
         flag=false;
         commandFlag=false;
         quit=false;
         return;
      }
      else{
         winsize();
        cout<<cur_path<<">>>>>"<<"Invalid Command";
      }
     }
     str_tokenss.clear();
     }
        }
int main()
{
   system("clear");  
   cur_path=startpath(); //getting current path in global
   enableRawMode();      //Starting non-canonical mode
   while(quit)
   {
        if(normalFlag){
            
            normalmode(cur_path.data(),0);
        }
        if(commandFlag)
        {
            
            cout<<endl<<endl;
            startcommandmode();
        }
   }
 endrawmode(); //ending non-canonical mode
   return 0;
}