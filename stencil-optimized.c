#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include<athread.h>
#include<mpi.h>
const char* version_name = "Optimized version";
volatile int step=0;
extern SLAVE_FUN(func)();
int f=0;
/* your implementation */
void create_dist_grid(dist_grid_info_t *grid_info, int stencil_type) {
	grid_info->local_size_x = 512;  
	grid_info->local_size_y = 512; 
	grid_info->local_size_z = 32;
	grid_info->offset_x=0;
	grid_info->offset_y=0;
	grid_info->offset_z=32*grid_info->p_id;
   	grid_info->halo_size_x = 1;
 	grid_info->halo_size_y = 1;
 	grid_info->halo_size_z = 1;
	   
}

/* your implementation */
void destroy_dist_grid(dist_grid_info_t *grid_info) {
	
}
//#define INDEX(xx, yy, zz, ldxx, ldyy) ((xx) + (ldxx) * ((yy) + (ldyy) * (zz)))
/* your implementation */
ptr_t stencil_7(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info, int nt) {
	double id=grid_info->p_id;
	ptr_t  buffer[3]={grid,aux,&id};
	
	int ldx = grid_info->local_size_x+2*grid_info->halo_size_x ; 
	int ldy = grid_info->local_size_y+2*grid_info->halo_size_y;
      	if(f==0) {
		athread_init(); f=1;
	}else {
		step=0;
	}
	    
	while(step<nt)  //步数迭代
	{	
		ptr_t a0 = buffer[step % 2];
       	 	ptr_t a1 = buffer[(step + 1) % 2];
		for(int i=0;i<512;i++){ 
			MPI_Request request1,request2;
			MPI_Status ierr1,ierr2;
			if(grid_info->p_id<15){
				//接收上面的	
				MPI_Irecv(&(a0[INDEX(grid_info->halo_size_x,i+grid_info->halo_size_y,grid_info->local_size_z+grid_info->halo_size_z,ldx,ldy)]),
				grid_info->local_size_x,MPI_DOUBLE,grid_info->p_id+1,1,MPI_COMM_WORLD,&request1);
			}
			if(grid_info->p_id>0){
				//接收下面的
				MPI_Irecv(&(a0[INDEX(grid_info->halo_size_x,i+grid_info->halo_size_y,0,ldx,ldy)]),grid_info->local_size_x,
				MPI_DOUBLE,grid_info->p_id-1,0,MPI_COMM_WORLD,&request2);
			}
			if(grid_info->p_id<15){
				//向上发				    
				MPI_Send(&(a0[INDEX(grid_info->halo_size_x,i+grid_info->halo_size_y,grid_info->local_size_z,ldx,ldy)]),
				grid_info->local_size_x,MPI_DOUBLE,grid_info->p_id+1,0,MPI_COMM_WORLD);
			}
			if(grid_info->p_id>0){
				//向下发
				MPI_Send(&(a0[INDEX(grid_info->halo_size_x,i+grid_info->halo_size_y,grid_info->halo_size_z,ldx,ldy)]),
				grid_info->local_size_x,MPI_DOUBLE,grid_info->p_id-1,1,MPI_COMM_WORLD);
			}	
			if(grid_info->p_id<16-1){
				MPI_Wait(&request1,&ierr1);							
			}
			if(grid_info->p_id>0){
				MPI_Wait(&request2,&ierr2);		
			}
		}	
		//进程间更新结束
		athread_spawn(func,buffer);	//从核计算
		athread_join();
		MPI_Barrier(MPI_COMM_WORLD);
	
	}
	if(f==1){
		f++;	
	}else{
		athread_halt();
	} 
    	return buffer[nt%2];
}

/* your implementation */
ptr_t stencil_27(ptr_t grid, ptr_t aux, const dist_grid_info_t *grid_info, int nt) {
    return grid;
}
