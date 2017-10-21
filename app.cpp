#include "app.h"

int32_t FONT_WIDTH, FONT_HEIGHT, NLINES;

// Speed settings
uint32_t
DRIVE_SPEED = 30,
SPECIAL_SPEED = 15;

// Sensor mapping
sensor_port_t
TLEFT_P = EV3_PORT_1, 
COLOR_P = EV3_PORT_2,
ULTRA_P = EV3_PORT_3,
TRIGHT_P = EV3_PORT_4;

// Motor mapping
motor_port_t
LEFT_P = EV3_PORT_A,
RIGHT_P = EV3_PORT_D;

bool_t
touch_left = false,
touch_right = false,
do_exit = false;
colorid_t color;
int16_t ultrasonic = 0;

uint8_t slave_address[6] = { 0x00, 0x17, 0xE9, 0xB2, 0x56, 0x99 };
const char* pin = "0000";
static FILE *bt_con;
bool_t is_master = false;
int colors[3] = {0, 0, 0};  //red, yellow, blue

int line = 0;

void main_task(intptr_t unused) {
    init();
    if (is_master)
        fprintf(bt_con, "Hello Slave!\n");
    else
        fprintf(bt_con, "Hello Master!\n");
    config();
    read_sensors(1);
    while(1)
    {

        ev3_led_set_color(LED_GREEN);
        ev3_motor_set_power(LEFT_P, DRIVE_SPEED);
        ev3_motor_set_power(RIGHT_P, DRIVE_SPEED);
        if(color == 1) //black border detected
        {
            ev3_led_set_color(LED_RED);
            avoid();
        }
        if(touch_left == 1 || touch_right == 1) //object detected
        {
            ev3_speaker_play_tone(500, 1000);
            ev3_led_set_color(LED_RED);
            avoid();
        }
        if(ultrasonic <=30) //object detected
        {
            ev3_led_set_color(LED_ORANGE);
            avoid();
        }

        detect_colors();

        sleep(100);
        read_sensors(1);
    }
    
       
}

void bt_task(intptr_t unused) {
    static char buf[16];
    while (fgets(buf, 16, bt_con)) {
        cycle_print(buf);
      //  if(&buf=="111")
      //  {
      //      mission_accomplished();
      //  }
        dly_tsk(500);
    }
}

void btConnect() {
    while(true) {
        if (is_master) {
            bt_con = fdopen(SIO_PORT_SPP_MASTER_TEST_FILENO, "a+");
            //open file for update(read & write)
            if (bt_con != NULL) {
                setbuf(bt_con, NULL);
                while (!isConnected()) { 
                    cycle_print((char*)"Connecting...");
                    spp_master_test_connect(slave_address, pin);
                    sleep(1000);
                }
                break;
            }
        } else {
            while (!ev3_bluetooth_is_connected()) { //check whether Bluetooth SPP is connected
                cycle_print((char*)"Waiting for connection...");
                sleep(1000);
            }
            bt_con = ev3_serial_open_file(EV3_SERIAL_BT); //open serial port as a file
            break;
        }
        sleep(1000);
    }
    cycle_print((char*)"Connected.");
    act_tsk(BT_TASK);
}

bool_t isConnected() {
    T_SERIAL_RPOR rpor;
    ER ercd = serial_ref_por(SIO_PORT_SPP_MASTER_TEST, &rpor);
    return ercd == E_OK;
}

void set_font(lcdfont_t font) {
    ev3_lcd_set_font(font);
    ev3_font_get_size(font, &FONT_WIDTH, &FONT_HEIGHT);
    NLINES = EV3_LCD_HEIGHT / FONT_HEIGHT;
}

void init() {
    set_font(EV3_FONT_SMALL);
    btConnect();
}

void cycle_print(char* message) {
    int printLine = ++line % NLINES;
    if (line >= NLINES)
        ev3_lcd_clear_line_range(printLine, printLine + 1);
    ev3_print(printLine, message);
}
void read_sensors(int display_line) {
	touch_left = ev3_touch_sensor_is_pressed(TLEFT_P);
	touch_right = ev3_touch_sensor_is_pressed(TRIGHT_P);
	color = ev3_color_sensor_get_color(COLOR_P);
	ultrasonic = ev3_ultrasonic_sensor_get_distance(ULTRA_P);
}
void avoid()
{
	//stop motors
	ev3_motor_stop(LEFT_P, true);
	ev3_motor_stop(RIGHT_P, true);
	//move backwards with 180 angle to the right
	ev3_motor_rotate(RIGHT_P, 180, -30, true);
}
void config() 
{
    //	Motor init
	ev3_motor_config(LEFT_P, LARGE_MOTOR);
	ev3_motor_config(RIGHT_P, LARGE_MOTOR);
    //	Sensor init
	ev3_sensor_config(ULTRA_P, ULTRASONIC_SENSOR);
	ev3_sensor_config(COLOR_P, COLOR_SENSOR);
	ev3_sensor_config(TLEFT_P, TOUCH_SENSOR);
	ev3_sensor_config(TRIGHT_P, TOUCH_SENSOR);
}
void make_aware(){
   int i;
   for(i=0; i<3; i++)
    {
        //memcpy(string, colors, sizeof(colors)-1);
        fprintf(bt_con,"%d ", colors[i]);        
    }
}
void detect_colors()
{
    if(color == COLOR_RED && colors[0] == 0)
    {
        colors[0] = '1';
        make_aware();
        ev3_speaker_play_tone(250, 500);
        //red found
    } 
    if(color == COLOR_YELLOW && colors[1] == 0)
    {
        colors[1] = '1';
        //yellow found
        make_aware();
        ev3_speaker_play_tone(250, 500);
    } 
    if(color == COLOR_BLUE && colors[2] == 0)
    {
        colors[2] ='1';
        ev3_speaker_play_tone(250, 500);
        //blue found
        make_aware();
    } 
}
void mission_accomplished()
{
    //stop motors
	ev3_motor_stop(LEFT_P, true);
    ev3_motor_stop(RIGHT_P, true);

    ev3_led_set_color(LED_GREEN);
    ev3_speaker_play_tone(250, 500);
}
