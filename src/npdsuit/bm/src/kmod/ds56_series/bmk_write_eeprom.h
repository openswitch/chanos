#ifndef BMK_WRITE_I2C 
#define BMK_WRITE_I2C
extern int _ax_octeon_twsi_write8(uint8_t, uint8_t, uint8_t );
int  _ax_i2c_write8(unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len);

#endif





