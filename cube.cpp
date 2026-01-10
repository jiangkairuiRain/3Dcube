#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
#include<sys/time.h>
#define pii pair<int,int>
#define pdd pair<double,double>
#define a first
#define b second
using namespace std;

class CubeRenderer;

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
        return (abs(x-a.x)<1e-9)&&(abs(y-a.y)<1e-9)&&(abs(z-a.z)<1e-9);
    }
    
    bool operator!=(const node &a) const {
        return !(*this==a);
    }
    
    node operator+(const node &a) const {
        return {x+a.x,y+a.y,z+a.z};
    }
    
    node operator-(const node &a) const {
        return {x-a.x,y-a.y,z-a.z};
    }
    
    node operator*(const double &a) const {
        return {x*a,y*a,z*a};
    }
    
    node operator/(const double &a) const {
        return {x/a,y/a,z/a};
    }
    
    double length() const {
        return sqrt(x*x+y*y+z*z);
    }
    
    node normalized() const {
        double len=length();
        if(len<1e-10) return {0,0,0};
        return {x/len,y/len,z/len};
    }
};

struct face {
    node M1,M2,M3,M4;
    
    node center() const {
        return (M1+M2+M3+M4)/4.0;
    }
    bool operator==(const face& a)const{
        return M1==a.M1&&M2==a.M2&&M3==a.M3&&M4==a.M4;
    }
};

class CubeRenderer {
private:
    map<pair<string,int>,vector<vector<int>>> all_cube_color;
    string idcube[4]={"","grass","stone","test"};  
    int screen_hw=128;
    
    node _A={-1,1,-1},_B={-1,-1,-1},_C={1,-1,-1},_D={1,1,-1};
    node _A1={-1,1,1},_B1={-1,-1,1},_C1={1,-1,1},_D1={1,1,1};
    
    node A,B,C,D,A1,B1,C1,D1;
    
    node OO={-12,0,0};
    node O={0,0,0};
    
    vector<vector<int>> scr;
    
    face BB,FF,RR,LL,DD,UU;
    
    int y=0,p=0,r=0;
    
    int ttmmpp[20][20];
    
    vector<double>recent_FPS;
    node cross(const node& a,const node& b) {
        return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
    }
    
    double dot(const node& a,const node& b) {
        return a.x*b.x+a.y*b.y+a.z*b.z;
    }
    
    node face_normal(const face& f) {
        node v1=f.M2-f.M1;
        node v2=f.M3-f.M1;
        node normal=cross(v1,v2);
        return normal.normalized();
    }
    
    int rd(double x) {
        if(x<0) return -(int)(-x+0.5);
        return (int)(x+0.5);
    }
    
    double sinn(int x) {
        return sin(x*asin(1)/90);
    }
    
    double coss(int x) {
        return cos(x*asin(1)/90);
    }
    
    void draw_scr(pii x) {
        int px=x.first;
        int py=x.second;
        
        if(px>=0&&px<=screen_hw&&py>=0&&py<=screen_hw) {
            scr[px][py]=1;
        }
    }
    
    bool is_in_face(pii P,face x) {
        pii m1=nd_to_pix(x.M1);
        pii m2=nd_to_pix(x.M2);
        pii m3=nd_to_pix(x.M3);
        pii m4=nd_to_pix(x.M4);
        
        auto cross=[&](pii a,pii b,pii c)->double {
            return (b.first-a.first)*(c.second-a.second)-(b.second-a.second)*(c.first-a.first);
        };
        
        double c1=cross(m1,m2,P);
        double c2=cross(m2,m3,P);
        double c3=cross(m3,m4,P);
        double c4=cross(m4,m1,P);
        
        const double eps=1e-9;
        bool all_non_negative=(c1>=-eps)&&(c2>=-eps)&&(c3>=-eps)&&(c4>=-eps);
        bool all_non_positive=(c1<=eps)&&(c2<=eps)&&(c3<=eps)&&(c4<=eps);
        
        return all_non_negative||all_non_positive;
    }
    
    void gotoxy(int x,int y) {
        COORD pos={static_cast<short>(x),static_cast<short>(y)};
        HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleCursorPosition(hOut,pos);
    }
    
    string cvt(int x) {
        int hun=x/100%10;
        int ten=x/10%10;
        int sng=x%10;
        string res="";
        res+=(char)(hun+'0');
        res+=(char)(ten+'0');
        res+=(char)(sng+'0');
        return res;
    }
    
    long long getTimeOfDayMicros() {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        
        long long microseconds=tv.tv_sec*1000000LL+tv.tv_usec;
        return microseconds;
    }
public:
    CubeRenderer() {
        scr.resize(screen_hw+1,vector<int>(screen_hw+1,0));
        over_all_init();
    }
    
    void set_face() {
        BB={A,B,C,D};//1 need flip
        FF={D1,C1,B1,A1};//2 OK
        RR={D,C,C1,D1};//3 OK
        LL={A1,B1,B,A};//4 need flip
        DD={A,D,D1,A1};//5 need filp
        UU={C,B,B1,C1};//6 OK
    }
    
    bool is_visible(const face& x) {
        node normal=face_normal(x);
        node center=x.center();
        node view_dir=center-OO;
        return dot(view_dir,normal)>0;
    }
    
    pii dir_to_pix(node dir) {
        if(fabs(dir.x)<1e-9) {
            return {screen_hw/2,screen_hw/2};
        }
        pdd ps={dir.y/dir.x*6,dir.z/dir.x*6};
        int px=rd(-screen_hw/2*ps.a+screen_hw/2);
        int py=rd(-screen_hw/2*ps.b+screen_hw/2);
        
        if(px<0) px=0;
        if(px>screen_hw) px=screen_hw;
        if(py<0) py=0;
        if(py>screen_hw) py=screen_hw;
        
        return {px,py};
    }
    
    pii nd_to_pix(node nd) {
        return dir_to_pix(nd-OO);
    }
    
    node rot(node nd,int y,int p,int r) {
        node res={0,0,0};
        double cosY=coss(y);
        double sinY=sinn(y);
        double cosP=coss(p);
        double sinP=sinn(p);
        double cosR=coss(r);
        double sinR=sinn(r);
        
        double m11=cosY*cosP;
        double m12=cosY*sinP*sinR-sinY*cosR;
        double m13=cosY*sinP*cosR+sinY*sinR;
        
        double m21=sinY*cosP;
        double m22=sinY*sinP*sinR+cosY*cosR;
        double m23=sinY*sinP*cosR-cosY*sinR;
        
        double m31=-sinP;
        double m32=cosP*sinR;
        double m33=cosP*cosR;
        
        res.x=m11*nd.x+m12*nd.y+m13*nd.z;
        res.y=m21*nd.x+m22*nd.y+m23*nd.z;
        res.z=m31*nd.x+m32*nd.y+m33*nd.z;
        
        return res;
    }
    
    void draw(node nd) {
        pii x=dir_to_pix(nd-OO);
        draw_scr(x);
    }
    
    void draw_line(node x,node y) {
        pii P=dir_to_pix(x-OO);
        pii Q=dir_to_pix(y-OO);
        
        int x0=P.first,y0=P.second;
        int x1=Q.first,y1=Q.second;
        
        int dx=abs(x1-x0);
        int dy=abs(y1-y0);
        int sx=(x0<x1)?1:-1;
        int sy=(y0<y1)?1:-1;
        int err=dx-dy;
        
        while(true) {
            if(x0>=0&&x0<=screen_hw&&y0>=0&&y0<=screen_hw) {
                scr[x0][y0]=1;
            }
            
            if(x0==x1&&y0==y1) break;
            
            int e2=2*err;
            if(e2>-dy) {
                err-=dy;
                x0+=sx;
            }
            if(e2<dx) {
                err+=dx;
                y0+=sy;
            }
        }
    }
    
    void fill_qd(face x,int color) {
        pii m1=nd_to_pix(x.M1),m2=nd_to_pix(x.M2),m3=nd_to_pix(x.M3),m4=nd_to_pix(x.M4);
        int imax=max(max(m1.a,m2.a),max(m3.a,m4.a));
        int imin=min(min(m1.a,m2.a),min(m3.a,m4.a));
        int jmax=max(max(m1.b,m2.b),max(m3.b,m4.b));
        int jmin=min(min(m1.b,m2.b),min(m3.b,m4.b));
        
        imin=max(0,imin);
        imax=min(screen_hw,imax);
        jmin=max(0,jmin);
        jmax=min(screen_hw,jmax);
        
        for(int i=imin;i<=imax;i++) {
            for(int j=jmin;j<=jmax;j++) {
                if(is_in_face({i,j},x)) {
                    scr[i][j]=color;
                }
            } 
        }
    }
    
    void cr(face x) {
        int face_num=0;
        
        if(x==BB) face_num=1;
        if(x==FF) face_num=2;
        if(x==RR) face_num=3;
        if(x==LL) face_num=4;
        if(x==DD) face_num=5;
        if(x==UU) face_num=6;
        
        for(int i=1;i<=16;i++) {
            for(int j=1;j<=16;j++) {
                double u1=(j-1.0)/16.0;
                double u2=j/16.0;
                double v1=(i-1.0)/16.0;
                double v2=i/16.0;
                
                node M1_tmp=x.M1*((1-u1)*(1-v1))+x.M2*(u1*(1-v1))+x.M3*(u1*v1)+x.M4*((1-u1)*v1);
                node M2_tmp=x.M1*((1-u2)*(1-v1))+x.M2*(u2*(1-v1))+x.M3*(u2*v1)+x.M4*((1-u2)*v1);
                node M3_tmp=x.M1*((1-u2)*(1-v2))+x.M2*(u2*(1-v2))+x.M3*(u2*v2)+x.M4*((1-u2)*v2);
                node M4_tmp=x.M1*((1-u1)*(1-v2))+x.M2*(u1*(1-v2))+x.M3*(u1*v2)+x.M4*((1-u1)*v2);
                
                face tmp={M1_tmp,M2_tmp,M3_tmp,M4_tmp};
                
                if(is_visible(tmp)) {
                    auto it=all_cube_color.find({"grass",face_num});
                    if(it!=all_cube_color.end()&&i-1<it->second.size()&&j-1<it->second[i-1].size()) {
                        fill_qd(tmp,it->second[i-1][j-1]);
                    } else {
                        fill_qd(tmp,0xFFFFFF); 
                    }
                }
            } 
        }
    }
    
    void over_all_init() {
        cout<<"初始化纹理..."<<endl;
        
        int material_count=3;  // grass, stone, test
        
        for(int i=1;i<=material_count;i++) {  
            if(idcube[i]=="") continue;
            
            for(int j=1;j<=6;j++) {
                string adr="./texture/"+idcube[i];
                char cha=j+'0';
                adr+=cha;
                adr+=".txt";
                
                FILE* fp=fopen(adr.c_str(),"r");
                if(fp==NULL) {
                    cout<<"警告：无法打开文件: "<<adr<<endl;
                    continue;
                }
                
                vector<vector<int>> all_color(16,vector<int>(16));
                bool success=true;
                
                for(int ii=0;ii<16&&success;ii++) {
                    for(int jj=0;jj<16;jj++) {
                        int tem;
                        if(fscanf(fp,"%d",&tem)!=1) {
                            cout<<"错误：读取文件失败: "<<adr<<" 在第 "<<ii+1<<" 行, 第 "<<jj+1<<" 列"<<endl;
                            success=false;
                            break;
                        }
                        all_color[ii][jj]=tem;
                    }
                }
                
                fclose(fp);
                
                if(success) {
                    all_cube_color[{idcube[i],j}]=all_color;
                    cout<<"成功加载纹理: "<<idcube[i]<<" 面 "<<j<<endl;
                }
            }
        }
        
        cout<<"纹理初始化完成"<<endl;
        
        for(int k=1;k<=material_count;k++) {  
            if(idcube[k]=="") continue;
            
            if(all_cube_color.find({idcube[k],1})!=all_cube_color.end()) {
                memset(ttmmpp,0,sizeof(ttmmpp));
                for(int i=0;i<=15;i++) {
                    for(int j=0;j<=15;j++) {
                        ttmmpp[15-i][j]=all_cube_color[{idcube[k],1}][i][j]; // B
                    }
                }
                for(int i=0;i<=15;i++) {
                    for(int j=0;j<=15;j++) {
                        all_cube_color[{idcube[k],1}][i][j]=ttmmpp[i][j]; // B
                    }
                }
            }
            
            if(all_cube_color.find({idcube[k],4})!=all_cube_color.end()) {
                memset(ttmmpp,0,sizeof(ttmmpp));
                for(int i=0;i<=15;i++) {
                    for(int j=0;j<=15;j++) {
                        ttmmpp[15-i][j]=all_cube_color[{idcube[k],4}][i][j]; // L
                    }
                }
                for(int i=0;i<=15;i++) {
                    for(int j=0;j<=15;j++) {
                        all_cube_color[{idcube[k],4}][i][j]=ttmmpp[i][j]; // L
                    }
                }
            }
            
            if(all_cube_color.find({idcube[k],5})!=all_cube_color.end()) {
                memset(ttmmpp,0,sizeof(ttmmpp));
                for(int i=0;i<=15;i++) {
                    for(int j=0;j<=15;j++) {
                        ttmmpp[i][15-j]=all_cube_color[{idcube[k],5}][i][j]; //D
                    }
                }
                for(int i=0;i<=15;i++) {
                    for(int j=0;j<=15;j++) {
                        all_cube_color[{idcube[k],5}][i][j]=ttmmpp[i][j]; // D
                    }
                }
            }
        }
    }
    
    void PrintColor(int x,int y,int color_val) {
        int rr=(color_val>>16)&255;
        int gg=(color_val>>8)&255;
        int bb=color_val&255;
        
        gotoxy(x,y);
        printf("\x1b[48;2;%d;%d;%dm",(signed)rr,(signed)gg,(signed)bb);
        printf(" ");
        printf("\x1b[0m");
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
    
    void SlowRender() {
        for(int j=0;j<=screen_hw;j++) {
            for(int i=0;i<=screen_hw;i++) {
                if(scr[i][j]==1) PrintColor(i,j,0xFFFFFF);
                else PrintColor(i,j,0x000000);
            }
        }
    }
    
    void OptimizedRender() {
        for(int y=0;y<=screen_hw;y++) {
            int current_color=-1;
            int run_start=0;
            int run_length=0;
            
            for(int x=0;x<=screen_hw;x++) {
                int color_val=scr[x][y];
                
                if(color_val==current_color) {
                    run_length++;
                } else {
                    if(current_color!=-1) {
                        PrintColoredSequence(run_start,y,run_length,current_color);
                    }
                    
                    current_color=color_val;
                    run_start=x;
                    run_length=1;
                }
            }
            
            if(current_color!=-1) {
                PrintColoredSequence(run_start,y,run_length,current_color);
            }
        }
    }
    
    void CharRender() {
        system("cls");
        string ss="";
        for(int j=0;j<=screen_hw;j++) {
            for(int i=0;i<=screen_hw;i++) {
                if(scr[i][j]) ss+='#';
                else ss+=' ';
            }
            ss+='\n';
        }
        cout<<ss;
    }
    
    vector<vector<int>> calc_scr(int y,int p,int r,int scr_hw) {
        int old_screen_hw=screen_hw;
        screen_hw=scr_hw;
        
        scr.resize(screen_hw+1);
        for(int i=0;i<=screen_hw;i++) {
            scr[i].resize(screen_hw+1,0);
        }
        
        for(int i=0;i<=screen_hw;i++) {
            for(int j=0;j<=screen_hw;j++) {
                scr[i][j]=0;
            }
        }
        
        A=rot(_A,y,p,r);
        B=rot(_B,y,p,r);
        C=rot(_C,y,p,r);
        D=rot(_D,y,p,r);
        A1=rot(_A1,y,p,r);
        B1=rot(_B1,y,p,r);
        C1=rot(_C1,y,p,r);
        D1=rot(_D1,y,p,r);
        set_face();
        
        if(is_visible(BB)) cr(BB);
        if(is_visible(FF)) cr(FF);
        if(is_visible(RR)) cr(RR);
        if(is_visible(LL)) cr(LL);
        if(is_visible(DD)) cr(DD);
        if(is_visible(UU)) cr(UU);
        
        screen_hw=old_screen_hw;
        
        vector<vector<int>> result(scr_hw+1,vector<int>(scr_hw+1));
        for(int i=0;i<=scr_hw;i++) {
            for(int j=0;j<=scr_hw;j++) {
                result[i][j]=scr[i][j];
            }
        }
        
        return result;
    }
    
    void run() {
        srand(time(NULL));
        system("cls");
        
        bool use_optimized=false;
        
        cout<<"选择渲染模式: (1)字符模式 (2)彩色优化模式: ";
        char choice=_getch();
        
        if(choice=='2') {
            use_optimized=true;
            printf("\x1b[?25l");  
        }
        
        while(true) {
            long long start_time=getTimeOfDayMicros();
            calc_scr(y,p,r,screen_hw);
            
            if(use_optimized) {
                OptimizedRender();
            } else {
                CharRender();
            }
            long long end_time=getTimeOfDayMicros();
            double now_FPS=1000000.0/(end_time-start_time);
            recent_FPS.push_back(now_FPS);
            gotoxy(0,0);
            if(use_optimized) {
                printf("\x1b[48;2;0;0;0m\x1b[38;2;255;255;255m");
            }
            cout<<"角度(Y/P/R): "<<y<<"/"<<p<<"/"<<r;
            cout<<"  控制: Q/E(Y)  A/D(P)  W/S(R)  ESC退出";
            cout<<"Screen size: "<<screen_hw<<"*"<<screen_hw<<"  |";
            cout<<"FPS:  ";
            for(int i=max(0,(int)(recent_FPS.size()-15));i<recent_FPS.size();i++)printf("%.5lf ",recent_FPS[i]);
            if(use_optimized) {
                printf("\x1b[0m");
            }
            
            int ch=_getch();
            
            if(ch==27) break;
            
            switch(ch) {
                case 'q': case 'Q':
                    y=(y+10)%360;
                    break;
                case 'e': case 'E':
                    y=(y-10+360)%360;
                    break;
                case 'a': case 'A':
                    p=(p+10)%360;
                    break;
                case 'd': case 'D':
                    p=(p-10+360)%360;
                    break;
                case 'w': case 'W':
                    r=(r+10)%360;
                    break;
                case 's': case 'S':
                    r=(r-10+360)%360;
                    break;
                default:
                    break;
            }
        }
        
        if(use_optimized) {
            printf("\x1b[?25h");  
        }
        
        system("cls");
        cout<<"程序结束"<<endl;
    }
    
    int getY() const { return y; }
    int getP() const { return p; }
    int getR() const { return r; }
    
    void setAngles(int newY,int newP,int newR) {
        y=newY%360;
        p=newP%360;
        r=newR%360;
    }
    
    void setScreenSize(int size) {
        screen_hw=size;
        scr.resize(screen_hw+1,vector<int>(screen_hw+1,0));
    }
};

int main() {
    cout<<"程序启动中..."<<endl;
    /*
    
    调用时 
     
    CubeRenderer renderer;
    vector<vector<int>> scr_res=renderer.calc_scr(y,p,r,scr_hw);
    
    */
    try {
        CubeRenderer renderer;
        renderer.run();
    } catch(const exception& e) {
        cout<<"程序发生异常: "<<e.what()<<endl;
        system("pause");
    }
    return 0;
}
