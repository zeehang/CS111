#include "display.h"

void send_command(uint8_t value)
{
    mraa_i2c_address(i2c, LCD_ADDRESS);
    uint8_t data[2] = {0x80, value};
    mraa_i2c_write(i2c, data, 2);
}

void send_data(uint8_t value)
{
    mraa_i2c_address(i2c, LCD_ADDRESS);
    uint8_t data[2] = {0x40, value};
    mraa_i2c_write(i2c, data, 2);
}

void set_reg(uint8_t addr, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = addr;
    buf[1] = data;
    mraa_i2c_address(i2c, RGB_ADDRESS);
    mraa_i2c_write(i2c, buf, 2);
}

void set_cursor(uint8_t col, uint8_t row)
{
    col = (row == 0 ? col|0x80 : col|0xc0);
    unsigned char dta[2] = {0x80, col};
    mraa_i2c_address(i2c, LCD_ADDRESS);
    mraa_i2c_write(i2c, dta, 2);
}

void set_RGB(uint8_t r, uint8_t g, uint8_t b)
{
    set_reg(REG_RED, r);
    set_reg(REG_GREEN, g);
    set_reg(REG_BLUE, b);
}

void print_temperature(char* toprint)
{
    set_cursor(0,0);
    for(int i = 0; i < strlen(toprint); i++)
    {
        send_data(toprint[i]);
    }
}

void display_on()
{
    displaycontrol |= LCD_DISPLAYON;
    send_command(LCD_DISPLAYCONTROL | displaycontrol);
}

void display_off()
{
    displaycontrol |= ~LCD_DISPLAYON;
    send_command(LCD_DISPLAYCONTROL | displaycontrol);
}

void initialize_connection()
{
    mraa_init();
    i2c = mraa_i2c_init(0);
    
    
    numlines = 2;
    current_line = 0;
    displayfunction = LCD_2LINE;

    usleep(50000);

    send_command(LCD_FUNCTIONSET | displayfunction);
    usleep(4500);

    send_command(LCD_FUNCTIONSET | displayfunction);
    usleep(4500);
    send_command(LCD_FUNCTIONSET | displayfunction);
    send_command(LCD_FUNCTIONSET | displayfunction);

    displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    send_command(LCD_DISPLAYCONTROL | displaycontrol);
    // send_command(LCD_CLEARDISPLAY);
    // sleep(1);

    display_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    send_command(LCD_ENTRYMODESET | display_mode);

    // set_reg(REG_MODE1, 0);
    // set_reg(REG_OUTPUT, 0XFF);
    // set_reg(REG_MODE2, 0x20);

    set_reg(0,0);
    set_reg(1,1);
    set_reg(0x08, 0xFF);
    set_reg(REG_MODE2, 0x20);

    set_RGB(255, 255, 255);
    //these ar eblink commands
     set_reg(0x07, 0x00);
     set_reg(0x06, 0xff);
  

    // send_command(LCD_CLEARDISPLAY);
    // sleep(1);
    send_command(LCD_DISPLAYCONTROL | displaycontrol);
    send_command(LCD_RETURNHOME);
    usleep(2000);
    send_command(LCD_CLEARDISPLAY);
    usleep(2000);
     
}
