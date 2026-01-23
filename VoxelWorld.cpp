#include<bits/stdc++.h>
#include<dirent.h>
#include<ctime>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<windows.h>
#include<cwchar>
#include<conio.h>
#define int long long
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME)&0x8000)?1:0)
#define a first
#define b second
using namespace std;
vector<double>recent_FPS; 
const double EPSILON=1e-7;
double now_FPS=0;
POINT p;
HANDLE hOutput=GetStdHandle(STD_OUTPUT_HANDLE);
HWND h=GetForegroundWindow();
CONSOLE_FONT_INFO consoleCurrentFont;

vector<int> prev_screen_color;
map<int,vector<pair<int,int> > > color_groups;
bool first_frame=true;
vector<int> current_frame;
bool force_redraw=false;

int getTimeOfDayMicros() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    
    int microseconds=tv.tv_sec*1000000LL+tv.tv_usec;
    return microseconds;
}

vector<string> getSaveFileNames() {
    vector<string> fileNames;
    const string folderPath="./saves";
    
    DIR* dir=opendir(folderPath.c_str());
    if(dir==NULL) {
        return fileNames;
    }

    dirent* entry;
    while((entry=readdir(dir))!=NULL) {
        if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0) {
            continue;
        }
        
        string fullPath=folderPath+"/"+entry->d_name;
        struct stat fileStat;
        if(stat(fullPath.c_str(),&fileStat)) {
            continue;
        }
        
        if(S_ISREG(fileStat.st_mode)) {
            fileNames.push_back(entry->d_name);
        }
    }
    closedir(dir);
    return fileNames;
}

string inttostring(int x) {
    string res="";
    while(x) {
        res=(char)(x%10+(int)'0')+res;
        x/=10;
    }
    return res;
}

struct node {
    double x,y,z;
    bool operator<(const node &a) const {
        if(x<a.x) return 1;
        if(x>a.x) return 0;
        if(y<a.y) return 1;
        if(y>a.y) return 0;
        if(z<a.z) return 1;
        return 0;
    }
    bool operator==(const node &a) const {
        return (x==a.x)&&(y==a.y)&&(z==a.z);
    }
    bool operator!=(const node &a) const {
        return !((x==a.x)&&(y==a.y)&&(z==a.z));
    }
    node operator+(const node &a) const {
        node res={x+a.x,y+a.y,z+a.z};
        return res;
    }
    node operator-(const node &a) const {
        node res={x-a.x,y-a.y,z-a.z};
        return res;
    }
    node operator*(const double &a) const {
        node res={x*a,y*a,z*a};
        return res;
    }
};

map<pair<string,int>,vector<vector<int> > > all_cube_color;
vector<int> screen_color;
double r=1,screen_x_wid=0.4,screen_y_wid=0.225;
node position={-1,0,0},eye_dir,eye_act,screen_x_dir,screen_y_dir,eye_xydir;
int screen_x_pixel=192,screen_y_pixel=54;
int updown_degree=2,leftright_degree=2;
double forbackward_step=0.2,leftright_step=0.2;
int render_distance=24;
double a,b;
string s;
bool change;
map<node,string> cube;
int now_cube=1;
string idcube[15]={"","grass","stone","test"};
double tens=100000000;
bool is_color=0;
int resolution=3;
bool is_round=0;
int resolution_x[10]={0,64,128,192,384,512,1024};
int resolution_y[10]={0,18,36,54,108,144,288};
double r_opt[10]={0,0.1,0.2,0.5,1,2,4,8,16};
int r_state=4;
node round_self(node n) {
    node res={floor(tens*n.x+0.5)/tens,floor(tens*n.y+0.5)/tens,floor(tens*n.z+0.5)/tens};
    return res;
}

double sinn(double x) {
    return floor(tens*sin(x/360*(asin(1)*4))+0.5)/tens;
}

double coss(double x) {
    return floor(tens*cos(x/360*(asin(1)*4))+0.5)/tens;
}

void over_all_init() {
    for(int i=1;i<=9;i++) {
        for(int j=1;j<=6;j++) {
            if(idcube[i]!="") {
                FILE* fp;
                string adr="./texture/"+idcube[i];
                char cha=j+'0';
                adr+=cha;
                adr+=".txt";
                fp=fopen(adr.c_str(),"r");
                
                vector<vector<int>> all_color;
                for(int ii=0;ii<16;ii++) {
                    vector<int> row_color;
                    for(int jj=0;jj<16;jj++) {
                        int tem;
                        fscanf(fp,"%lld",&tem);
                        row_color.push_back(tem);
                    }
                    all_color.push_back(row_color);
                }
                all_cube_color[(pair<string,int>){idcube[i],j}]=all_color;
                fclose(fp);
            }
        }
    }
}

void init() {
    s="";
    screen_x_wid=r*0.4;
    screen_y_wid=r*0.225;
    eye_xydir=(node){coss(a),sinn(a),0};
    eye_dir=(node){coss(b)*coss(a),coss(b)*sinn(a),sinn(b)};
    eye_act=eye_dir*r;
    screen_x_dir=(node){sinn(a),-coss(a),0};
    screen_y_dir=(node){-sinn(b)*coss(a),-sinn(b)*sinn(a),coss(b)};
    eye_xydir=round_self(eye_xydir);
    eye_dir=round_self(eye_dir);
    eye_act=round_self(eye_act);
    screen_x_dir=round_self(screen_x_dir);
    screen_y_dir=round_self(screen_y_dir);
}

node eye_next(node cur_position,node direction) {
    node res={-1,-1,-1};
    if(direction.x!=0) {
        double boundary=(direction.x>0)?floor(cur_position.x+1.0+EPSILON):ceil(cur_position.x-1.0-EPSILON);
        res.x=(boundary-cur_position.x)/direction.x;
    }
    if(direction.y!=0) {
        double boundary=(direction.y>0)?floor(cur_position.y+1.0+EPSILON):ceil(cur_position.y-1.0-EPSILON);
        res.y=(boundary-cur_position.y)/direction.y;
    }
    if(direction.z!=0) {
        double boundary=(direction.z>0)?floor(cur_position.z+1.0+EPSILON):ceil(cur_position.z-1.0-EPSILON);
        res.z=(boundary-cur_position.z)/direction.z;
    }
    return res;
}

node find_color(string cube_id,node cube_relative,int inlight_direction) {
    vector<vector<int>> all_color=all_cube_color[(pair<string,int>){cube_id,inlight_direction}];
    int pixel_color;
    if((inlight_direction-1)/2==0) {
        pixel_color=all_color[(int)min(floor(cube_relative.y*16),15.0)][(int)min(floor(cube_relative.z*16),15.0)];
    }
    else if((inlight_direction-1)/2==1) {
        pixel_color=all_color[(int)min(floor(cube_relative.x*16),15.0)][(int)min(floor(cube_relative.z*16),15.0)];
    }
    else if((inlight_direction-1)/2==2) {
        pixel_color=all_color[(int)min(floor(cube_relative.x*16),15.0)][(int)min(floor(cube_relative.y*16),15.0)];
    }
    node color;
    color.x=(double)(pixel_color>>16);
    color.y=(double)((pixel_color>>8)&((1<<8)-1));
    color.z=(double)(pixel_color&((1<<8)-1));
    return color;
}

void record_s(string cube_id,int inlight_direction) {
    if(cube_id!="") {
        if(inlight_direction==1) s+='B';
        else if(inlight_direction==2) s+='F';
        else if(inlight_direction==3) s+='R';
        else if(inlight_direction==4) s+='L';
        else if(inlight_direction==5) s+='D';
        else if(inlight_direction==6) s+='U';
    }
    else if(inlight_direction) s+='_';
    else s+='|';
}

void print(node color_rgb) {
    signed xxx=floor(color_rgb.x),yyy=floor(color_rgb.y),zzz=floor(color_rgb.z);
    int color_val=(xxx<<16)+(yyy<<8)+zzz;
    screen_color.push_back(color_val);
    current_frame.push_back(color_val);
}

void gogogo(node direction) {
    node cur_position=direction+position;
    cur_position=cur_position+direction*EPSILON;
    
    for(int i=1;i<=render_distance;i++) {
        node eye_next__=eye_next(cur_position,direction);
        
        int xyz=0;
        if(eye_next__.x==-1&&eye_next__.y==-1&&eye_next__.z==-1) {
            continue;
        }
        else if(eye_next__.y==-1&&eye_next__.z==-1) {
            xyz=1;
        }
        else if(eye_next__.x==-1&&eye_next__.z==-1) {
            xyz=2;
        }
        else if(eye_next__.x==-1&&eye_next__.y==-1) {
            xyz=3;
        }
        else if(eye_next__.x==-1) {
            if(eye_next__.y<=eye_next__.z) {
                xyz=2;
            }
            else xyz=3;
        }
        else if(eye_next__.y==-1) {
            if(eye_next__.x<=eye_next__.z) {
                xyz=1;
            }
            else xyz=3;
        }
        else if(eye_next__.z==-1) {
            if(eye_next__.x<=eye_next__.y) {
                xyz=1;
            }
            else xyz=2;
        }
        else {
            if(eye_next__.x<=eye_next__.y&&eye_next__.x<=eye_next__.z) {
                xyz=1;
            }
            else if(eye_next__.y<=eye_next__.x&&eye_next__.y<=eye_next__.z) {
                xyz=2;
            }
            else xyz=3;
        }
        
        if(xyz==1) {
            int inlight_direction=xyz*2;
            if(direction.x>=0) inlight_direction--;
            cur_position=cur_position+(direction*eye_next__.x);
            
            node cube_num;
            cube_num.x=(direction.x>=0)?floor(cur_position.x+EPSILON):floor(cur_position.x-EPSILON);
            cube_num.y=floor(cur_position.y);
            cube_num.z=floor(cur_position.z);
            
            if(cube[cube_num]=="") continue;
            node cube_relative=cur_position-cube_num;
            
            record_s(cube[cube_num],inlight_direction); 
            print(find_color(cube[cube_num],cube_relative,inlight_direction));
            return;
        }
        else if(xyz==2) {
            int inlight_direction=xyz*2;
            if(direction.y>=0) inlight_direction--;
            cur_position=cur_position+(direction*eye_next__.y);
            
            node cube_num;
            cube_num.y=(direction.y>=0)?floor(cur_position.y+EPSILON):floor(cur_position.y-EPSILON);
            cube_num.x=floor(cur_position.x);
            cube_num.z=floor(cur_position.z);
            
            if(cube[cube_num]=="") continue;
            node cube_relative=cur_position-cube_num;
            
            record_s(cube[cube_num],inlight_direction); 
            print(find_color(cube[cube_num],cube_relative,inlight_direction));
            return;
        }
        else if(xyz==3) {
            int inlight_direction=xyz*2;
            if(direction.z>=0) inlight_direction--;
            cur_position=cur_position+(direction*eye_next__.z);
            
            node cube_num;
            cube_num.z=(direction.z>=0)?floor(cur_position.z+EPSILON):floor(cur_position.z-EPSILON);
            cube_num.x=floor(cur_position.x);
            cube_num.y=floor(cur_position.y);
            
            if(cube[cube_num]=="") continue;
            node cube_relative=cur_position-cube_num;
            
            record_s(cube[cube_num],inlight_direction); 
            print(find_color(cube[cube_num],cube_relative,inlight_direction));
            return;
        }
    }
    record_s("",(int)(direction.z>=0));
    print((node){0,0,0});
}

pair<int,node> eyegogogo(node direction) {
    node cur_position=direction+position;
    cur_position=cur_position+direction*EPSILON;
    
    for(int i=1;i<=render_distance;i++) {
        node eye_next__=eye_next(cur_position,direction);
        
        int xyz=0;
        if(eye_next__.x==-1&&eye_next__.y==-1&&eye_next__.z==-1) {
            continue;
        }
        else if(eye_next__.y==-1&&eye_next__.z==-1) {
            xyz=1;
        }
        else if(eye_next__.x==-1&&eye_next__.z==-1) {
            xyz=2;
        }
        else if(eye_next__.x==-1&&eye_next__.y==-1) {
            xyz=3;
        }
        else if(eye_next__.x==-1) {
            if(eye_next__.y<=eye_next__.z) {
                xyz=2;
            }
            else xyz=3;
        }
        else if(eye_next__.y==-1) {
            if(eye_next__.x<=eye_next__.z) {
                xyz=1;
            }
            else xyz=3;
        }
        else if(eye_next__.z==-1) {
            if(eye_next__.x<=eye_next__.y) {
                xyz=1;
            }
            else xyz=2;
        }
        else {
            if(eye_next__.x<=eye_next__.y&&eye_next__.x<=eye_next__.z) {
                xyz=1;
            }
            else if(eye_next__.y<=eye_next__.x&&eye_next__.y<=eye_next__.z) {
                xyz=2;
            }
            else xyz=3;
        }
        
        if(xyz==1) {
            int inlight_direction=xyz*2;
            if(direction.x>=0) inlight_direction--;
            cur_position=cur_position+(direction*eye_next__.x);
            
            node cube_num;
            cube_num.x=(direction.x>=0)?floor(cur_position.x+EPSILON):floor(cur_position.x-EPSILON);
            cube_num.y=floor(cur_position.y);
            cube_num.z=floor(cur_position.z);
            
            if(cube[cube_num]=="") continue;
            return (pair<int,node>){inlight_direction,cube_num};
        }
        else if(xyz==2) {
            int inlight_direction=xyz*2;
            if(direction.y>=0) inlight_direction--;
            cur_position=cur_position+(direction*eye_next__.y);
            
            node cube_num;
            cube_num.y=(direction.y>=0)?floor(cur_position.y+EPSILON):floor(cur_position.y-EPSILON);
            cube_num.x=floor(cur_position.x);
            cube_num.z=floor(cur_position.z);
            
            if(cube[cube_num]=="") continue;
            return (pair<int,node>){inlight_direction,cube_num};
        }
        else if(xyz==3) {
            int inlight_direction=xyz*2;
            if(direction.z>=0) inlight_direction--;
            cur_position=cur_position+(direction*eye_next__.z);
            
            node cube_num;
            cube_num.z=(direction.z>=0)?floor(cur_position.z+EPSILON):floor(cur_position.z-EPSILON);
            cube_num.x=floor(cur_position.x);
            cube_num.y=floor(cur_position.y);
            
            if(cube[cube_num]=="") continue;
            return (pair<int,node>){inlight_direction,cube_num};
        }
    }
    return (pair<int,node>){0,(node){0,0,0}};
}

void add_cube(pair<int,node> pid) {
    int inlight_direction=pid.a;
    node cube_num=pid.b;
    if(inlight_direction==1) {
        cube[(node){cube_num.x-1,cube_num.y,cube_num.z}]=idcube[now_cube];
    }
    else if(inlight_direction==2) {
        cube[(node){cube_num.x+1,cube_num.y,cube_num.z}]=idcube[now_cube];
    }
    else if(inlight_direction==3) {
        cube[(node){cube_num.x,cube_num.y-1,cube_num.z}]=idcube[now_cube];
    }
    else if(inlight_direction==4) {
        cube[(node){cube_num.x,cube_num.y+1,cube_num.z}]=idcube[now_cube];
    }
    else if(inlight_direction==5) {
        cube[(node){cube_num.x,cube_num.y,cube_num.z-1}]=idcube[now_cube];
    }
    else if(inlight_direction==6) {
        cube[(node){cube_num.x,cube_num.y,cube_num.z+1}]=idcube[now_cube];
    }
}

void delete_cube(pair<int,node> pid) {
    int inlight_direction=pid.a;
    node cube_num=pid.b;
    if(inlight_direction!=0) cube[(node){cube_num.x,cube_num.y,cube_num.z}]="";
}

void gotoxy(int x,int y) {
    COORD pos={(short)x,(short)y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);
}

void screen_close() {
}

bool EnableVirtualTerminal() {
    HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
    if(hOut==INVALID_HANDLE_VALUE) return false;
    
    DWORD dwMode=0;
    if(!GetConsoleMode(hOut,&dwMode)) return false;
    
    dwMode|=0x0004;
    if(!SetConsoleMode(hOut,dwMode)) return false;
    
    return true;
}

void PrintColoredSequence(int start_x,int y,int length,int color_val) {
    int rr=(color_val>>16)&255;
    int gg=(color_val>>8)&255;
    int bb=color_val&255;
    
    gotoxy(start_x,y);
    printf("\x1b[48;2;%d;%d;%dm",(signed)rr,(signed)gg,(signed)bb);
    for(int i=0;i<length;i++) {
        printf(" ");
    }
    printf("\x1b[0m");
}

void print_position_info() {
    gotoxy(0,0);
    if(is_color) {
        printf("\x1b[48;2;0;0;0m\x1b[38;2;255;255;255m");  
    }
    cout<<"Pos: "<<position.x+eye_act.x<<"  "<<position.y+eye_act.y<<"  "<<position.z+eye_act.z<<"  |"
        <<"Angle: "<<a<<" "<<b<<"  |"
        <<"r: "<<r<<"  |"
        <<"Now cube: "<<idcube[now_cube]<<"  |"
        <<"Screen size: "<<screen_x_pixel<<"*"<<screen_y_pixel<<"  |"
        <<"FPS: ";
    for(int i=max(0ll,(int)(recent_FPS.size()-15));i<recent_FPS.size();i++)printf("%.5lf ",recent_FPS[i]);
    if(is_color) {
        printf("\x1b[0m");
    }
}

void optimized_render() {
    if(!is_color) {
        if(first_frame||force_redraw) {
            system("cls");
            print_position_info();
            gotoxy(0,1); 
            cout<<s;
            first_frame=false;
            force_redraw=false;
            prev_screen_color=current_frame;
        }
        return;
    }

    if(first_frame||force_redraw) {
        system("cls");
        for(int i=0;i<=screen_y_pixel;i++) {
            for(int j=0;j<=screen_x_pixel;j++) {
                int idx=i*(screen_x_pixel+1)+j;
                int color_val=current_frame[idx];
                int rr=(color_val>>16)&255;
                int gg=(color_val>>8)&255;
                int bb=color_val&255;
                gotoxy(j,i);
                printf("\x1b[48;2;%d;%d;%dm \x1b[0m",(signed)rr,(signed)gg,(signed)bb);
            }
        }
        first_frame=false;
        force_redraw=false;
        prev_screen_color=current_frame;
        
        print_position_info();
        return;
    }

    bool need_update=false;
    
    for(int y=0;y<=screen_y_pixel;y++) {
        int start_x=-1;
        int current_color=-1;
        int run_length=0;
        
        for(int x=0;x<=screen_x_pixel;x++) {
            int idx=y*(screen_x_pixel+1)+x;
            int prev_color=prev_screen_color[idx];
            int new_color=current_frame[idx];
            
            if(prev_color!=new_color) {
                need_update=true;
                
                if(current_color!=-1&&new_color!=current_color) {
                    PrintColoredSequence(start_x,y,run_length,current_color);
                    start_x=x;
                    current_color=new_color;
                    run_length=1;
                } 
                else if(current_color==-1) {
                    start_x=x;
                    current_color=new_color;
                    run_length=1;
                }
                else {
                    run_length++;
                }
            } 
            else if(current_color!=-1) {
                PrintColoredSequence(start_x,y,run_length,current_color);
                current_color=-1;
                run_length=0;
            }
        }
        
        if(current_color!=-1) {
            PrintColoredSequence(start_x,y,run_length,current_color);
        }
    }
    
    if(need_update) {
        prev_screen_color=current_frame;
    }
    
    print_position_info();
}

void add_center_cross() {
    int center_x=screen_x_pixel/2;
    int center_y=screen_y_pixel/2;
    
    for(int x=center_x-2;x<=center_x+2;x++) {
        if(x>=0&&x<=screen_x_pixel) {
            int idx=center_y*(screen_x_pixel+1)+x;
            if(idx>=0&&idx<screen_color.size()) {
                screen_color[idx]=0xFFFFFF;
            }
        }
    }
    
    for(int y=center_y-1;y<=center_y+1;y++) {
        if(y>=0&&y<=screen_y_pixel) {
            int idx=y*(screen_x_pixel+1)+center_x;
            if(idx>=0&&idx<screen_color.size()) {
                screen_color[idx]=0xFFFFFF;
            }
        }
    }
}
int get_nowtime(){
    time_t now=time(0);
    tm* ltm=localtime(&now);
    int nowtime=(int)(1900+ltm->tm_year)*10000000000ll+
                  (int)(1+ltm->tm_mon)*100000000ll+
                  (int)(ltm->tm_mday)*1000000ll+
                  (int)(ltm->tm_hour)*10000ll+
                  (int)(ltm->tm_min)*100+
                  (ltm->tm_sec);
    return nowtime;
}
void touch_c(){
    int nowtime=get_nowtime();
    FILE* fp;
    string adr="images/"+inttostring(nowtime)+".txt";
    fp=fopen(adr.c_str(),"w");
    fprintf(fp,"%s",s.c_str());
    fclose(fp);
}
void touch_k(){
    int nowtime=get_nowtime();
    string adr="saves/"+inttostring(nowtime)+".txt";
    FILE *fp=fopen(adr.c_str(),"w");
    map<node,string>::iterator it;
    int cubesize=0;
    
    it=cube.begin();
    while(it!=cube.end()) {
        if((*it).b!="") cubesize++;
        it++;
    }
    
    it=cube.begin();
    fprintf(fp,"%d\n",cubesize);
    while(it!=cube.end()) {
        if((*it).b!="") {
            fprintf(fp,"%lf %lf %lf   ",(*it).a.x,(*it).a.y,(*it).a.z);
            fprintf(fp,"%s\n",(*it).b.c_str());
        }
        it++;
    }
    fclose(fp);
                    
}
signed main() {
    cout<<"init...";
    over_all_init();
    system("cls");
    cout<<"init ok!";
    Sleep(50);
    EnableVirtualTerminal();
    
    int screen_size=(screen_x_pixel+1)*(screen_y_pixel+1);
    prev_screen_color.resize(screen_size,-1);
    current_frame.reserve(screen_size);
    
    string path_saves="./saves";
    string path_images="./images";
    CreateDirectory(path_saves.c_str(),NULL);
    CreateDirectory(path_images.c_str(),NULL);
    
    cube.insert((pair<node,string>){((node){0,0,0}),"grass"});
    cube.insert((pair<node,string>){((node){2,0,0}),"grass"});
    cube.insert((pair<node,string>){((node){0,2,0}),"grass"});
    cube.insert((pair<node,string>){((node){2,2,0}),"grass"});
    cube.insert((pair<node,string>){((node){0,0,2}),"grass"});
    cube.insert((pair<node,string>){((node){2,0,2}),"grass"});
    cube.insert((pair<node,string>){((node){0,2,2}),"grass"});
    cube.insert((pair<node,string>){((node){2,2,2}),"grass"});
    
    while(1) {
        int start_time=getTimeOfDayMicros();
        init();
        
        screen_color.clear();
        current_frame.clear();
        s="";
        
        for(int j=screen_y_pixel;j>=0;j--) {
            for(int i=0;i<=screen_x_pixel;i++) {
                node eye_to_pixel;
                if(is_round)eye_to_pixel=eye_act*(sinn(i*1.0/screen_x_pixel*360+90)*sinn(j*1.0/screen_y_pixel*180+180))+
                                    screen_x_dir*(sinn(i*1.0/screen_x_pixel*360)*sinn(j*1.0/screen_y_pixel*180+180))+
                                    screen_y_dir*coss(j*1.0/screen_y_pixel*180+180);
                else{
                    eye_to_pixel=eye_act+
                                  (screen_x_dir*screen_x_wid*(i*1.0/screen_x_pixel*2-1))+
                                  (screen_y_dir*screen_y_wid*(j*1.0/screen_y_pixel*2-1));
                }
                eye_to_pixel=round_self(eye_to_pixel);
                gogogo(eye_to_pixel);
                
                if(!is_color&&j==screen_y_pixel/2&&i==screen_x_pixel/2) {
                    if(s.length()>0) {
                        int pos=(screen_y_pixel-j)*(screen_x_pixel+2)+i;
                        if(pos<s.length()) {
                            s[pos]='+';
                        }
                    }
                } 
            }
            s+='\n';
        }
        
        if(is_color) {
            add_center_cross();
            current_frame=screen_color;
        }
        
        if(!is_color) {
            force_redraw=true;
        }
        
        optimized_render();
        int end_time=getTimeOfDayMicros();
        
        now_FPS=1000000.0/(end_time-start_time);
        recent_FPS.push_back(now_FPS);
        while(1) {
            if(_kbhit()) {
                int ch=_getch();
                
                if(ch==45){
                    if(resolution>1&&!is_color){
                        resolution--;
                        screen_x_pixel=resolution_x[resolution];
                        screen_y_pixel=resolution_y[resolution];
                    }
                    break;  
                }
                else if(ch==61){
                    if(resolution<6&&!is_color){
                        resolution++;
                        screen_x_pixel=resolution_x[resolution];
                        screen_y_pixel=resolution_y[resolution];
                    }
                    break;
                }
                else if(ch==91){
                    if(r_state>1){
                        r_state--;
                        r=r_opt[r_state];
                    }
                    break;
                }
                else if(ch==93){
                    if(r_state<8){
                        r_state++;
                        r=r_opt[r_state];
                    }
                    break;
                }
                else if(ch==119) {
                    position=position+(eye_dir*forbackward_step);    
                    break;
                }
                else if(ch==115) {
                    position=position-(eye_dir*forbackward_step);    
                    break;
                }
                else if(ch==97) {
                    position=position-(screen_x_dir*leftright_step);
                    break;
                }
                else if(ch==100) {
                    position=position+(screen_x_dir*leftright_step);
                    break;
                }
                else if(ch==112) {
                    position.z+=forbackward_step;
                    break;
                }
                else if(ch==108) {
                    position.z-=forbackward_step;
                    break;
                }
                else if(ch==113) {
                    a+=leftright_degree;
                    if(a>360) a-=360;
                    break;
                }
                else if(ch==101) {
                    a-=leftright_degree;
                    if(a<0) a+=360;
                    break;
                }
                else if(ch==114) {
                    if(b+updown_degree<=90) {
                        b+=updown_degree;
                    }
                    break;
                }
                else if(ch==102) {
                    if(b-updown_degree>=-90) {
                        b-=updown_degree;
                    }
                    break;
                }
                else if(ch==116) {
                    position=position+(eye_xydir*forbackward_step);
                    break;
                }
                else if(ch==103) {
                    position=position-(eye_xydir*forbackward_step);
                    break;
                }
                else if(ch==98) {
                    add_cube(eyegogogo(eye_dir));
                    change=1;
                    break;
                }
                else if(ch==118) {
                    delete_cube(eyegogogo(eye_dir));
                    change=1;
                    break;
                }
                else if(ch>=(int)'1'&&ch<=(int)'9') {
                    now_cube=ch-(int)'0';
                    break;
                }
                else if(ch==104) {
                    is_color=!is_color;
                    force_redraw=true;
                    break;
                }
                else if(ch==121) {
                    is_round=!is_round;
                    break;
                }
                else if(ch==99) {
                    touch_c();
                    break;
                }
                else if(ch==107) {
                    touch_c();
                    touch_k();
                    change=0;
                    break;
                }
                else if(ch==106) {
                    system("cls");
                    vector<string> temp_file_names=getSaveFileNames();
                    int pos=0;
                    for(int i=0;i<temp_file_names.size();i++) {
                        cout<<temp_file_names[i]<<' ';
                        if(i==pos) cout<<"<<";
                        cout<<endl;
                    }
                    cout<<"\nexit ";
                    if(pos==temp_file_names.size()) cout<<"<<";
                    if(pos!=temp_file_names.size()) {
                        FILE* fp;
                        string adr="images/"+temp_file_names[pos];
                        fp=fopen(adr.c_str(),"r");
                        if(fp!=NULL) {
                            char* line=new char[2001];
                            int line_number=1;
                            while(true) {
                                if(fgets(line,2000,fp)==NULL) {
                                    if(feof(fp)) break;   
                                    if(ferror(fp)) {
                                        perror("¶ÁÈ¡´íÎó");
                                        break;
                                    }
                                }   
                                printf("%s",line);
                                line_number++;
                            }
                            delete[] line;
                            fclose(fp);
                        }
                    }
                    while(1) {
                        if(_kbhit()) {
                            int ch=_getch();
                            if(ch==119||ch==115) {
                                if(ch==119) {
                                    if(pos>0) pos--;
                                }
                                else if(ch==115) {
                                    if(pos<temp_file_names.size()) pos++;
                                }
                                system("cls");
                                for(int i=0;i<temp_file_names.size();i++) {
                                    cout<<temp_file_names[i]<<' ';
                                    if(i==pos) cout<<"<<";
                                    cout<<endl;
                                }
                                cout<<"\nexit ";
                                if(pos==temp_file_names.size()) cout<<"<<";
                                cout<<endl<<endl;
                                if(pos!=temp_file_names.size()) {
                                    FILE* fp;
                                    string adr="images/"+temp_file_names[pos];
                                    fp=fopen(adr.c_str(),"r");
                                    if(fp!=NULL) {
                                        char* line=new char[2001];
                                        int line_number=1;
                                        string s_out=""; 
                                        while(true) {
                                            if(fgets(line,2000,fp)==NULL) {
                                                if(feof(fp)) break;   
                                                if(ferror(fp)) {
                                                    perror("¶ÁÈ¡´íÎó");
                                                    break;
                                                }
                                            }   
                                            s_out+=line;
                                            line_number++;
                                        }
                                        delete[] line;
                                        cout << s_out;
                                        fclose(fp);
                                    }
                                }
                            }
                            else if(ch==13) {
                                if(change) {
                                    change=0;
                                    touch_c();
                                    touch_k();
                                }
                                if(pos==temp_file_names.size()) break;
                                FILE* fp;
                                string adr="saves/"+temp_file_names[pos];
                                fp=fopen(adr.c_str(),"r");
                                int cubesize=0;
                                fscanf(fp,"%lld",&cubesize);
                                if(cubesize==0) break;
                                cube.clear();
                                for(int i=1;i<=cubesize;i++) {
                                    node cube_num=(node){0,0,0};
                                    string cubestyle;
                                    fscanf(fp,"%lf %lf %lf",&cube_num.x,&cube_num.y,&cube_num.z);
                                    cubestyle.resize(128); 
                                    fscanf(fp,"%s",&cubestyle[0]);
                                    cubestyle.resize(strlen(cubestyle.c_str()));
                                    cube[cube_num]=cubestyle;
                                }
                                break;
                            }
                            else continue;
                        }
                    }
                    force_redraw=true;
                    break;
                }
            }
        }
    }
    return 0;
}
