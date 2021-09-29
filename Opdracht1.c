#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "PJ_RPI.h"
#include <time.h>
#include <gpiod.h>
#include <unistd.h>

int Register(lineButton, con);
int shtable(con);

int Register(lineButton, con)
{
	int val, val1;
	int flag = 0;

	while (flag == 0)
	{

		// Read button status and exit if pressed
		val = gpiod_line_get_value(lineButton);

		/*gpiod_line_release(lineButton);
		gpiod_chip_close(chip);*/

		//////////////////////////////////////////////////////// time registration
		if (val != val1)
		{
			time_t rawtime;
			struct tm *timeinfo;

			time(&rawtime);
			timeinfo = localtime(&rawtime);

			char pin[] = "pin 5";
			char state[] = "4";
			char p1[255] = "INSERT INTO subjects( pin, state, time) VALUES ('";
			char p2[] = "',";
			char p3[] = ",'";
			char p4[] = "');";

			sprintf(state, "%d", val); // convert int to char

			strcat(p1, pin); // Concatenates p1 met pin
			strcat(p1, p2);	 // Concatenates p1 (p1+pin) met p2
			strcat(p1, state);
			strcat(p1, p3);
			strcat(p1, asctime(timeinfo));
			strcat(p1, p4);

			printf("%s\r\n", p1);

			if (mysql_query(con, p1))
			{
				finish_with_error(con);
			}
			val1 = val;
			flag = 1;
		}
	}
}

int shtable(con) //////////////////////////////////////////////////////// Retrieve
{
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

	//////////////////////////////////////////////////////// reading
	const char *chipname = "gpiochip0";
	struct gpiod_chip *chip;
	struct gpiod_line *lineButton; // Pushbutton

	// Open GPIO chip
	chip = gpiod_chip_open_by_name(chipname);

	lineButton = gpiod_chip_get_line(chip, 21);

	// Open switch line for input
	gpiod_line_request_input(lineButton, "test");

	while (true)
	{
		Register(lineButton, con);
	}

	shtable(con);

	exit(0);
}