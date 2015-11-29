
/**
This class implements a user space gpio based I2C interface.  It must
be provided a gpio for scl and sda and then allows bus primitive operations
by slave device drivers.
*/
class UI2C {

public:

    /** Error flags */
#   define UI2C_ERR_CFG          0x00000001
#   define UI2C_ERR_START_COND   0x00000002
#   define UI2C_ERR_STOP_COND    0x00000004
#   define UI2C_ERR_WRITE_ACK    0x00000008
#   define UI2C_ERR_READ_ACK     0x00000010
#   define UI2C_ERR_HIGH_TOUT    0x00000020

    UI2C();

    uint32_t Dbg( uint32_t dbg );
    uint32_t configure(  GpioUtil *scl, GpioUtil *sda );
    uint32_t start_cond();
    uint32_t stop_cond();
    uint32_t write_cycle( uint8_t byte );
    uint32_t read_cycle(  uint8_t *bytePtr, int nack );

    /** Debug flags */
#   define UI2C_DBG_CFG          0x00000001
#   define UI2C_DBG_START        0x00000002
#   define UI2C_DBG_WRITE_CYCLE  0x00000004

private:
    int       mDbg;      // Debug flags (or in items of interest)
    int       mUsHold;   // Microseconds to hold values
    GpioUtil *mGpioSCL;  // User space SCL gpio object (see configure)
    GpioUtil *mGpioSDA;  // User space SDA gpio object (see configure)

    uint32_t  wait_high( GpioUtil *pin );
    uint32_t  read_value( GpioUtil *pin );
    uint32_t  pull_low( GpioUtil *pin );

};

