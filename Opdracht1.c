#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "PJ_RPI.h"

int GPIO();

int GPIO(){
    if(map_peripheral(&gpio) == -1) 
	{
       	 	printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        	return -1;
    	}

	// Define gpio 17 as output
	INP_GPIO(17);
	OUT_GPIO(17);

	while(1)
	{
		// Toggle 17 (blink a led!)
		GPIO_SET = 1 << 17;
		sleep(1);

		GPIO_CLR = 1 << 17;
		sleep(1);
	}

	return 0;	
}

int main(int argc, char **argv)
{
	MYSQL *con = mysql_init(NULL);
	printf("MySQL client version: %s\n", mysql_get_client_info());

	if (con == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		exit(1);
	}

	if (mysql_real_connect(con, "localhost", "webuser", "secretpassword",
												 "globe_bank", 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(con));
		mysql_close(con);
		exit(1);
	}

    GPIO();

    //mysql_free_result(result);
	mysql_close(con);
	exit(0);
}
