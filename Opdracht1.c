#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "PJ_RPI.h"
#include <time.h>

int GPIO();

int GPIO()
{
	if (map_peripheral(&gpio) == -1)
	{
		printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
		return -1;
	}

	// Define gpio 17 as output
	INP_GPIO(17);
	OUT_GPIO(17);

	while (1)
	{
		// Toggle 17 (blink a led!)
		GPIO_SET = 1 << 17;
		sleep(1);

		GPIO_CLR = 1 << 17;
		sleep(1);
	}

	return 0;
}

void finish_with_error(MYSQL *con)
{
	fprintf(stderr, "%s\n", mysql_error(con));
	mysql_close(con);
	exit(1);
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

	///////////////////////////////// time registration
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	

	char pin[] = "pin 5";
	char state[] = "1";
	char p1[255] = "INSERT INTO subjects( pin, state, time) VALUES ('";
	char p2[] = "',";
	char p3[] = ",'";
	char p4[] = "');";

	strcat(p1, pin); // Concatenates p1 met pin
	strcat(p1, p2);	 // Concatenates p1 (p1+pin) met p2
	strcat(p1, state);
	strcat(p1, p3);
	strcat(p1, asctime(timeinfo));
	strcat(p1, p4);

	printf("%s", p1);
	if (mysql_query(con, p1))
	{
		finish_with_error(con);
	}

	//////////////////////////////////////////////////////// Retrieve
	if (mysql_query(con, "SELECT * FROM subjects"))
	{
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);

	if (result == NULL)
	{
		finish_with_error(con);
	}

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	MYSQL_FIELD *field;

	while ((row = mysql_fetch_row(result)))
	{
		for (int i = 0; i < num_fields; i++)
		{
			if (i == 0)
			{
				while (field = mysql_fetch_field(result))
				{
					printf("%s ", field->name);
				}

				printf("\n");
			}

			printf("%s  ", row[i] ? row[i] : "NULL");
		}
	}

	printf("\n");

	mysql_free_result(result);
	mysql_close(con);
	exit(0);
}
