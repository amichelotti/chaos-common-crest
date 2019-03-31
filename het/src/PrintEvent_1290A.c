#include <stdlib.h>

void PrintEvent_1290A(unsigned int data) {

  unsigned int flag_ev;
  int flag_count,flag_geo;
  int flag_evid,flag_bunchid;
  int flag_trle,flag_chan,flag_data;
  int flag_wordc;
  int flag_error;
  int flag_trig_tt;
  int flag_trai_wc,flag_trai_tdce,flag_trai_obo,flag_trai_tl;

  unsigned int id_ev;
  int id_count,id_geo;
  int id_evid,id_bunchid;
  int id_trle,id_chan,id_data;
  int id_wordc;
  int id_error;
  int id_trig_tt;
  int id_trai_wc,id_trai_tdce,id_trai_obo,id_trai_tl,id_trai_geo;
  
  int ret = 0;

// Global Header 
  flag_ev        = 0xF8000000;
  flag_count     = 0x07FFFFE0;
  flag_geo       = 0x0000001F;
// TDC Header
  flag_evid      = 0x00FFF000;  
  flag_bunchid   = 0x00000FFF;
// TDC Measurement
  flag_trle      = 0x04000000;
  flag_chan      = 0x03E00000;
  flag_data      = 0x001FFFFF;
// TDC Trailer
  flag_wordc     = 0x00000FFF;
// TDC Error
  flag_error     = 0x00007FFF;
// Trigger Time Tag 
  flag_trig_tt   = 0x07FFFFFF;
// Trailer 
  flag_trai_wc   = 0x001FFFE0;
  flag_trai_tdce = 0x01000000;
  flag_trai_obo  = 0x02000000;
  flag_trai_tl   = 0x04000000;

  id_ev = (data & flag_ev) >> 27;

  switch (id_ev) {
    case 0 : 
      printf("Output Buffer : TDC Measurement\n");
      id_trle = (data & flag_trle) >> 26;
      id_chan = (data & flag_chan) >> 21;
      id_data = (data & flag_data);
/*
      printf(" id_trle : %x (1=trailing, 0=leading) \n",id_trle);
      printf(" id_chan : %x %d \n",(data&flag_chan),id_chan);
      printf(" id_data : %x %d \n",(data&flag_data),id_data);
      printf(" *** TDC_1290 *** CH: %d  TIME(ps): %d \n", id_chan, id_data*25);
      dr->id_trle[id_chan] = id_trle;
      dr->id_chan[id_chan] = id_chan;
      dr->id_data[id_chan] = id_data;
*/
    
      break;
    case 1:
      printf("Output Buffer : TDC Header\n");
      id_evid = (data & flag_evid) >> 12;
      id_bunchid = (data & flag_bunchid);
/*
      printf(" id_evid : %x %d \n",(data&flag_evid),id_evid);
      printf(" id_bunchid : %x %d \n",(data&flag_bunchid),id_bunchid);
      dr->id_evidh = id_evid;
      dr->id_bunchid = id_bunchid;
*/

      break;
    case 3:
      printf("Output Buffer : TDC Trailer\n");
      id_evid = (data & flag_evid) >> 12;
      id_wordc = (data & flag_wordc);
/*
      printf(" id_evid : %x %d \n",(data&flag_evid),id_evid);
      printf(" id_wordc : %x %d \n",(data&flag_wordc),id_wordc);
      dr->id_wordc = id_wordc;
      dr->id_evidt = id_evid;
*/

      break;
    case 4:
      printf("Output Buffer : TDC Error\n");
      id_error = (data & flag_error);
/*
      printf(" id_error : %x %d \n",(data&flag_error),id_error);
      dr->id_error = id_error;
*/

      break;
    case 8:
      printf("Output Buffer : Global Header \n");
      id_count = (data & flag_count) >> 5;
      id_geo = (data & flag_geo);
/*
      printf(" id_count : %x %d \n",(data&flag_count),id_count);
      printf(" id_geo : %x %d \n",(data&flag_geo),id_geo);
      dr->id_count = id_count;
      dr->id_geo = id_geo;
*/

      break;
    case 16:
      printf("Output Buffer : Trailer\n\n");
      id_trai_tl = (data & flag_trai_tl) >> 26;
      id_trai_obo = (data & flag_trai_obo) >> 25;
      id_trai_tdce = (data & flag_trai_tdce) >> 24;
      id_trai_wc = (data & flag_trai_wc) >> 5;
      id_trai_geo = (data & flag_geo);
/*
      printf(" id_trai_tl : %x %d \n",(data&flag_trai_tl),id_trai_tl);
      printf(" id_trai_obo : %x %d \n",(data&flag_trai_obo),id_trai_obo);
      printf(" id_trai_tdce : %x %d \n",(data&flag_trai_tdce),id_trai_tdce);
      printf(" id_trai_wc : %x %d \n",(data&flag_trai_wc),id_trai_wc);
      printf(" id_trai_geo : %x %d \n",(data&flag_geo),id_trai_geo);
      dr->id_trai_wc = id_trai_wc;
      dr->id_trai_tdce = id_trai_tdce;
      dr->id_trai_obo = id_trai_obo;
      dr->id_trai_tl = id_trai_tl;
      dr->id_trai_geo = id_trai_geo;
*/

      break;
    case 17:
      printf("Output Buffer : Extended Trigger Time Tag\n");
      id_trig_tt = (data & flag_trig_tt);
/*
      printf(" id_trig_tt : %x %d \n",(data&flag_trig_tt),id_trig_tt);
      dr->id_trig_tt = id_trig_tt;
*/

      break;
    default: 
      printf("Boh id = %d data = %08x \n",id_ev,data);

      break;
  }
  
}
