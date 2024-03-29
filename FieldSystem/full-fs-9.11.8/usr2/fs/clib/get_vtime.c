/* get time setting information from mcbcns */

#include <memory.h>

#include "../include/params.h"
#include "../include/req_ds.h"
#include "../include/res_ds.h"

get_vtime(centisec,fm_tim,ip,to)
long centisec[6];
int fm_tim[6];
long ip[5];                          /* ipc array */
int to;
{
      struct req_buf buffer;
      struct req_rec request;
      struct res_buf buffer_out;
      struct res_rec response;

      ini_req(&buffer);                      /* format the buffer */
      memcpy(request.device,DEV_VFM,2);
      request.type=5; 
      request.addr=0x2B;
      add_req(&buffer,&request);
      
      request.type=1; 
      request.addr=0x2A; add_req(&buffer,&request);
      request.addr=0x29; add_req(&buffer,&request);
      request.addr=0x28; add_req(&buffer,&request);

      end_req(ip,&buffer);                  /* send buffer and schedule */
      if(to!=0) {
	char *name;
	name="mcbcn";
	while(skd_run_to(name,'w',ip,120)==1) {
	  if (nsem_test("fs   ") != 1) {
	    return 1;
	  }
	  name=NULL;
	}
      }	else
	skd_run("mcbcn",'w',ip);
      skd_par(ip);
      if(ip[2] <0) return 0;

      opn_res(&buffer_out,ip);              /* decode response */
      get_res(&response, &buffer_out);
      memcpy(centisec,response.array,24);
      fm_tim[0]=0;
      fm_tim[1]=    (0xF & response.data   ) +  10*(0xF &response.data>>4);
      fm_tim[2]=    (0xF & response.data>>8) +  10*(0xF &response.data>>12);

      get_res(&response, &buffer_out);
      fm_tim[3]=      (0xF & response.data) +  10*(0xF &response.data>>4);

      get_res(&response, &buffer_out);
      fm_tim[4]=      (0xF & response.data) +  10*(0xF &response.data>>4);
      fm_tim[4]+= 100*(0xF & response.data>>8);

      get_res(&response, &buffer_out);
      fm_tim[5]=      (0xF & response.data) +  10*(0xF &response.data>>4);
      fm_tim[5]+=  100*(0xF & response.data>>8);
      fm_tim[5]+= 1000*(0xF & response.data>>12);

      if(response.state == -1) {
        clr_res(&buffer_out);
        ip[2]=-990;
	memcpy(ip+3,"vf",2);
        return 0;
      } 

       clr_res(&buffer_out);
       return 0;
}
