/**
 * This is a minimal pruss emulation api used to build applications
 * without the actual pruss interface library. On actual platforms
 * you need to configure the build path to pickup the actual
 * prussdrv.h
 */
#define PRUSS0_PRU0_DATARAM     0
#define PRUSS0_PRU1_DATARAM     1
#define PRUSS0_PRU0_IRAM        2
#define PRUSS0_PRU1_IRAM        3

#define PRU_EVTOUT_0            0
#define PRU_EVTOUT_1            1
#define PRU_EVTOUT_2            2
#define PRU_EVTOUT_3            3
#define PRU_EVTOUT_4            4
#define PRU_EVTOUT_5            5
#define PRU_EVTOUT_6            6
#define PRU_EVTOUT_7            7

#if defined (__cplusplus) 
extern "C" {
#endif

typedef struct {
  int words;
} tpruss_intc_initdata;

int prussdrv_init( void );
int prussdrv_open( unsigned int evts );
int prussdrv_map_extmem( void ** addr );
int prussdrv_map_prumem( unsigned int which, void **addr );
int prussdrv_pru_enable( unsigned int prunum );
int prussdrv_pru_disable( unsigned int prunum );
int prussdrv_pru_write_memory(unsigned int pru_ram_id,
                              unsigned int wordoffset,
                              unsigned int *memarea,
                              unsigned int bytelength);
int prussdrv_pruintc_init( tpruss_intc_initdata * prussintc_init_data );
unsigned int prussdrv_get_phys_addr(void *address);

#if defined (__cplusplus) 
}
#endif
