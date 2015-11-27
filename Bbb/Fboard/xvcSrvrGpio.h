
#define GPIO_TDI 0
#define GPIO_TDO 1
#define GPIO_TCK 2
#define GPIO_TMS 3

void gpio_init(void);
void gpio_close(void);
void gpio_set(int i, int val);
int  gpio_get(int i);

extern int verbose;
