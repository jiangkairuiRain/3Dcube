#include<bits/stdc++.h>
using namespace std;
int a[20][20];
int tmp[20][20];
void rot(){
	for(int i=1;i<=16;i++){
		for(int j=1;j<=16;j++){
			tmp[j][17-i]=a[i][j];
		}
	}
	for(int i=1;i<=16;i++){
		for(int j=1;j<=16;j++){
			a[i][j]=tmp[i][j];
		}
	}
}
int main(){
	string ss="test";
	for(int k=1;k<=6;k++){
		string s=ss+(char)('0'+k);
		s+=".txt";
		FILE *fp=fopen(s.c_str(),"r");
		for(int i=1;i<=16;i++){
			for(int j=1;j<=16;j++){
				fscanf(fp,"%d",&a[i][j]);
			}
		}
		fclose(fp);
		for(int c=0;c<=3;c++){
			string sss="rot";
			sss+=(char)('0'+k);
			sss+=' ';
			sss+=(char)('0'+c);
			sss+=".txt";
			
			FILE *fp1=fopen(sss.c_str(),"w");
			for(int i=1;i<=16;i++){
				for(int j=1;j<=16;j++){
					fprintf(fp1,"%d ",a[i][j]);
				}
				fprintf(fp1,"\n");
			}
			fclose(fp1);
			rot();
		}
	}
	
	return 0;
} 
