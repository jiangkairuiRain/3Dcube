#include<bits/stdc++.h>
using namespace std;
int a[20][20];
int tmp[20][20];
int main(){
	for(int i=1;i<=16;i++){
		for(int j=1;j<=16;j++){
			cin >> a[i][j];
			tmp[i][17-j]=a[i][j];
		}
	} 
//	1
//	for(int i=1;i<=16;i++){
//		for(int j=1;j<=16;j++){
//			cin >> a[i][j];
//			tmp[17-i][j]=a[i][j];
//		}
//	} 
//	2
	cout << endl;
	for(int i=1;i<=16;i++){
		for(int j=1;j<=16;j++){
			cout << tmp[i][j] << ' ';
		}
		cout << endl;
	} 
	return 0;
} 
