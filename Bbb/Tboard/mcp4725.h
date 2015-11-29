class MCP4725 {

public:

    MCP4725();

    uint32_t Configure( UI2C *ui2c );
    uint32_t Dbg( uint32_t dbg );
    uint32_t Set( uint8_t devAddr, int value );

    // obsolete this
    uint32_t Write( uint8_t  devAddr, 
                    uint8_t  regAddr, 
                    uint8_t *regBytes, 
                    int      nBytes );

    // obsolete this
    uint32_t Read(  uint8_t  devAddr, 
                    uint8_t  regAddr, 
                    uint8_t *regBytes, 
                    int      nBytes );
    
private:
    uint32_t         mDbg;
    UI2C            *mI2c;

};

