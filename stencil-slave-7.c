#include<slave.h>
#include<stdio.h>
#include "common.h"
#define local_size_x 256
#define local_size_y 2
#define local_size_z 1

#define slave_x 512
#define slave_y  64
#define slave_z 4

#define size_x 512
#define size_y  512
#define size_z  32
#define halo 1
extern volatile int step;
__thread_local data_t slave_old[2][(local_size_x+2*halo)*(local_size_y+2*halo)*(local_size_z+2*halo)]; 
__thread_local data_t slave_new[2][local_size_x*local_size_y*local_size_z]; 
void func(ptr_t data[3]){
        volatile int    get_reply[2],put_reply[2];
        int index,next,pre,cur,ne;
        int cnt=-1;
        int my_id = athread_get_id(-1); 
        index=step%2;
        next=(step+1)%2;
        int xx,yy,zz;
	const int times=(slave_x/local_size_x)*(slave_y/local_size_y)*(slave_z/local_size_z); //the iteration times
	const int get_answer=(local_size_y+2*halo)*(local_size_z+2*halo); //the get_reply answer size
	const int put_answer=local_size_y*local_size_z; //the put_reply answer size
        for(int z=0;z<slave_z;z+=local_size_z){ 
                for(int y=0;y<slave_y;y+=local_size_y){ 
                        for(int x=0;x<slave_x;x+=local_size_x){ 
                                //get data
                                cnt++;
                                cur=cnt%2;
                                ne=(cnt+1)%2;
                                pre=(cnt-1)%2;
                                if(x==0){
                                        zz=z;yy=y;xx=local_size_x;
                                }
                                else{
                                        if(y==slave_y-local_size_y){zz=z+local_size_z;yy=0;xx=0;}
                                        else{zz=z;yy=y+local_size_y;xx=0;}
                                }
				
                                if(cnt==0){
                                        get_reply[cur]=0;
                                        for(int i=0;i<local_size_z+2*halo;i++){
                                                for(int j=0;j<local_size_y+2*halo;j++){
                                                        athread_get(PE_MODE,&(data[index][INDEX(x,(my_id%8)*64+y+j,(my_id/8)*4+z+i,size_x+2*halo,size_y+2*halo)]),\
                                                        &(slave_old[cur][INDEX(0,j,i,local_size_x+2*halo,local_size_y+2*halo)]),(local_size_x+2*halo)*8,\
                                                        &get_reply[cur], 0, 0, 0);
                                                }
                                        }
                                }   
                                if(cnt<times-1){
                                        get_reply[ne]=0;
                                        for(int i=0;i<local_size_z+2*halo;i++){
                                                for(int j=0;j<local_size_y+2*halo;j++){  
                                                        athread_get(PE_MODE,&(data[index][INDEX(xx,(my_id%8)*64+yy+j,(my_id/8)*4+zz+i,size_x+2*halo,size_y+2*halo)]),\                                                                                                                                                                                                
                                                        &(slave_old[ne][INDEX(0,j,i,local_size_x+2*halo,local_size_y+2*halo)]),(local_size_x+2*halo)*8,\
                                                        &get_reply[ne], 0, 0, 0);
                                                }
                                        }
                                }
                                while(get_reply[cur]!=get_answer) ;


                                //caculation
                                for(int i=halo;i<local_size_x+halo;i++){
                                        for(int j=halo;j<local_size_y+halo;j++){
                                                for(int k=halo;k<local_size_z+halo;k++){
                                                        slave_new[cur][INDEX(i-halo,j-halo,k-halo,local_size_x,local_size_y)]\
                                                        =ALPHA_ZZZ*slave_old[cur][INDEX(i,j,k,local_size_x+2*halo,local_size_y+2*halo)]\
                                                        +ALPHA_NZZ*slave_old[cur][INDEX(i-1,j,k,local_size_x+2*halo,local_size_y+2*halo)]\
                                                        +ALPHA_PZZ*slave_old[cur][INDEX(i+1,j,k,local_size_x+2*halo,local_size_y+2*halo)]\
                                                        +ALPHA_ZNZ*slave_old[cur][INDEX(i,j-1,k,local_size_x+2*halo,local_size_y+2*halo)]\
                                                        +ALPHA_ZPZ*slave_old[cur][INDEX(i,j+1,k,local_size_x+2*halo,local_size_y+2*halo)]\
                                                        +ALPHA_ZZN*slave_old[cur][INDEX(i,j,k-1,local_size_x+2*halo,local_size_y+2*halo)]\
                                                        +ALPHA_ZZP*slave_old[cur][INDEX(i,j,k+1,local_size_x+2*halo,local_size_y+2*halo)];
                                                }
                                        }
                                }
                                //put data
                                //
                                //                                                             

                                put_reply[cur]=0;
                                for(int i=0;i<local_size_z;i++){
                                        for(int j=0;j<local_size_y;j++){
                                                athread_put(PE_MODE,&slave_new[cur][INDEX(0,j,i,local_size_x,local_size_y)],\
                                                &(data[next][INDEX(x+halo,(my_id%8)*64+y+j+halo,(my_id/8)*4+z+i+halo,size_x+2*halo,size_y+2*halo)]),\
                                                local_size_x*8,&put_reply[cur], 0, 0);
                                        }
                                }
                                if(cnt>0) {while(put_reply[pre]!=put_answer);}
                                if(cnt==times-1) { while(put_reply[cur]!=put_answer);}
                        }
                }
        }
        if(my_id==0) step++;
}
                                                                                                                                                                                                 


