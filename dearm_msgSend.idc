/*

dearm_msgSend.idc ... IDA Pro 5.x script to "demange" native
objc_msgSend calls for iPhoneOS/arm binaries.

Copyright (C) 2012 x264msna

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifndef _IDC_objc_msgSend
#define _IDC_objc_msgSend

#include <idc.idc>

//#define MSGSEND_DEBUG
 
#ifndef MSGSEND_DEBUG
#define Message //
#endif

static strcmp(str1, str2){
 return strlen(str1) == strlen(str2) && strstr( str1, str2 ) == 0;
}

static main(void)
{
 auto Start, Stop, i, RelAddr, b;
 auto OldAddr, NewAddr;
 
 CreateArray("arm_regs");
 
 auto arm_regs = GetArrayId("arm_regs");
 
 Start = ScreenEA();
 if(Start == BADADDR)
 {
   Message("Invalid address");
   return;
 }

 Stop = FindFuncEnd(Start);
 if(Stop == BADADDR)
 {
   Message("Not in function or invalid address");
   return;
 }  

 Start = GetFunctionAttr(Start, FUNCATTR_START);

 Message("Func <%x, %x>...\n", Start, Stop);

 RelAddr = 0;

 for(b = 0; b <= 15; b++ ){
	SetArrayLong(arm_regs,b,0);
 }

 for(i = Start; i < Stop; i = ItemEnd(i))
 {
   if( strcmp(GetMnem(i), "MOV" ) ){
     auto cur_reg = substr( GetOpnd(i,0), 1,-1 );
     auto cur_val = "";
     if( strstr( GetOpnd(i,1), "#") == 0 ){
	     cur_val = substr( GetOpnd(i,1), 1,-1 );
     }
     else
     if( strstr( GetOpnd(i,1), "0x") == 0 ){
	     cur_val = substr( GetOpnd(i,1), 0,-1 );
     }

     if(strlen(cur_val) > 0 ){
	SetArrayLong(arm_regs,cur_reg, xtol(cur_val) );
     	Message("MOV %s [%s=%x|%s]\n", GetMnem(i), cur_reg, GetArrayElement(AR_LONG, arm_regs, cur_reg ), cur_val );
     }

   }
   else
   if( strcmp(GetMnem(i), "MOVT" ) ){
     cur_reg = substr( GetOpnd(i,0), 1,-1 );
     if( strstr( GetOpnd(i,1), "#") == 0 ){
	     auto cur_i = GetArrayElement(AR_LONG, arm_regs, cur_reg );
	     auto cur_x = xtol(substr( GetOpnd(i,1), 1,-1 ));
             Message("MOVT %s [%s=%x|%x] = %x ", GetMnem(i), cur_reg, cur_i, cur_x , ((cur_x << 16) | cur_i) );
	     SetArrayLong(arm_regs,cur_reg, ((cur_x << 16) | cur_i) );
             Message("new check = %x\n", GetArrayElement(AR_LONG, arm_regs, cur_reg ) );
     }
   }
   else
   if( strcmp(GetMnem(i), "ADD" ) ){
     cur_reg = substr( GetOpnd(i,0), 1,-1 );
     if( strstr( GetOpnd(i,1), "PC") == 0 ){
	     cur_i = GetArrayElement(AR_LONG, arm_regs, cur_reg );
	     cur_x = i;
             Message("%s PC [%s=%x|%x] ", GetMnem(i), cur_reg, cur_i, cur_x );
	     SetArrayLong(arm_regs,cur_reg, (cur_i + cur_x + 4) );
             Message("new check = %x\n", GetArrayElement(AR_LONG, arm_regs, cur_reg ) );
     }
   }
   else
   if( strcmp(GetMnem(i), "LDR" ) ){
     cur_reg = substr( GetOpnd(i,0), 1,-1 );
     auto cur_last = strstr( GetOpnd(i,1), "]");
     auto cur_first = strstr( GetOpnd(i,1), "[R");
     if( cur_first == 0 && cur_last != -1 ){
	     auto cur_ref = atol( substr( GetOpnd(i,1), 2, cur_last ) );
Message("check |%s|%s| and %x\n\n", GetOpnd(i,1), substr( GetOpnd(i,1), 2, cur_first ), cur_last );
	     cur_x = i;
             Message("%s [%s=%s|%x]\n", GetMnem(i), cur_reg, cur_ref, GetArrayElement(AR_LONG, arm_regs, cur_ref ) );
	     SetArrayLong(arm_regs,cur_reg, GetArrayElement(AR_LONG, arm_regs, cur_ref ) );
     }
   }
   else   
   if( GetOpnd(i,0) == "_objc_msgSend"  ){
   	if( GetMnem(i) == "BLX" ) {
		add_dref(i, GetArrayElement(AR_LONG, arm_regs, "1" ),  dr_O); 
 		Message(">>>>>>>>>>>call (%s)[%s] to %x\n", GetMnem(i), GetOpnd(i,0), GetArrayElement(AR_LONG, arm_regs, "1" ) );
 	}
 	else
 		Warning("found not BLX for _objc_msgSend!\n");
 		
   }
   
//	Message("found (%s)[%s|%s]\n", GetMnem(i), GetOpnd(i,0), GetOpnd(i,1) );
 
 }
}

#endif // _IDC_objc_msgSend