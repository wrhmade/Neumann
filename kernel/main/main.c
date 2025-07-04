/*
main.c
进入C语言后的第一个程序
Copyright W24 Studio 
*/

#include <nasmfunc.h>
#include <binfo.h> 
#include <macro.h>
#include <sysset.h>
#include <graphic.h>
#include <desktop.h>
#include <gdtidt.h>
#include <mouse.h>
#include <keyboard.h>
#include <fifo.h>
#include <mm.h>
#include <sheet.h>
#include <window.h>
#include <timer.h>
#include <task.h>
#include <stddef.h>
#include <console.h>
#include <cmos.h>
#include <ini.h>
#include <string.h>
#include <com.h>
#include <message.h>
#include <ime.h>
#include <krnlcons.h>
#include <fpu.h>
#include <cpu.h>
#include <pci.h>
#include <fdc.h>
#include <syscall.h>
#include <vfs.h>
#include <vdisk.h>
#include <devfs.h>
#include <fat.h>
#include <hd.h>
#include <fullscreen.h>
#include <dbuffer.h>
#include <ide.h>
#include <iso9660.h>
#include <stdio.h>
#include <exec.h>
#include <nullzero.h>

extern fifo_t decoded_key;
extern fifo_t mouse_fifo;
extern ime_status_t *status;
extern fifo_t key_fifo;
mdec_t mdec;

shtctl_t *global_shtctl;
sheet_t *global_sht_back;
uint32_t *global_buf_back;
int global_mousebtn;
int process=0;
int mouse_x,mouse_y;
sheet_t *keywin;

void _PUTSTR(console_t *console,char *s)
{
	if(console==0)
	{
		krnlcons_putstr(s);
	}
	else
	{
		console_putstr(console,s);
	}
}

void print_pcinfo(console_t *console)
{
	char s[200];
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
	_PUTSTR(console,"\n\nComputer Info\n-------------------------------------------------------------------------------\nItem\t\t\t\tValue\n-------------------------------------------------------------------------------\n");
	_PUTSTR(console,"CPU\n");

	cpu_version_t ver;
	cpu_vendor_t vendor;
	cpu_version(&ver);
	cpu_vendor_id(&vendor);

	_PUTSTR(console,"\tModel Name\t\t");
	char model_name[50];
	cpu_get_model_name(model_name);
	sprintf(s,"%s\n",model_name);
	_PUTSTR(console,s);

	_PUTSTR(console,"\tVendor\t\t\t");
	sprintf(s,"%s\n",vendor.info);
	_PUTSTR(console,s);


	_PUTSTR(console,"\tFPU\t\t\t\t");
	if(ver.FPU)
	{
		_PUTSTR(console,"True\n");
	}
	else
	{
		_PUTSTR(console,"False\n");
	}
	_PUTSTR(console,"\tBase Count\t\t");
	sprintf(s,"%08x\n",binfo->base_count);
	_PUTSTR(console,s);

	
	_PUTSTR(console,"Memory\t\t\t\t");
	sprintf(s,"%dMB\n",binfo->memtotal/1024/1024);
	_PUTSTR(console,s);

	_PUTSTR(console,"PCI\t\t\t\t\t");
	sprintf(s,"%d Total\n",count_pci_device());
	_PUTSTR(console,s);

	_PUTSTR(console,"Storage Devices\n");
	_PUTSTR(console,"\tFloppy Disk\t\t");
	int fdinfo=get_fdinfo();
	if(fdinfo==-1)
	{
		_PUTSTR(console,"None\n");
	}
	else
	{
		switch(fdinfo)
		{
			case 0:
				_PUTSTR(console,"3.5-inch/1.44MB\n");
				break;
			case 1:
				_PUTSTR(console,"3.5-inch/1.68MB\n");
				break;
		}
		
	}
	_PUTSTR(console,"\tIDE\n");
	ide_info_t info;
	_PUTSTR(console,"\t\tPri. Master\t");
	get_ide_info(0,0,&info);
	if(info.reserved)
	{
		sprintf(s,"%s [%s,%dKB]\n",info.name,info.type?"ATAPI":"ATA",info.size/2);
		_PUTSTR(console,s);
	}
	else
	{
		_PUTSTR(console,"(None)\n");
	}
	
	_PUTSTR(console,"\t\tPri. Slave\t");
	get_ide_info(0,1,&info);
	if(info.reserved)
	{
		sprintf(s,"%s [%s,%dKB]\n",info.name,info.type?"ATAPI":"ATA",info.size/2);
		_PUTSTR(console,s);
	}
	else
	{
		_PUTSTR(console,"(None)\n");
	}

	_PUTSTR(console,"\t\tSec. Master\t");
	get_ide_info(1,0,&info);
	if(info.reserved)
	{
		sprintf(s,"%s [%s,%dKB]\n",info.name,info.type?"ATAPI":"ATA",info.size/2);
		_PUTSTR(console,s);
	}
	else
	{
		_PUTSTR(console,"(None)\n");
	}

	_PUTSTR(console,"\t\tSec. Slave\t");
	get_ide_info(1,1,&info);
	if(info.reserved)
	{
		sprintf(s,"%s [%s,%dKB]\n",info.name,info.type?"ATAPI":"ATA",info.size/2);
		_PUTSTR(console,s);
	}
	else
	{
		_PUTSTR(console,"(None)\n");
	}
	//_PUTSTR(console,s);

}

sheet_t *sht_mouse;
uint32_t *buf_mouse;

void taskc_main(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int mmx=-1,mmy=-1,mmx2=0,x,y;
	int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0,i;
	
	sheet_t *sht;
	uint8_t mouse_data,keyboard_data;
	for(;;)
	{

		if(fifo_status(&mouse_fifo)>0)
		{
			mouse_data=fifo_get(&mouse_fifo);
			if(mouse_decode(&mdec,mouse_data)!=0)
			{
				//boxfill(binfo->vram,binfo->scrnx,mouse_x,mouse_y,mouse_x+23,mouse_y+23,DESKTOP_BACKCOLOR);
				mouse_x+=mdec.x;
				mouse_y+=mdec.y;
				if (mouse_x < 0) {
					mouse_x = 0;
				}
				if (mouse_y < 0) {
					mouse_y = 0;
				}
				if (mouse_x > binfo->scrnx-1)
				{
					mouse_x = binfo->scrnx-1;
				}
				if (mouse_y > binfo->scrny-1)
				{
					mouse_y = binfo->scrny-1;
				}
				new_mx=mouse_x;
				new_my=mouse_y;
				global_mousebtn=mdec.btn;
				if((mdec.btn & 0x01)!=0)
				{
					if(mmx<0)
					{
						for(i=global_shtctl->top-1;i>0;i--)
						{
							sht = global_shtctl->sheets[i];
							x=mouse_x-sht->vx0;
							y=mouse_y-sht->vy0;
							if(0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize && (sht->window!=0 || sht->movable) && !get_isfullscreen())
							{
								sheet_updown(sht,global_shtctl->top-1);
								keywin=sht;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < 18) {
    		                        mmx = mouse_x;
    		                        mmy = mouse_y;
    		                        mmx2 = sht->vx0;
    		                        new_wy = sht->vy0;
    		                    }
								if(sht->bxsize-17 <= x && x<= sht->bxsize-1 && 1<=y && y<=17 && sht->window->close_btn)
								{
									if(sht->window->isconsole)
									{
										close_console(sht->window->console);
									}
									else
									{
										 if((sht->flags & 0x10) != 0)
										 {
										 	app_kill(sht);
										 }
										 else
										 {
											if(sht->window->task!=0)
											{
												task_remove(sht->window->task);
											}
											close_window(sht->window);
										}
									}
								}
								break;
							}
						}
					}
					else
					{
						x = mouse_x - mmx;
    		            y = mouse_y - mmy;
    		            new_wx = (mmx2 + x + 2) & ~3;
    		            new_wy = new_wy + y;
    		            mmy = mouse_y;
					}
				}
				else
				{
					mmx = -1; /* 记录鼠标没有按下左键 */
    		        /* 将窗口移动到鼠标移动位置处,并标识窗口已移动过一次 */
    		        if (new_wx != 0x7fffffff) {
    		            move_window(sht->window, new_wx, new_wy);
    		            new_wx = 0x7fffffff;
    		        }
				}
			}
		}
		if(fifo_status(&decoded_key)>0)
		{
			keyboard_data=fifo_get(&decoded_key);
			if(!status->enabled)
			{
				fifo_put(&keywin->window->task->fifo,keyboard_data+256);
			}
			else
			{
				fifo_put(&key_fifo,keyboard_data+256);
			}
		}
		

		if(fifo_status(&decoded_key)==0 && fifo_status(&mouse_fifo)==0)
		{
			if (new_mx >= 0) {
                sheet_slide(sht_mouse, new_mx, new_my);
                new_mx = -1;
            } else if (new_wx != 0x7fffffff) {
                sheet_slide(sht, new_wx, new_wy);
                new_wx = 0x7fffffff;
            }
		}
	}
}

void taskb_main(void)
{
	current_time_t *ctime=(current_time_t *)kmalloc(sizeof(current_time_t));
	int year,month,day,hour,minute,second;
	char iWeek_str[10],result[60];
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int _year=0,_month=0,_day=0,_hour=0,_minute=0,_second=0;
	for(;;)
	{
		get_current_time(ctime);
		year=ctime->year;
		month=ctime->month;
		day=ctime->day;
		hour=ctime->hour;
		minute=ctime->min;
		second=ctime->sec;
		if(_year==year && _month==month && _day==day && _hour==hour && _minute==minute && _second==second)
		{
			continue;
		}

		if(month==1 || month==2)
		{
			month+=12;
			year--;
		}
		int iWeek=(day+2*month+3*(month+1)/5+year+year/4-year/100+year/400)%7;
		switch(iWeek)
		{
			case 0:strcpy(iWeek_str,"Mon. ");break;
			case 1:strcpy(iWeek_str,"Tues.");break;
			case 2:strcpy(iWeek_str,"Wed. ");break;
			case 3:strcpy(iWeek_str,"Thur.");break;
			case 4:strcpy(iWeek_str,"Fri. ");break;
			case 5:strcpy(iWeek_str,"Sat. ");break;
			case 6:strcpy(iWeek_str,"Sun. ");break;
		}
		sprintf(result, "%04d/%02d/%02d %s", ctime->year,ctime->month, ctime->day,iWeek_str);
		putstr_ascii_sheet(global_sht_back,binfo->scrnx-190,binfo->scrny-20,0x000000,DESKTOP_BACKCOLOR,result);
		sprintf(result, "%02d:%02d:%02d", hour, minute,second);
		putstr_ascii_sheet(global_sht_back,binfo->scrnx-69,binfo->scrny-20,0x000000,DESKTOP_BACKCOLOR,result);

	}
}

void log(char *s)
{
	krnlcons_putstr(s);
	krnlcons_putchar('\n');
}


void ready_system(void);

void krnlc_main(void)
{

	
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	//int mmx=-1,mmy=-1,mmx2=0,x,y;
	//int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
	uint32_t memtotal;
	process=0;
	char s[40];
	shtctl_t *shtctl;
	sheet_t *sht_back;
	uint32_t *buf_back;
	console_t *console;
	
	krnlcons_display();

	// boxfill(binfo->vram,binfo->scrnx,20,20,300,300,0xFF0000);
	// boxfill(binfo->vram,binfo->scrnx,40,40,320,320,0x00FF00);
	// boxfill(binfo->vram,binfo->scrnx,60,60,340,340,0x0000FF);
	
	log("Neumann Operating System\nVersion 0.8 [Beta 6]\nCopyright(c) 2023-2025 W24 Studio & 71GN Deep Space");
	log("Now initializing system...");

	log("Initializing GDT and IDT...");
	init_gdtidt();

	

	log("Initializing Serial Port...");
	init_com();
	

	log("Initializing PS/2 Keyboard...");
	init_keyboard();

	log("Initializing PS/2 Mouse...");
	init_ps2mouse();
	
	//for(;;);

	log("Initializing Memory...");
	memtotal=init_mem();

	sprintf(s,"INFO:Memory Total:%dMB",memtotal/1024/1024);
	log(s);
	
	void *test_buf=kmalloc(20);
	sprintf(s,"INFO:test_buf at %p\n",test_buf);
	log(s);

	//sprintf(s,"memtotal=%uMB",memtotal/1024/1024);
	
	
	
	binfo->memtotal=memtotal;
	

	//putstr_ascii(binfo->vram,binfo->scrnx,0,0,0x000000,s);

	log("Initializing Double Buffer...");
	init_dbuffer();
	

	log("Initializing Sheet...");
	
	shtctl = shtctl_init(binfo->vram, binfo->scrnx, binfo->scrny);
	global_shtctl=shtctl;
	
	sht_back  = sheet_alloc(shtctl);
    buf_back  = (uint32_t*) kmalloc(sizeof(uint32_t)*binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny,-1);
	global_sht_back=sht_back;
	global_buf_back=buf_back;

	log("Initializing PCI...");
	pci_init();
	sprintf(s,"INFO:PCI devices total:%d",count_pci_device());
	log(s);


	log("Initializing VDISK...");
	vdisk_init();


	log("Initializing FDC...");
	int fdc_version;
	fdc_version=fdc_init();
	if(fdc_version==-1)
	{
		krnlcons_putstr_color("Warning:FDC not found.\n",0xFFFFFF,0x000000);
	}
	else if(fdc_version==-2)
	{
		krnlcons_putstr_color("Warning:FDC timeout.\n",0xFFFFFF,0x000000);
	}
	else
	{
		sprintf(s,"FDC Version:0x%x",fdc_version);
		log(s);
	}

	log("Initializing IDE...");
	init_ide();

	log("Initializing null & zero device...");
	init_nullzero();

	log("Initializing VFS...");
	vfs_init();


	log("Registering file system...");
	krnlcons_putstr_color("Device File system...",0xFFFFFF,0x000000);
	devfs_regist();
	krnlcons_putstr_color("OK\n",0x00FF00,0x000000);

	krnlcons_putstr_color("FAT...",0xFFFFFF,0x000000);
	fatfs_regist();
	krnlcons_putstr_color("OK\n",0x00FF00,0x000000);

	// krnlcons_putstr_color("ISO9660...",0xFFFFFF,0x000000);
	// iso9660_regist();
	// krnlcons_putstr_color("OK\n",0x00FF00,0x000000);
	
	log("\n");

	log("Initializing Multi Task...");
	task_t *task_a=task_init();

	log("Initializing Timer...");
	init_timer(100);
	

	log("Starting Interrupt...");
	asm_sti();
	

	log("Initializing FPU...");
	if(!init_fpu())
	{
		log("Warning:FPU is not found!");
	}

	if(vfs_do_search(vfs_open("/dev"), "hdb"))
	{
		log("Mount:/dev/hdb => /");
		vfs_mount("/dev/hdb", vfs_open("/"));
	}
	
	

	log("Loading Font...");
	char *value=(char *)kmalloc(sizeof(char)*10);
	vfs_node_t node;
	if(read_ini("/config/neumann.ini","System","load_hzk16",value)==0)
	{
		node=vfs_open("/resource/font/hzk16.bin");
		if(node!=0 && strcmp(value,"true")==0)
		{
			//成功打开
			binfo->hzk16=(char *)kmalloc(node->size+5);
			task_a->langmode=1;
			vfs_read(node,binfo->hzk16,0,node->size);
		}
		else
		{
			//文件不存在
			binfo->hzk16=0;
			task_a->langmode=0;
		}
	}
	else
	{
		//无法读出配置文件
		binfo->hzk16=0;
	}

	if(read_ini("/config/neumann.ini","System","load_hzk16f",value)==0)
	{
		node=vfs_open("/resource/font/hzk16f.bin");
		if(node!=0 && strcmp(value,"true")==0)
		{
			//成功打开
			binfo->hzk16f=(char *)kmalloc(node->size+5);
			if(binfo->hzk16==0)
			{
				task_a->langmode=2;
			}
			vfs_read(node,binfo->hzk16f,0,node->size);
		}
		else
		{
			//文件不存在
			binfo->hzk16f=0;
			task_a->langmode=0;
		}
	}
	else
	{
		//无法读出配置文件
		binfo->hzk16f=0;
	}

	kfree(value);
		
	
	log("Benching CPU...");
	uint32_t base_count=benchcpu();
	binfo->base_count=base_count;
	sprintf(s,"Base Count is %08x",base_count);
	log(s);

	log("System is ready.");
	
	
	// vdisk_print();
	// char *vdisk_buf=kmalloc(2048);
	// int status=rw_vdisk(2,0,vdisk_buf,1,1);z
	// if(status==0)
	// {
	// 	krnlcons_putstr("Failed.\n");
	// }
	// for(int i=0;i<512;i++)
	// {
	// 	sprintf(s,"0x%02x ",vdisk_buf[i]&0xff);
	// 	krnlcons_putstr(s);
	// }
	// for(;;);

	print_pcinfo(0);


	//for(;;);



	log("\n\nNow Loading Desktop...");
	
	init_desktop(buf_back,binfo->scrnx,binfo->scrny);
	
	
	
	

	
	

	buf_mouse=(uint32_t *)kmalloc(sizeof(uint32_t)*24*24);
	draw_mouse(buf_mouse);
	

	sht_mouse=sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse,buf_mouse,24,24,DESKTOP_BACKCOLOR);
	
	
	
	mouse_x=binfo->scrnx/2;
	mouse_y=binfo->scrny/2;

	sheet_slide(sht_back,  0,  0);
	sheet_updown(sht_back,  0);
	sheet_slide(sht_mouse,  mouse_x,  mouse_y);
	sheet_updown(sht_mouse,  1);

	//putblock(binfo->vram,binfo->scrnx,24,24,mouse_x,mouse_y,mouse,24);

	//使其能够处理鼠标与键盘
	task_t *task_c=create_kernel_task(taskc_main);
	task_run(task_c);

	ready_system();//准备系统


	task_t *task_b=create_kernel_task(taskb_main);
	task_run(task_b);

	ime_init();

	


	if(binfo->hzk16==0 && binfo->hzk16f==0)warn_message("Chinese character library loading failed! Some Chinese characters may become garbled.","Chinese Font Library Not Loaded");
	if(binfo->hzk16==0 && binfo->hzk16f!=0)warn_message("简体中文字库(HZK16.BIN)无法加载！","字库未加载");
	if(binfo->hzk16!=0 && binfo->hzk16f==0)warn_message("繁体中文字库(HZK16F.BIN)无法加载！","字库未加载");
	
	console=open_console();

	keywin=console->window->sheet;

	for(;;);//悬挂
}

void ready_system()
{
	// window_t *win=create_window("Login",300,16*8+18,-1,0);

	// while(1);
}