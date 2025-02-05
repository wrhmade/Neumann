/*
keyboard.c
键盘驱动程序
Copyright W24 Studio 
*/

#include <stdint.h>
#include <stddef.h>
#include <keyboard.h>
#include <io.h>
#include <int.h>
#include <macro.h>
#include <binfo.h>
#include <graphic.h>
#include <fifo.h>

void log(char *s);

#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

fifo_t keyfifo;
uint32_t keybuf[32];
extern uint32_t keymap[];

fifo_t decoded_key;
uint32_t dkey_buf[32];

int shift_pressing,ctrl_pressing;


static int code_with_E0 = 0;
static int shift_l;
static int shift_r;
static int alt_l;
static int alt_r;
static int ctrl_l;
static int ctrl_r;
static int caps_lock;
static int num_lock;
static int scroll_lock;
static int column;

// static void in_process(uint32_t key)
// {
//     struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    
//     char s[40];

//     if (!(key & FLAG_EXT)) {
//         sprintf(s,"Keyboard Data:%c",key & 0xFF);
//         boxfill(binfo->vram,binfo->scrnx,0,16,15*8,16+15,0xFFFFFF);
//         putstr_ascii(binfo->vram,binfo->scrnx,0,16,0xFF0000,s);
//     }
// }

static void in_process(uint32_t key)
{
    char s[50];
    sprintf(s,"Keyboard Input:%c\n",key);
    serial_putstr(s);
    if (!(key & FLAG_EXT)) {
        fifo_put(&decoded_key, key & 0xFF);
    } else {
        int raw_key = key & MASK_RAW;
        switch (raw_key) {
            case ENTER:
                fifo_put(&decoded_key, '\n');
                break;
            case BACKSPACE:
                fifo_put(&decoded_key, '\b');
                break;
            case TAB:
                fifo_put(&decoded_key, '\t');
                break;
        }
    }
}

static uint8_t get_scancode()
{
    uint8_t scancode;
    asm_cli();
    scancode = fifo_get(&keyfifo);
    asm_sti();
    return scancode;
}

static void kb_wait()
{
    uint8_t kb_stat;
    do {
        kb_stat = io_in8(KB_CMD); // KB_CMD: 0x64
    } while (kb_stat & 0x02);
}

static void kb_ack()
{
    uint8_t kb_data;
    do {
        kb_data = io_in8(KB_DATA); // KB_DATA: 0x60
    } while (kb_data != KB_ACK); // KB_ACK: 0xFA
}

static void set_leds()
{
    uint8_t led_status = (caps_lock << 2) | (num_lock << 1) | scroll_lock;

    kb_wait();
    io_out8(KB_DATA, LED_CODE); // LED_CODE: 0xED
    kb_ack();

    kb_wait();
    io_out8(KB_DATA, led_status);
    kb_ack();
}

static void keyboard_read()
{
    uint8_t scancode;
    int make;
    uint32_t key = 0;
    uint32_t *keyrow;
    if (fifo_status(&keyfifo) > 0) {
        scancode = get_scancode();
        if (scancode == 0xE1) {
            // 特殊开头，暂不做处理
        } else if (scancode == 0xE0) {
            code_with_E0 = 1;
        } else {
            make = scancode & FLAG_BREAK ? false : true;
            keyrow = &keymap[(scancode & 0x7f) * MAP_COLS];
            column = 0;

            

            if(scancode==0x2a)
            {
                shift_pressing|=1;
            }
            if(scancode==0x36)
            {
                shift_pressing|=2;
            }
            if(scancode==0xaa)
            {
                shift_pressing&=~1;
            }
            if(scancode==0x1d)
            {
                ctrl_pressing=1;
            }
            if(scancode==0x9d)
            {
                ctrl_pressing=0;
            }

        

            int caps = shift_l || shift_r;
            if (caps_lock) {
                if ((keyrow[0] >= 'a') && (keyrow[0] <= 'z')) caps = !caps;
            }
            if (caps) {
                column = 1;
            }
            if (code_with_E0) {
                column = 2;
                code_with_E0 = 0;
            }
            key = keyrow[column];
            
            switch (key) {
                case SHIFT_L:
                    shift_l = make;
                    break;
                case SHIFT_R:
                    shift_r = make;
                    break;
                case CTRL_L:
                    ctrl_l = make;
                    break;
                case CTRL_R:
                    ctrl_r = make;
                    break;
                case ALT_L:
                    alt_l = make;
                    break;
                case ALT_R:
                    alt_r = make;
                    break;
                case CAPS_LOCK:
                    if (make) {
                        caps_lock = !caps_lock;
                        set_leds();
                    }
                    break;
                case NUM_LOCK:
                    if (make) {
                        num_lock = !num_lock;
                        set_leds();
                    }
                    break;
                case SCROLL_LOCK:
                    if (make) {
                        scroll_lock = !scroll_lock;
                        set_leds();
                    }
                    break;
                default:
                    break;
            }
            
            if (make) {
                int pad = 0;

                if ((key >= PAD_SLASH) && (key <= PAD_9)) {
                    pad = 1;
                    switch (key) {
                        case PAD_SLASH:
                            key = '/';
                            break;
                        case PAD_STAR:
                            key = '*';
                            break;
                        case PAD_MINUS:
                            key = '-';
                            break;
                        case PAD_PLUS:
                            key = '+';
                            break;
                        case PAD_ENTER:
                            key = ENTER;
                            break;
                        default:
                            if (num_lock && (key >= PAD_0) && (key <= PAD_9)) {
                                key = key - PAD_0 + '0';
                            } else if (num_lock && (key == PAD_DOT)) {
                                key = '.';
                            } else {
                                switch (key) {
                                    case PAD_HOME:
                                        key = HOME;
                                        break;
                                    case PAD_END:
                                        key = END;
                                        break;
                                    case PAD_PAGEUP:
                                        key = PAGEUP;
                                        break;
                                    case PAD_PAGEDOWN:
                                        key = PAD_PAGEDOWN;
                                        break;
                                    case PAD_INS:
                                        key = INSERT;
                                        break;
                                    case PAD_UP:
                                        key = UP;
                                        break;
                                    case PAD_DOWN:
                                        key = DOWN;
                                        break;
                                    case PAD_LEFT:
                                        key = LEFT;
                                        break;
                                    case PAD_RIGHT:
                                        key = RIGHT;
                                        break;
                                    case PAD_DOT:
                                        key = DELETE;
                                        break;
                                    default:
                                        break;
                                }
                            }
                            break;
                    }
                }
                key |= shift_l ? FLAG_SHIFT_L : 0;
                key |= shift_r ? FLAG_SHIFT_R : 0;
                key |= ctrl_l  ? FLAG_CTRL_L  : 0;
                key |= ctrl_r  ? FLAG_CTRL_R  : 0;
                key |= alt_l   ? FLAG_ALT_L   : 0;
                key |= alt_r   ? FLAG_ALT_R   : 0;
                key |= pad     ? FLAG_PAD     : 0;

                in_process(key);
            }

        }
    }
}




void keyboard_handler(registers_t regs)
{
    fifo_put(&keyfifo, io_in8(KB_DATA));
    keyboard_read();

}

void init_keyboard(void)
{
    wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE); 

    fifo_init(&keyfifo, 32, keybuf);
    fifo_init(&decoded_key, 32, dkey_buf);
    
    shift_l = shift_r = 0;
    alt_l = alt_r = 0;
    ctrl_l = ctrl_r = 0;

    caps_lock = 0;
    num_lock = 0;
    scroll_lock = 0;
    set_leds();

    
    register_interrupt_handler(IRQ1,keyboard_handler);
}